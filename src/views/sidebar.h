#pragma once
/*
 * Serene File Explorer - Places / Bookmarks / Devices sidebar
 * Dolphin-style navigation panel with Serene theming.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QToolButton;

class Sidebar : public QWidget
{
    Q_OBJECT
public:
    explicit Sidebar(QWidget *parent = nullptr);

    /// Highlight + auto-scroll to the entry matching \p path (if visible).
    void setCurrentPath(const QString &path);

    void refreshDevices();

signals:
    void locationRequested(const QString &path);

private slots:
    void onItemActivated(QListWidgetItem *item);
    void onAddBookmark();
    void onRemoveBookmark();
    void onContextMenu(const QPoint &pos);

private:
    enum Section { SecPlaces = 0, SecBookmarks = 1, SecDevices = 2 };

    void buildStatic();
    void reloadBookmarks();
    void addItem(int section, const QString &title, const QString &path,
                 const QString &iconName = QString(), const QString &tooltip = QString());

    QListWidget *m_list        = nullptr;
    QToolButton *m_addBtn      = nullptr;
    QToolButton *m_removeBtn   = nullptr;
    QString      m_currentPath;
};
