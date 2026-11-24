#include "sidebar.h"

#include <QApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QStyle>
#include <QTreeWidget>
#include <QUrl>
#include <QVBoxLayout>

#include "core/settings.h"

namespace {

enum Role { PathRole = Qt::UserRole + 1 };

QIcon driveIcon(const QStorageInfo &info)
{
    Q_UNUSED(info);
    return QIcon::fromTheme(QStringLiteral("drive-harddisk"),
        QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon));
}

QTreeWidgetItem *makeSection(QTreeWidget *tree, const QString &title,
                             const QFont &baseFont)
{
    auto *section = new QTreeWidgetItem(tree);
    section->setText(0, title);
    QFont f = baseFont;
    f.setBold(true);
    section->setFont(0, f);
    section->setFlags(Qt::ItemIsEnabled); // header rows are not navigable
    return section;
}

} // namespace

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setRootIsDecorated(false);
    m_tree->setIndentation(12);
    m_tree->setFrameShape(QFrame::NoFrame);
    m_tree->setUniformRowHeights(true);
    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(m_tree);

    buildSections();
    refreshDevices();
    reloadBookmarks();
    reloadTags();

    connect(m_tree, &QTreeWidget::itemClicked, this, &Sidebar::onItemClicked);
    connect(m_tree, &QTreeWidget::customContextMenuRequested,
            this, &Sidebar::onContextMenu);
}

void Sidebar::buildSections()
{
    m_tree->clear();
    const QFont base = m_tree->font();

    // ---------- Places ----------
    m_places = makeSection(m_tree, tr("Places"), base);

    struct Place { QString label; QStandardPaths::StandardLocation loc; };
    const Place places[] = {
        { tr("Home"),      QStandardPaths::HomeLocation },
        { tr("Documents"), QStandardPaths::DocumentsLocation },
        { tr("Downloads"), QStandardPaths::DownloadLocation },
        { tr("Pictures"),  QStandardPaths::PicturesLocation },
        { tr("Music"),     QStandardPaths::MusicLocation },
        { tr("Videos"),    QStandardPaths::MoviesLocation },
    };
    for (const Place &p : places) {
        auto *it = new QTreeWidgetItem(m_places);
        it->setText(0, p.label);
        it->setData(0, PathRole, QStandardPaths::writableLocation(p.loc));
    }

    auto *rootIt = new QTreeWidgetItem(m_places);
    rootIt->setText(0, tr("File System"));
    rootIt->setData(0, PathRole, QStringLiteral("/"));

    auto *trashIt = new QTreeWidgetItem(m_places);
    trashIt->setText(0, tr("Trash"));
    trashIt->setData(0, PathRole,
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
        + QStringLiteral("/.local/share/Trash"));

    m_tree->expandItem(m_places);

    // ---------- Devices ----------
    m_devices = makeSection(m_tree, tr("Devices"), base);
    m_tree->expandItem(m_devices);

    // ---------- Bookmarks ----------
    m_bookmarks = makeSection(m_tree, tr("Bookmarks"), base);
    m_tree->expandItem(m_bookmarks);

    // ---------- Tags ----------
    m_tags = makeSection(m_tree, tr("Tags"), base);
    m_tree->expandItem(m_tags);
}

QTreeWidgetItem *Sidebar::sectionItem(int section) const
{
    switch (section) {
    case SecPlaces:    return m_places;
    case SecDevices:   return m_devices;
    case SecBookmarks: return m_bookmarks;
    case SecTags:      return m_tags;
    }
    return nullptr;
}

void Sidebar::refreshDevices()
{
    while (m_devices->childCount())
        delete m_devices->takeChild(0);

    const QList<QStorageInfo> vols = QStorageInfo::mountedVolumes();
    for (const QStorageInfo &v : vols) {
        if (!v.isValid() || !v.isReady() || v.rootPath().isEmpty())
            continue;
        auto *it = new QTreeWidgetItem(m_devices);
        QString label = v.displayName();
        if (label.isEmpty())
            label = v.rootPath();
        if (v.isReadOnly())
            label += tr(" (read-only)");
        it->setText(0, label);
        it->setData(0, PathRole, v.rootPath());
        it->setIcon(0, driveIcon(v));
        it->setToolTip(0, v.rootPath());
    }
}

