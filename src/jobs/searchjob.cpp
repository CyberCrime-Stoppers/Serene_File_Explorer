#include "searchjob.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

#include "core/filesystem.h"

SearchJob::SearchJob(const QString &root, const QString &pattern,
                     int mode, int maxResults, QObject *parent)
    : QObject(parent), m_root(root), m_pattern(pattern),
      m_mode(mode), m_maxResults(maxResults)
{
    setAutoDelete(true);
}

bool SearchJob::locateAvailable()
{
    return QStandardPaths::findExecutable(QStringLiteral("locate")).isEmpty()
               ? false : true;
}

void SearchJob::run()
{
    if (m_cancelled)
        return;

    bool useIndex = (m_mode == 1);
    if (m_mode == 0) // auto: prefer locate when present
        useIndex = locateAvailable();

    if (useIndex) {
        if (locateAvailable()) {
            runWithLocate();
            return;
        }
        // fall through to scan if locate suddenly became unavailable
    }
    runWithScan();
}

void SearchJob::runWithLocate()
{
    QProcess proc;
    proc.setProgram(QStringLiteral("locate"));
    proc.setArguments({ QStringLiteral("-l"), QString::number(m_maxResults),
                        QStringLiteral("-i"), m_pattern });
    proc.start();
    if (!proc.waitForStarted(3000)) {
        runWithScan();
        return;
    }
    proc.waitForFinished(10000);

    const QStringList lines = QString::fromUtf8(proc.readAllStandardOutput())
                                  .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (m_cancelled || m_hits >= m_maxResults)
            break;
        QFileInfo fi(line);
        if (!fi.exists())
            continue; // stale index entry
        FileEntry e;
        e.name  = fi.fileName();
        e.path  = fi.absoluteFilePath();
        e.isDir = fi.isDir();
        e.size  = fi.isDir() ? 0 : fi.size();
        e.mtime = fi.lastModified();
        ++m_hits;
        emit hitFound(e);
    }
    emit finished(m_root, m_hits, true, QString());
}

void SearchJob::runWithScan()
{
    QDirIterator it(m_root,
                    QDir::AllEntries | QDir::System | QDir::Hidden | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);

    const QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(
                                    m_pattern, QRegularExpression::NonPathWildcardConversion),
                                QRegularExpression::CaseInsensitiveOption);

    while (!m_cancelled && m_hits < m_maxResults && it.hasNext()) {
        const QString path = it.next();
        const QFileInfo fi(path);
        if (!rx.match(fi.fileName()).hasMatch())
            continue;
        FileEntry e;
        e.name  = fi.fileName();
        e.path  = fi.absoluteFilePath();
        e.isDir = fi.isDir();
        e.size  = fi.isDir() ? 0 : fi.size();
        e.mtime = fi.lastModified();
        ++m_hits;
        emit hitFound(e);
    }

    emit finished(m_root, m_hits, false,
                  m_cancelled ? QStringLiteral("cancelled") : QString());
}
