#include "filesystem.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace FS {

QList<FileEntry> listDirectory(const QString &path, bool showHidden, QString *error)
{
    QList<FileEntry> out;
    QDir dir(path);
    if (!dir.exists()) {
        if (error) *error = QObject::tr("Folder does not exist: %1").arg(path);
        return out;
    }
    const QFlags<QDir::Filter> filter = QDir::AllEntries | QDir::System
        | QDir::NoDotAndDotDot
        | (showHidden ? QDir::Hidden : QFlags<QDir::Filter>());
    const QFileInfoList infos = dir.entryInfoList(filter,
        QDir::DirsFirst | QDir::Name | QDir::LocaleAware);
    out.reserve(infos.size());
    for (const QFileInfo &fi : infos) {
        FileEntry e;
        e.name   = fi.fileName();
        e.path   = fi.absoluteFilePath();
        e.isDir  = fi.isDir();
        e.size   = fi.isDir() ? 0 : fi.size();
        e.mtime  = fi.lastModified();
        out.append(e);
    }
    return out;
}

QString humanSize(qint64 bytes)
{
    if (bytes < 0)    return QObject::tr("Unknown");
    if (bytes < 1024) return QString::number(bytes) + QLatin1String(" B");
    const double kb = bytes / 1024.0;
    if (kb < 1024.0)  return QString::number(kb, 'f', 1) + QLatin1String(" KB");
    const double mb = kb / 1024.0;
    if (mb < 1024.0)  return QString::number(mb, 'f', 1) + QLatin1String(" MB");
    const double gb = mb / 1024.0;
    if (gb < 1024.0)  return QString::number(gb, 'f', 1) + QLatin1String(" GB");
    return QString::number(gb / 1024.0, 'f', 1) + QLatin1String(" TB");
}

QString fileTypeName(const QFileInfo &fi)
{
    if (fi.isDir())     return QObject::tr("Folder");
    if (fi.isSymLink()) return QObject::tr("Symbolic link");
    const QString suf = fi.suffix().toLower();
    if (suf.isEmpty())  return QObject::tr("File");
    return suf.toUpper() + QLatin1String(" ");
}

QString uniqueChildName(const QString &dir, const QString &name)
{
    QString candidate = dir + QLatin1Char('/') + name;
    if (!QFileInfo::exists(candidate))
        return name;

    const QFileInfo fi(name);
    const QString base = fi.completeBaseName();
    const QString ext  = fi.suffix();
    for (int n = 2; n < 10000; ++n) {
        QString test = base + QStringLiteral(" (%1)").arg(n);
        if (!ext.isEmpty())
            test += QLatin1Char('.') + ext;
        candidate = dir + QLatin1Char('/') + test;
        if (!QFileInfo::exists(candidate))
            return test;
    }
    return name;
}

bool moveToTrash(const QStringList &paths, QString *error)
{
    QString errs;
    for (const QString &p : paths) {
        QString ignored;
        if (!QFile::moveToTrash(p, &ignored))
            errs += p + QLatin1String(": ") + ignored + QLatin1String("; ");
    }
    if (error) *error = errs;
    return errs.isEmpty();
}

bool deletePermanent(const QStringList &paths, QString *error)
{
    QString errs;
    for (const QString &p : paths) {
        QFileInfo fi(p);
        bool ok = false;
        if (fi.isDir()) {
            ok = QDir(p).removeRecursively();
            if (!ok) errs += p + QLatin1String(": cannot remove folder; ");
        } else {
            ok = QFile::remove(p);
            if (!ok) errs += p + QLatin1String(": cannot remove file; ");
        }
    }
    if (error) *error = errs;
    return errs.isEmpty();
}

bool createFolder(const QString &path, QString *error)
{
    if (QDir().mkpath(path))
        return true;
    if (error) *error = QObject::tr("Could not create folder: %1").arg(path);
    return false;
}

bool createEmptyFile(const QString &path, QString *error)
{
    QFile f(path);
    if (f.open(QIODevice::WriteOnly | QIODevice::NewOnly)) {
        f.close();
        return true;
    }
    if (error) *error = f.errorString();
    return false;
}

} // namespace FS
