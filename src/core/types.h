#pragma once
/*
 * Serene File Explorer - core shared types
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Part of SereneOS / SereneForge
 */
#include <QString>
#include <QDateTime>
#include <QMetaType>

struct FileEntry {
    QString   name;
    QString   path;
    bool      isDir  = false;
    qint64    size   = 0;
    QDateTime mtime;
};

Q_DECLARE_METATYPE(FileEntry)
Q_DECLARE_METATYPE(QList<FileEntry>)
