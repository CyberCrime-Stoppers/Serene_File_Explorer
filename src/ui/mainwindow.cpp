#include "mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QProgressBar>
#include <QProcess>
#include <QShortcut>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>
#include <QUrl>

#include "core/clipboard.h"
#include "core/filesystem.h"
#include "core/settings.h"
#include "models/filemodel.h"
#include "transfer/transfermanager.h"
#include "ui/theme.h"
#include "views/directoryview.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    buildUi();
    buildMenus();
    updateActions();

    // Global shortcuts
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this, SLOT(newTab()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this, SLOT(closeCurrentTab()));
    new QShortcut(QKeySequence(Qt::ALT | Qt::Key_Return), this, SLOT(onProperties()));

    // Transfers
    m_transfers = new TransferManager(this);
    connect(m_transfers, &TransferManager::started, this, [this](int totalJobs) {
        m_progress->setRange(0, totalJobs);
        m_progress->setValue(0);
        m_progress->show();
    });
    connect(m_transfers, &TransferManager::itemFinished, this, [this](int index) {
        m_progress->setValue(index + 1);
        if (index + 1 >= m_progress->maximum()) {
            m_progress->hide();
            if (currentView())
                currentView()->model()->reload();
        }
    });
    connect(m_transfers, &TransferManager::error, this, [this](const QString &msg) {
        statusBar()->showMessage(msg, 8000);
    });
    connect(m_transfers, &TransferManager::finished, this, [this](bool cancelled) {
        m_progress->hide();
        if (!cancelled && currentView())
            currentView()->model()->reload();
    });

    // First tab with home folder
    newTab();
    resize(1200, 760);
}

void MainWindow::goToUrl(const QString &path)
{
    if (currentView())
        currentView()->go(path);
}

DirectoryView *MainWindow::currentView() const
{
    return qobject_cast<DirectoryView *>(m_tabs->currentWidget());
}

void MainWindow::buildUi()
{
    setWindowTitle(tr("Serene File Explorer"));

    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true);
    setCentralWidget(m_tabs);

    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);
    connect(m_tabs, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);

    // Status bar
    m_status   = new QLabel(this);
    m_itemInfo = new QLabel(this);
    m_progress = new QProgressBar(this);
    m_progress->hide();
    m_progress->setMaximumWidth(220);

    statusBar()->addWidget(m_status, 1);
    statusBar()->addPermanentWidget(m_itemInfo);
    statusBar()->addPermanentWidget(m_progress);
}

