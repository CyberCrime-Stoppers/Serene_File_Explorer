#pragma once
/*
 * Serene File Explorer - hybrid search job (background recursive scan,
 * with optional 'locate' index acceleration).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QObject>
#include <QRunnable>
#include <QStringList>

#include "core/types.h"

class SearchJob : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /// mode: 0 = auto, 1 = locate, 2 = live scan
    explicit SearchJob(const QString &root, const QString &pattern,
                       int mode, int maxResults = 5000, QObject *parent = nullptr);

    void run() override;
    void cancel() { m_cancelled = true; }

    /// True if the 'locate' binary is available on this system.
    static bool locateAvailable();

signals:
    void hitFound(const FileEntry &entry);
    void finished(const QString &root, int totalHits, bool usedIndex, const QString &error);

private:
    void runWithLocate();
    void runWithScan();

    QString     m_root;
    QString     m_pattern;
    int         m_mode;
    int         m_maxResults;
    volatile bool m_cancelled = false;
    int         m_hits = 0;
};
