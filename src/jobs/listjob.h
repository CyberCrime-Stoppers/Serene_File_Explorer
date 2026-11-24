#pragma once
/*
 * Serene File Explorer - background directory listing job (thread pool)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QObject>
#include <QRunnable>

#include "core/types.h"

class ListJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ListJob(const QString &path, bool showHidden, QObject *parent = nullptr);

    void run() override;

    /// Cancel: results will not be emitted after cancel() is called.
    void cancel() { m_cancelled = true; }

signals:
    void finished(const QString &path, const QList<FileEntry> &entries,
                  const QString &error);

private:
    QString m_path;
    bool    m_showHidden;
    volatile bool m_cancelled = false;
};