void MainWindow::buildMenus()
{
    // ---------- File menu
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    auto *actNewTab = fileMenu->addAction(tr("New &Tab"), this, &MainWindow::newTab,
                                          QKeySequence(Qt::CTRL | Qt::Key_T));
    auto *actNewWin = fileMenu->addAction(tr("New &Window"), qApp, []() {
        // Milestone 2: spawn detached instance
    }, QKeySequence(Qt::CTRL | Qt::Key_N));
    fileMenu->addSeparator();
    auto *actNewFolder = fileMenu->addAction(tr("New F&older"), this, &MainWindow::onNewFolder,
                                             QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    auto *actNewFile   = fileMenu->addAction(tr("New &File"), this, &MainWindow::onNewFile,
                                             QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("&Quit"), qApp, &QApplication::quit, QKeySequence(Qt::CTRL | Qt::Key_Q));

    // ---------- Edit menu
    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    auto *actCut  = editMenu->addAction(tr("Cu&t"), this, &MainWindow::onCut,
                                        QKeySequence::Cut);
    auto *actCopy = editMenu->addAction(tr("&Copy"), this, &MainWindow::onCopy,
                                        QKeySequence::Copy);
    auto *actPaste = editMenu->addAction(tr("&Paste"), this, &MainWindow::onPaste,
                                         QKeySequence::Paste);
    editMenu->addSeparator();
    auto *actRename = editMenu->addAction(tr("Ren&ame"), this, &MainWindow::onRename,
                                          QKeySequence(Qt::Key_F2));
    editMenu->addSeparator();
    auto *actTrash  = editMenu->addAction(tr("Move to &Trash"), this, &MainWindow::onDeleteToTrash,
                                         QKeySequence(Qt::Key_Delete));
    auto *actDel    = editMenu->addAction(tr("&Delete Permanently"), this, &MainWindow::onDeletePermanent,
                                         QKeySequence(Qt::SHIFT | Qt::Key_Delete));
    editMenu->addSeparator();
    auto *actSelectAll = editMenu->addAction(tr("Select &All"), this, &MainWindow::onSelectAll,
                                            QKeySequence::SelectAll);

    // ---------- View menu
    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
    auto *actReload = viewMenu->addAction(tr("&Reload"), this, &MainWindow::onRefresh,
                                          QKeySequence(Qt::Key_F5));
    viewMenu->addSeparator();

    auto *modeGroup = new QActionGroup(this);
    auto *actList  = new QAction(tr("&List view"), this);
    auto *actIcons = new QAction(tr("&Icon view"), this);
    actList->setCheckable(true);
    actIcons->setCheckable(true);
    modeGroup->addAction(actList);
    modeGroup->addAction(actIcons);
    actList->setData(Settings::ViewList);
    actIcons->setData(Settings::ViewIcons);
    viewMenu->addAction(actList);
    viewMenu->addAction(actIcons);
    if (Settings::viewMode() == Settings::ViewList)
        actList->setChecked(true);
    else
        actIcons->setChecked(true);
    connect(actList, &QAction::triggered, this, &MainWindow::onViewModeChanged);
    connect(actIcons, &QAction::triggered, this, &MainWindow::onViewModeChanged);
    connect(modeGroup, &QActionGroup::triggered, this, [this](QAction *a) {
        Q_UNUSED(a);
        onViewModeChanged();
    });

    viewMenu->addSeparator();
    auto *actHidden = viewMenu->addAction(tr("Show &hidden files"), this,
                                          &MainWindow::onToggleHidden);
    actHidden->setCheckable(true);
    actHidden->setChecked(Settings::showHidden());

    // ---------- Go menu
    QMenu *goMenu = menuBar()->addMenu(tr("&Go"));
    goMenu->addAction(tr("&Home"), this, [this]() { goToUrl(QDir::homePath()); },
                      QKeySequence(Qt::ALT | Qt::Key_H));
    goMenu->addAction(tr("&Documents"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    });
    goMenu->addAction(tr("&Downloads"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    });
    goMenu->addAction(tr("&Pictures"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    });
    goMenu->addAction(tr("&Music"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::MusicLocation));
    });
    goMenu->addAction(tr("&Videos"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    });
    goMenu->addSeparator();
    goMenu->addAction(tr("&Root (/)"), this, [this]() { goToUrl(QStringLiteral("/")); });
    goMenu->addAction(tr("&Trash"), this, [this]() {
        goToUrl(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                + QStringLiteral("/.local/share/Trash"));
    });

    // ---------- Tools menu (milestone 2+ modules dock here)
    QMenu *toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(tr("&Search Files…"), this, &MainWindow::onSearchTriggered,
                         QKeySequence(Qt::CTRL | Qt::Key_F));
    toolsMenu->addSeparator();
    toolsMenu->addAction(tr("&Drive Manager…"), this, &MainWindow::onDriveManager);
    toolsMenu->addAction(tr("&Network Manager…"), this, &MainWindow::onNetworkManager);
    toolsMenu->addAction(tr("&Cloud Connections…"), this, &MainWindow::onCloudConnections);
    toolsMenu->addSeparator();
    toolsMenu->addAction(tr("&Control Panel…"), this, &MainWindow::onControlPanel);
    toolsMenu->addAction(tr("P&references…"), this, &MainWindow::onPreferences);

    // ---------- Help menu
    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(tr("&About Serene File Explorer"), this, [this]() {
        QMessageBox::about(this, tr("About Serene File Explorer"),
            tr("<h3>Serene File Explorer</h3>"
               "<p>A feature-rich file manager for SereneOS.</p>"
               "<p>Part of the SereneForge project, GPL-3.0-or-later.</p>"));
    });

    // Toolbar with the most-used actions
    QToolBar *tb = addToolBar(tr("Main"));
    tb->setMovable(false);
    tb->setObjectName(QStringLiteral("mainToolbar"));
    tb->addAction(actNewTab);
    tb->addSeparator();
    tb->addAction(actCut);
    tb->addAction(actCopy);
    tb->addAction(actPaste);
    tb->addSeparator();
    tb->addAction(actNewFolder);
    tb->addAction(actRename);
    tb->addAction(actTrash);
    tb->addSeparator();
    tb->addAction(actReload);
    tb->addAction(actHidden);

    Q_UNUSED(actNewWin)
}