void Sidebar::reloadBookmarks()
{
    while (m_bookmarks->childCount())
        delete m_bookmarks->takeChild(0);

    const QStringList bookmarks = Settings::bookmarks();
    for (const QString &bm : bookmarks) {
        auto *it = new QTreeWidgetItem(m_bookmarks);
        const QString name = QFileInfo(bm).fileName();
        it->setText(0, name.isEmpty() ? bm : name);
        it->setData(0, PathRole, bm);
        it->setToolTip(0, bm);
    }

    if (m_bookmarks->childCount() == 0)
        m_tree->collapseItem(m_bookmarks);
    else
        m_tree->expandItem(m_bookmarks);
}

void Sidebar::reloadTags()
{
    while (m_tags->childCount())
        delete m_tags->takeChild(0);

    const QStringList tags = Settings::tags();
    for (const QString &tag : tags) {
        auto *it = new QTreeWidgetItem(m_tags);
        it->setText(0, tag);
        it->setFlags(Qt::ItemIsEnabled); // not navigable yet; tag search comes later
        it->setToolTip(0, tr("Tag — searchable once media filtering lands"));
    }
    if (m_tags->childCount() == 0)
        m_tree->collapseItem(m_tags);
    else
        m_tree->expandItem(m_tags);
}

bool Sidebar::addBookmark(const QString &absPath)
{
    QStringList bookmarks = Settings::bookmarks();
    if (bookmarks.contains(absPath))
        return false;
    bookmarks.append(absPath);
    Settings::setBookmarks(bookmarks);
    reloadBookmarks();
    return true;
}

void Sidebar::setCurrentPath(const QString &absPath)
{
    m_currentPath = absPath;
}

void Sidebar::onItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if (!item)
        return;
    const QString path = item->data(0, PathRole).toString();
    if (!path.isEmpty())
        emit placeActivated(path);
}

void Sidebar::onContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = m_tree->itemAt(pos);
    QMenu menu(this);

    // Global: bookmark whichever folder the active tab is showing
    if (!m_currentPath.isEmpty()) {
        menu.addAction(tr("Bookmark Current Folder"), this, [this]() {
            addBookmark(m_currentPath);
        });
        menu.addSeparator();
    }

    if (!item)
        return;

    // Section headers
    if (item == m_tags) {
        menu.addAction(tr("New Tag…"), this, [this]() {
            bool ok = false;
            const QString name = QInputDialog::getText(this, tr("New Tag"),
                tr("Tag name:"), QLineEdit::Normal, QString(), &ok);
            if (!ok || name.isEmpty())
                return;
            QStringList tags = Settings::tags();
            if (!tags.contains(name)) {
                tags.append(name);
                Settings::setTags(tags);
                reloadTags();
            }
        });
        menu.exec(m_tree->viewport()->mapToGlobal(pos));
        return;
    }
    if (item == m_devices) {
        menu.addAction(tr("Refresh Devices"), this, &Sidebar::refreshDevices);
        menu.exec(m_tree->viewport()->mapToGlobal(pos));
        return;
    }

    QTreeWidgetItem *parent = item->parent();
    const QString path = item->data(0, PathRole).toString();

    if (parent == m_bookmarks && !path.isEmpty()) {
        menu.addAction(tr("Remove Bookmark"), this, [this, path]() {
            QStringList bookmarks = Settings::bookmarks();
            bookmarks.removeAll(path);
            Settings::setBookmarks(bookmarks);
            reloadBookmarks();
        });
        menu.addAction(tr("Open in New Tab"), this, [this, path]() {
            emit placeActivated(path); // multi-tab targeting arrives later
        });
    } else if (parent == m_tags) {
        menu.addAction(tr("Delete Tag"), this, [this, item]() {
            QStringList tags = Settings::tags();
            tags.removeAll(item->text(0));
            Settings::setTags(tags);
            reloadTags();
        });
    }

    menu.exec(m_tree->viewport()->mapToGlobal(pos));
}
