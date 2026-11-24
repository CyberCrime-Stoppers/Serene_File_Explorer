#include "transfermanager.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QAtomicInt>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#include "core/filesystem.h"

TransferManager::TransferManager(QObject *parent)
    : QObject(parent)
{
    m_pause.storeRelease(0);
    m_cancel.storeRelease(0);
}

void TransferManager::enqueueCopy(const QStringList &paths, const QString &destDir)
{
    for (const QString &p : paths) {
        QFileInfo fi(p);
        TransferItem item;
        item.srcPath    = p;
        item.dstPath    = destDir + QLatin1Char('/') + FS::uniqueChildName(destDir, fi.fileName());
        item.isMove     = false;
        item.totalBytes = fi.isDir() ? 0 : fi.size();
        m_queue.enqueue(item);
    }
    if (m_state != Running)
        pump();
}

void TransferManager::enqueueMove(const QStringList &paths, const QString &destDir)
{
    for (const QString &p : paths) {
        QFileInfo fi(p);
        TransferItem item;
        item.srcPath    = p;
        item.dstPath    = destDir + QLatin1Char('/') + FS::uniqueChildName(destDir, fi.fileName());
        item.isMove     = true;
        item.totalBytes = fi.isDir() ? 0 : fi.size();
        m_queue.enqueue(item);
    }
    if (m_state != Running)
        pump();
}

void TransferManager::pause()  { m_pause.storeRelease(1); }
void TransferManager::resume() { m_pause.storeRelease(0); }

void TransferManager::cancel()
{
    m_cancel.storeRelease(1);
    m_pause.storeRelease(0); // unblock so loop can exit
}

void TransferManager::pump()
{
    if (m_queue.isEmpty()) {
        m_state = Idle;
        return;
    }

    m_state = Running;
    m_cancel.storeRelease(0);
    m_pause.storeRelease(0);
    emit started(m_queue.size());

    int index = 0;
    while (!m_queue.isEmpty()) {
        if (m_cancel.loadAcquire()) {
            m_queue.clear();
            m_state = Done;
            emit finished(true);
            return;
        }
        while (m_pause.loadAcquire()) {
            QThread::msleep(100);
            QCoreApplication::processEvents();
            if (m_cancel.loadAcquire())
                break;
        }
        if (m_cancel.loadAcquire()) {
            m_queue.clear();
            m_state = Done;
            emit finished(true);
            return;
        }

        const TransferItem item = m_queue.dequeue();
        QFileInfo srcFi(item.srcPath);

        if (item.isMove) {
            // Fast path: rename within same filesystem
            if (QFile::rename(item.srcPath, item.dstPath)) {
                emit itemFinished(index++);
                continue;
            }
            // Otherwise fall back to copy+delete (cross-device move)
        }

        if (srcFi.isDir()) {
            // Recursive copy/move
            QDir(srcFi.absolutePath()).mkpath(item.dstPath);
            QDirIterator dit(item.srcPath,
                             QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot,
                             QDirIterator::Subdirectories);
            while (dit.hasNext()) {
                if (m_cancel.loadAcquire()) {
                    m_queue.clear();
                    m_state = Done;
                    emit finished(true);
                    return;
                }
                while (m_pause.loadAcquire()) {
                    QThread::msleep(100);
                    QCoreApplication::processEvents();
                }
                const QString child = dit.next();
                const QFileInfo cfi(child);
                const QString rel   = child.mid(item.srcPath.length() + 1);
                const QString target = item.dstPath + QLatin1Char('/') + rel;
                if (cfi.isDir()) {
                    QDir().mkpath(target);
                } else {
                    QFile::remove(target);
                    if (!QFile::copy(child, target)) {
                        emit error(tr("Failed to copy %1").arg(child));
                    }
                }
                emit itemProgress(cfi.size());
            }
            if (item.isMove) {
                QDir(item.srcPath).removeRecursively();
            }
        } else {
            QFile src(item.srcPath);
            if (!src.open(QIODevice::ReadOnly)) {
                emit error(tr("Cannot open %1: %2").arg(item.srcPath, src.errorString()));
                ++index;
                continue;
            }
            QFile dst(item.dstPath);
            if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                emit error(tr("Cannot write %1: %2").arg(item.dstPath, dst.errorString()));
                ++index;
                continue;
            }
            const qint64 total = src.size();
            const qint64 chunk = 256 * 1024;
            qint64 done = 0;
            while (!src.atEnd()) {
                if (m_cancel.loadAcquire())
                    break;
                while (m_pause.loadAcquire()) {
                    QThread::msleep(100);
                    QCoreApplication::processEvents();
                }
                const QByteArray buf = src.read(chunk);
                if (dst.write(buf) != buf.size()) {
                    emit error(tr("Write failed on %1").arg(item.dstPath));
                    break;
                }
                done += buf.size();
                emit itemProgress(buf.size());
            }
            dst.close();
            src.close();
            if (item.isMove && !m_cancel.loadAcquire() && done == total)
                QFile::remove(item.srcPath);
        }
        emit itemFinished(index++);
    }

    m_state = Done;
    emit finished(false);
}
