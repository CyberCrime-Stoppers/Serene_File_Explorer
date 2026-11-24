#pragma once
/*
 * Serene File Explorer - sidebar (places, devices, bookmarks, tags)
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Part of SereneOS / SereneForge
 */
#include <QWidget>

class QTreeWidget;
class QTreeWidgetItem;

class Sidebar : public QWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    /// Track the active directory (context for "Add Bookmark", highlight)
    void setCurrentPath(const QString &absPath);

    void refreshDevices();
    void reloadBookmarks();
    void reloadTags();

    /// Add a bookmark; returns false if the path is already bookmarked.
    bool addBookmark(const QString &absPath);

signals:
    /// Emitted when the user activates a location. Empty = ignore.
    void placeActivated(const QString &absPath);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onContextMenu(const QPoint &pos);

private:
    enum Section { SecPlaces = 0, SecDevices, SecBookmarks, SecTags };

    void buildSections();
    QTreeWidgetItem *sectionItem(int section) const;

    QTreeWidget    *m_tree      = nullptr;
    QTreeWidgetItem *m_places    = nullptr;
    QTreeWidgetItem *m_devices   = nullptr;
    QTreeWidgetItem *m_bookmarks = nullptr;
    QTreeWidgetItem *m_tags      = nullptr;

    QString m_currentPath;
};
