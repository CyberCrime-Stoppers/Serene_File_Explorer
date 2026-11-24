#include "filemodel.h"

#include <QThreadPool>
#include <QDir>
#include <QFileInfo>
#include <algorithm>

#include "core/filesystem.h"
#include "jobs/listjob.h"
#include "core/settings.h"

FileModel::FileModel(QObject *parent)
    : QAbstractListModel(parent)
{
    qRegisterMetaType<QList<FileEntry>>("QList<FileEntry>");
}

int FileModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant FileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};
    const FileEntry &e = m_entries.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return e.name;
    case NameRole:
        return e.name;
    case PathRole:
        return e.path;
    case IsDirRole:
        return e.isDir;
    case SizeRole:
        return e.size;
    case MTimeRole:
        return e.mtime;
    case TypeNameRole:
        return e.isDir ? tr("Folder") : QFileInfo(e.path).suffix().toUpper()
                                             + QStringLiteral(" ");
    case HumanSizeRole:
        return FS::humanSize(e.size);
    default:
        return {};
    }
}

QHash<int, QByteArray> FileModel::roleNames() const
{
    auto roles = QAbstractListModel::roleNames();
    roles[NameRole]      = "name";
    roles[PathRole]      = "path";
    roles[IsDirRole]     = "isDir";
    roles[SizeRole]      = "size";
    roles[MTimeRole]     = "mtime";
    roles[TypeNameRole]  = "typeName";
    roles[HumanSizeRole] = "humanSize";
    return roles;
}

void FileModel::setSortMode(int mode)
{
    m_sortMode = mode;
    reload();
}

void FileModel::setShowHidden(bool show)
{
    m_showHidden = show;
    reload();
}

void FileModel::setFilter(const QString &text)
{
    m_filterText = text;
    applyClientFilter();
}

void FileModel::applyClientFilter()
{
    beginResetModel();
    if (m_filterText.isEmpty()) {
        m_entries = m_all;
    } else {
        m_entries.clear();
        for (const FileEntry &e : m_all) {
            if (e.name.contains(m_filterText, Qt::CaseInsensitive))
                m_entries.append(e);
        }
    }
    endResetModel();
}

void FileModel::reload()
{
    if (m_path.isEmpty())
        return;

    cancelPending();

    auto *job = new ListJob(m_path, m_showHidden);
    m_activeJob = job;
    connect(job, &ListJob::finished, this,
            [this](const QString &p, const QList<FileEntry> &entries,
                   const QString &err) {
                if (m_activeJob == sender())
                    m_activeJob = nullptr;
                onJobFinished(p, entries, err);
            },
            Qt::QueuedConnection);
    emit loadingStarted(m_path);
    QThreadPool::globalInstance()->start(job);
}

void FileModel::cancelPending()
{
    if (m_activeJob) {
        // Jobs are auto-delete QRunnables; static_cast through the lambda
        // capture is unsafe, so we simply mark the connection dead by
        // dropping the pointer. The job may still emit once; the guarded
        // connect below ignores late results via sender() check.
        auto *job = static_cast<ListJob *>(m_activeJob);
        job->cancel();
        m_activeJob = nullptr;
    }
}

void FileModel::onJobFinished(const QString &path, const QList<FileEntry> &entries,
                              const QString &error)
{
    if (path != m_path)
        return; // stale result for a folder we've navigated away from

    if (!error.isEmpty()) {
        emit loadFailed(m_path, error);
        return;
    }

    m_all = entries;

    // Apply sorting according to current mode
    switch (m_sortMode) {
    case Settings::SortSize:
        std::sort(m_all.begin(), m_all.end(),
                  [](const FileEntry &a, const FileEntry &b) {
                      if (a.isDir != b.isDir) return a.isDir; // dirs first
                      return a.size > b.size;
                  });
        break;
    case Settings::SortModified:
        std::sort(m_all.begin(), m_all.end(),
                  [](const FileEntry &a, const FileEntry &b) {
                      if (a.isDir != b.isDir) return a.isDir;
                      return a.mtime > b.mtime;
                  });
        break;
    case Settings::SortName:
    default:
        std::sort(m_all.begin(), m_all.end(),
                  [](const FileEntry &a, const FileEntry &b) {
                      if (a.isDir != b.isDir) return a.isDir;
                      return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
                  });
        break;
    }

    applyClientFilter();
    emit loaded(m_path, m_all.size());
}

int FileModel::rowForPath(const QString &absolutePath) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries.at(i).path == absolutePath)
            return i;
    }
    return -1;
}

FileEntry FileModel::entryAt(const QModelIndex &idx) const
{
    if (!idx.isValid() || idx.row() >= m_entries.size())
        return {};
    return m_entries.at(idx.row());
}
