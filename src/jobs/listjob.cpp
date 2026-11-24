#include "listjob.h"

#include "core/filesystem.h"

ListJob::ListJob(const QString &path, bool showHidden, QObject *parent)
    : QObject(parent), m_path(path), m_showHidden(showHidden)
{
    setAutoDelete(true);
}

void ListJob::run()
{
    if (m_cancelled)
        return;

    QString err;
    const QList<FileEntry> entries = FS::listDirectory(m_path, m_showHidden, &err);

    if (!m_cancelled)
        emit finished(m_path, entries, err);
}
