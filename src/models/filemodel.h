#pragma once
/*
 * Serene File Explorer - async file list model (thread-pool backed)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QAbstractListModel>
#include <QStringList>

#include "core/types.h"

class FileModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FileModel(QObject *parent = nullptr);

    enum Roles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        IsDirRole,
        SizeRole,
        MTimeRole,
        TypeNameRole,
        HumanSizeRole
    };

    // QAbstractItemModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // State
    QString path() const { return m_path; }
    void setPath(const QString &absPath);   // normalize + reload

    void setSortMode(int mode);            // Settings::SortMode
    void setShowHidden(bool show);
    void setFilter(const QString &text);   // instant name filter (client-side)

    void reload();                         // spawn a background ListJob
    void cancelPending();

    /// Convenience: find row for absolute path (used by tests + selection ops)
    int rowForPath(const QString &absolutePath) const;

    FileEntry entryAt(const QModelIndex &idx) const;

signals:
    void loadingStarted(const QString &path);
    void loaded(const QString &path, int count);
    void loadFailed(const QString &path, const QString &error);

private slots:
    void onJobFinished(const QString &path, const QList<FileEntry> &entries,
                       const QString &error);

private:
    void applyClientFilter();

    QString            m_path;
    QList<FileEntry>   m_entries;   // filtered view
    QList<FileEntry>   m_all;       // unfiltered (from last successful list)
    int                m_sortMode   = 0;
    bool               m_showHidden = false;
    QString            m_filterText;
    QObject           *m_activeJob  = nullptr; // for cancel tracking
};