void MainWindow::wireDirectoryView(DirectoryView *view)
{
    connect(view, &DirectoryView::pathChanged, this, [this, view](const QString &p) {
        const int idx = m_tabs->indexOf(view);
        if (idx >= 0)
            m_tabs->setTabText(idx, QDir(p).dirName().isEmpty() ? p : QDir(p).dirName());
        updateActions();
    });
    connect(view, &DirectoryView::selectionChanged, this, &MainWindow::updateActions);
    connect(view, &DirectoryView::contextMenuRequested, this,
            [this](const QPoint &global, const QStringList &sel) {
                QMenu menu(this);
                if (sel.isEmpty()) {
                    menu.addAction(tr("New Folder"), this, &MainWindow::onNewFolder);
                    menu.addAction(tr("Paste"), this, &MainWindow::onPaste);
                    menu.addSeparator();
                    menu.addAction(tr("Open in Terminal"), this, []() {});
                } else {
                    menu.addAction(tr("Cut"), this, &MainWindow::onCut);
                    menu.addAction(tr("Copy"), this, &MainWindow::onCopy);
                    menu.addAction(tr("Rename…"), this, &MainWindow::onRename);
                    menu.addSeparator();
                    menu.addAction(tr("Move to Trash"), this, &MainWindow::onDeleteToTrash);
                    menu.addAction(tr("Delete Permanently"), this, &MainWindow::onDeletePermanent);
                    menu.addSeparator();
                    menu.addAction(tr("Properties"), this, &MainWindow::onProperties);
                }
                menu.exec(global);
            });

    connect(view, &DirectoryView::fileActivated, this, [](const QString &path) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    });
}

void MainWindow::newTab()
{
    auto *view = new DirectoryView(this);
    wireDirectoryView(view);
    const int idx = m_tabs->addTab(view, tr("Loading…"));
    m_tabs->setCurrentIndex(idx);
    view->go(QDir::homePath());
    view->setViewMode(Settings::viewMode());
    view->setShowHidden(Settings::showHidden());
}

void MainWindow::closeTab(int index)
{
    if (m_tabs->count() == 1) {
        // Keep one tab open; window closes via Ctrl+W when last tab is last?
        // Simplest: closing last tab exits.
        close();
        return;
    }
    QWidget *w = m_tabs->widget(index);
    m_tabs->removeTab(index);
    delete w;
}

void MainWindow::closeCurrentTab()
{
    closeTab(m_tabs->currentIndex());
}

void MainWindow::onTabChanged(int index)
{
    Q_UNUSED(index);
    updateActions();
}

// ---------------------------------------------------------------------------
// Edit operations
// ---------------------------------------------------------------------------

void MainWindow::onCut()
{
    if (!currentView())
        return;
    Clipboard::instance().clear();
    Clipboard::instance().mode  = Clipboard::Cut;
    Clipboard::instance().paths = currentView()->selectedPaths();
    updateActions();
}

void MainWindow::onCopy()
{
    if (!currentView())
        return;
    Clipboard::instance().clear();
    Clipboard::instance().mode  = Clipboard::Copy;
    Clipboard::instance().paths = currentView()->selectedPaths();
    updateActions();
}

void MainWindow::onPaste()
{
    if (!currentView())
        return;
    const QString dest = currentView()->path();
    if (Clipboard::instance().isEmpty())
        return;

    const QStringList paths = Clipboard::instance().paths;
    if (Clipboard::instance().mode == Clipboard::Cut) {
        m_transfers->enqueueMove(paths, dest);
        Clipboard::instance().clear();
    } else {
        m_transfers->enqueueCopy(paths, dest);
    }
    updateActions();
}

void MainWindow::onDeleteToTrash()
{
    if (!currentView())
        return;
    const QStringList sel = currentView()->selectedPaths();
    if (sel.isEmpty())
        return;
    QString err;
    if (!FS::moveToTrash(sel, &err))
        QMessageBox::warning(this, tr("Trash failed"), err);
    currentView()->model()->reload();
}

void MainWindow::onDeletePermanent()
{
    if (!currentView())
        return;
    const QStringList sel = currentView()->selectedPaths();
    if (sel.isEmpty())
        return;
    if (QMessageBox::question(this, tr("Delete permanently"),
            tr("Permanently delete %1 item(s)? This cannot be undone.").arg(sel.size()))
        != QMessageBox::Yes)
        return;
    QString err;
    if (!FS::deletePermanent(sel, &err))
        QMessageBox::warning(this, tr("Delete failed"), err);
    currentView()->model()->reload();
}

