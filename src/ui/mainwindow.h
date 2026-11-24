#pragma once
/*
 * Serene File Explorer - main window
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QMainWindow>

class DirectoryView;
class TransferManager;
class QTabWidget;
class QLabel;
class QProgressBar;
class QSplitter;
class Sidebar;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void goToUrl(const QString &path);

private slots:
    void newTab();
    void closeTab(int index);
    void closeCurrentTab();
    void onTabChanged(int index);

    // Edit operations
    void onCut();
    void onCopy();
    void onPaste();
    void onDeleteToTrash();
    void onDeletePermanent();
    void onRename();
    void onNewFolder();
    void onNewFile();
    void onProperties();
    void onSelectAll();
    void onRefresh();
    void onToggleHidden();
    void onViewModeChanged();

    // Search
    void onSearchTriggered();

    // Managers (milestone 2+ placeholders)
    void onDriveManager();
    void onNetworkManager();
    void onCloudConnections();
    void onControlPanel();
    void onPreferences();

    void updateActions();

private:
    DirectoryView *currentView() const;
    void buildUi();
    void buildMenus();
    void wireDirectoryView(DirectoryView *view);
    void applyClipboardState();

    QTabWidget      *m_tabs       = nullptr;
    QLabel          *m_status     = nullptr;
    QLabel          *m_itemInfo   = nullptr;
    QProgressBar    *m_progress   = nullptr;
    TransferManager *m_transfers  = nullptr;

    // Milestone 2 sidebar
    Sidebar     *m_sidebar    = nullptr;
    QSplitter   *m_splitter   = nullptr;
};
