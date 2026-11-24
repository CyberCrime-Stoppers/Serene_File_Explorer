#pragma once
/*
 * Serene File Explorer - filesystem helpers (sync API, called from worker jobs)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "types.h"

class QFileInfo;

namespace FS {

QList<FileEntry> listDirectory(const QString &path, bool showHidden, QString *error);

QString humanSize(qint64 bytes);

QString fileTypeName(const QFileInfo &fi);

/// Returns a name that does not collide inside \p dir ("name (2).ext" pattern).
QString uniqueChildName(const QString &dir, const QString &name);

bool moveToTrash(const QStringList &paths, QString *error);
bool deletePermanent(const QStringList &paths, QString *error);
bool createFolder(const QString &path, QString *error);
bool createEmptyFile(const QString &path, QString *error);

} // namespace FS