void MainWindow::onRename()
{
    if (!currentView())
        return;
    const QStringList sel = currentView()->selectedPaths();
    if (sel.isEmpty())
        return;
    const QString oldName = QFileInfo(sel.first()).fileName();
    bool ok = false;
    const QString newName = QInputDialog::getText(this, tr("Rename"),
        tr("New name:"), QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.isEmpty() || newName == oldName)
        return;

    const QString dir = QFileInfo(sel.first()).absolutePath();
    const QString target = dir + QLatin1Char('/') + newName;
    if (QFile::rename(sel.first(), target)) {
        currentView()->model()->reload();
    } else {
        QMessageBox::warning(this, tr("Rename failed"),
                             tr("Could not rename %1").arg(sel.first()));
    }
}

void MainWindow::onNewFolder()
{
    if (!currentView())
        return;
    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("New Folder"),
        tr("Folder name:"), QLineEdit::Normal,
        FS::uniqueChildName(currentView()->path(), tr("New Folder")), &ok);
    if (!ok || name.isEmpty())
        return;
    QString err;
    const QString path = currentView()->path() + QLatin1Char('/') + name;
    if (!FS::createFolder(path, &err))
        QMessageBox::warning(this, tr("Create failed"), err);
    currentView()->model()->reload();
}

void MainWindow::onNewFile()
{
    if (!currentView())
        return;
    bool ok = false;
    const QString name = QInputDialog::getText(this, tr("New File"),
        tr("File name:"), QLineEdit::Normal,
        FS::uniqueChildName(currentView()->path(), tr("untitled.txt")), &ok);
    if (!ok || name.isEmpty())
        return;
    QString err;
    const QString path = currentView()->path() + QLatin1Char('/') + name;
    if (!FS::createEmptyFile(path, &err))
        QMessageBox::warning(this, tr("Create failed"), err);
    currentView()->model()->reload();
}

void MainWindow::onProperties()
{
    if (!currentView())
        return;
    const QStringList sel = currentView()->selectedPaths();
    if (sel.isEmpty())
        return;
    // Milestone 2: dedicated Properties dialog with permissions tab.
    QString info;
    for (const QString &p : sel) {
        const QFileInfo fi(p);
        info += QStringLiteral("%1\n  Type:  %2\n  Size:  %3\n  Modified: %4\n\n")
            .arg(fi.fileName(),
                 fi.isDir() ? tr("Folder") : tr("File"),
                 FS::humanSize(fi.isDir() ? 0 : fi.size()),
                 fi.lastModified().toString(Qt::SystemLocaleLongDate));
    }
    QMessageBox::information(this, tr("Properties"), info.trimmed());
}

void MainWindow::onSelectAll()
{
    if (currentView())
        currentView()->selectAllFiles();
}

void MainWindow::onRefresh()
{
    if (currentView())
        currentView()->model()->reload();
}

void MainWindow::onToggleHidden()
{
    const bool on = !Settings::showHidden();
    Settings::setShowHidden(on);
    if (currentView())
        currentView()->setShowHidden(on);
    updateActions();
}

void MainWindow::onViewModeChanged()
{
    QActionGroup *grp = qobject_cast<QActionGroup *>(sender());
    if (!grp)
        return;
    const QAction *checked = grp->checkedAction();
    if (!checked)
        return;
    const int mode = checked->data().toInt();
    Settings::setViewMode(mode);
    if (currentView())
        currentView()->setViewMode(mode);
}

void MainWindow::onSearchTriggered()
{
    if (currentView())
        currentView()->focusFilter();
}

// Milestone 2+ module launchers - placeholders wired to the Tools menu
void MainWindow::onDriveManager()      {}
void MainWindow::onNetworkManager()    {}
void MainWindow::onCloudConnections()  {}
void MainWindow::onControlPanel()
{
    const QString cmd = Settings::controlPanelCommand();
    if (!QStandardPaths::findExecutable(cmd).isEmpty()) {
        QProcess::startDetached(cmd);
    } else {
        QMessageBox::information(this, tr("Control Panel"),
            tr("'%1' was not found. Install SereneOS Control Panel or set the "
               "command in Preferences.").arg(cmd));
    }
}

void MainWindow::onPreferences()
{
    QMessageBox::information(this, tr("Preferences"),
        tr("Full preferences dialog arrives in milestone 2. "
           "Theme toggle: coming with dark/light switching."));
}

void MainWindow::updateActions()
{
    if (!currentView())
        return;
    const int selCount = currentView()->selectedPaths().size();
    const QString p = currentView()->path();
    m_status->setText(p);
    const FileModel *m = currentView()->model();
    m_itemInfo->setText(tr("%1 item(s)").arg(m->rowCount()));
    if (selCount > 0)
        m_itemInfo->setText(tr("%1 of %2 selected")
                                .arg(selCount).arg(m->rowCount()));
}
