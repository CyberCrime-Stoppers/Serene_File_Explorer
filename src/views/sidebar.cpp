#include "sidebar.h"

#include <QDir>
#include <QListWidget>
#include <QMenu>
#include <QSettings>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

Sidebar::Sidebar(QWidget *parent)
    : QWidget(parent)
{
    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_list = new QListWidget(this);
    m_list->setUniformItemSizes(false);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setWordWrap(false);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    rootLayout->addWidget(m_list, /*stretch=*/1);

    // Bottom button row: Add / Remove bookmark
    auto *btnRow = new QHBoxLayout;
    btnRow->setContentsMargins(4, 4, 4, 4);
    m_addBtn = new QToolButton(this);
    m_addBtn->setText(QStringLiteral("+"));
    m_addBtn->setToolTip(tr("Bookmark current folder"));
    m_removeBtn = new QToolButton(this);
    m_removeBtn->setText(QStringLiteral("\u2212")); // − minus sign
    m_removeBtn->setToolTip(tr("Remove selected bookmark"));
    btnRow->addWidget(m_addBtn);
    btnRow->addWidget(m_removeBtn);
    btnRow->addStretch(1);
    auto *btns = new QWidget(this);
    btns->setLayout(btnRow);
    rootLayout->addWidget(btns);

    buildStatic();
    reloadBookmarks();
    refreshDevices();

    connect(m_list, &QListWidget::itemActivated, this, &Sidebar::onItemActivated);
    connect(m_list, &QListWidget::customContextMenuRequested,
            this, &Sidebar::onContextMenu);
    connect(m_addBtn, &QToolButton::clicked, this, &Sidebar::onAddBookmark);
    connect(m_removeBtn, &QToolButton::clicked, this, &Sidebar::onRemoveBookmark);
}

void Sidebar::addItem(int section, const QString &title, const QString &path,
                      const QString &iconName, const QString &tooltip)
{
    auto *item = new QListWidgetItem(title, m_list);
    item->setData(Qt::UserRole, path);
    item->setData(Qt::UserRole + 1, section);
    if (!iconName.isEmpty())
        item->setIcon(QIcon::fromTheme(iconName));
    item->setToolTip(tooltip.isEmpty() ? path : tooltip);
}

void Sidebar::buildStatic()
{
    m_list->clear();

    // --- Places ---
    addItem(SecPlaces, tr("Home"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
            QStringLiteral("user-home"));
    addItem(SecPlaces, tr("Documents"),
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
            QStringLiteral("folder-documents"));
    addItem(SecPlaces, tr("Downloads"),
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
            QStringLiteral("folder-downloads"));
    addItem(SecPlaces, tr("Pictures"),
            QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
            QStringLiteral("folder-pictures"));
    addItem(SecPlaces, tr("Music"),
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
            QStringLiteral("folder-music"));
    addItem(SecPlaces, tr("Videos"),
            QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
            QStringLiteral("folder-videos"));
    addItem(SecPlaces, tr("Trash"),
            QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                + QStringLiteral("/.local/share/Trash"),
            QStringLiteral("user-trash"));
}

void Sidebar::reloadBookmarks()
{
    // Remove previous bookmark rows, then re-add from settings
    for (int i = m_list->count() - 1; i >= 0; --i) {
        if (m_list->item(i)->data(Qt::UserRole + 1).toInt() == SecBookmarks)
            delete m_list->takeItem(i);
    }

    QSettings s(QStringLiteral("SereneForge"), QStringLiteral("SereneFileExplorer"));
    const int count = s.beginReadArray(QStringLiteral("bookmarks"));
    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        addItem(SecBookmarks, s.value(QStringLiteral("name")).toString(),
                s.value(QStringLiteral("path")).toString(),
                QStringLiteral("folder-bookmark"));
    }
    s.endArray();
}

void Sidebar::refreshDevices()
{
    // Remove previous device rows, then re-add from QStorageInfo
    for (int i = m_list->count() - 1; i >= 0; --i) {
        if (m_list->item(i)->data(Qt::UserRole + 1).toInt() == SecDevices)
            delete m_list->takeItem(i);
    }

    int index = 0;
    for (const QStorageInfo &vol : QStorageInfo::mountedVolumes()) {
        if (vol.isRoot() || vol.displayName().isEmpty())
            continue;
        QString label = vol.displayName();
        QString tip = tr("%1 — mounted at %2").arg(label, vol.rootPath());
        addItem(SecDevices, label, vol.rootPath(),
                QStringLiteral("drive-removable-media"), tip);
        ++index;
        if (index >= 12) // cap the list
            break;
    }
}

void Sidebar::setCurrentPath(const QString &path)
{
    m_currentPath = path;

    // Exact match first
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem *it = m_list->item(i);
        if (it->data(Qt::UserRole).toString() == path) {
            m_list->setCurrentRow(i);
            return;
        }
    }
    // Fall back to longest-prefix device match
    for (int i = 0; i < m_list->count(); ++i) {
        QListWidgetItem *it = m_list->item(i);
        const QString p = it->data(Qt::UserRole).toString();
        if (!p.isEmpty() && path.startsWith(p + QLatin1Char('/'))
            && it->data(Qt::UserRole + 1).toInt() == SecDevices) {
            m_list->setCurrentRow(i);
            return;
        }
    }
    m_list->setCurrentRow(-1);
}

void Sidebar::onItemActivated(QListWidgetItem *item)
{
    if (!item)
        return;
    const QString p = item->data(Qt::UserRole).toString();
    if (!p.isEmpty())
        emit locationRequested(p);
}

void Sidebar::onAddBookmark()
{
    if (m_currentPath.isEmpty())
        return;
    QSettings s(QStringLiteral("SereneForge"), QStringLiteral("SereneFileExplorer"));

    // Load existing bookmarks
    const int count = s.beginReadArray(QStringLiteral("bookmarks"));
    QList<QPair<QString, QString>> existing;
    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        existing.append({ s.value(QStringLiteral("name")).toString(),
                          s.value(QStringLiteral("path")).toString() });
    }
    s.endArray();

    // Skip duplicates
    for (const auto &bm : existing) {
        if (bm.second == m_currentPath)
            return;
    }
    existing.append({ QDir(m_currentPath).dirName(), m_currentPath });

    s.remove(QStringLiteral("bookmarks"));
    s.beginWriteArray(QStringLiteral("bookmarks"));
    for (int i = 0; i < existing.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue(QStringLiteral("name"), existing[i].first);
        s.setValue(QStringLiteral("path"), existing[i].second);
    }
    s.endArray();
    reloadBookmarks();
}

void Sidebar::onRemoveBookmark()
{
    QListWidgetItem *cur = m_list->currentItem();
    if (!cur || cur->data(Qt::UserRole + 1).toInt() != SecBookmarks)
        return;

    QSettings s(QStringLiteral("SereneForge"), QStringLiteral("SereneFileExplorer"));
    const int count = s.beginReadArray(QStringLiteral("bookmarks"));
    QList<QPair<QString, QString>> keep;
    for (int i = 0; i < count; ++i) {
        s.setArrayIndex(i);
        const QString p = s.value(QStringLiteral("path")).toString();
        if (p != cur->data(Qt::UserRole).toString())
            keep.append({ s.value(QStringLiteral("name")).toString(), p });
    }
    s.endArray();

    s.remove(QStringLiteral("bookmarks"));
    s.beginWriteArray(QStringLiteral("bookmarks"));
    for (int i = 0; i < keep.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue(QStringLiteral("name"), keep[i].first);
        s.setValue(QStringLiteral("path"), keep[i].second);
    }
    s.endArray();
    reloadBookmarks();
}

void Sidebar::onContextMenu(const QPoint &pos)
{
    QListWidgetItem *it = m_list->itemAt(pos);
    QMenu menu(this);
    if (it && it->data(Qt::UserRole + 1).toInt() == SecBookmarks) {
        menu.addAction(tr("Remove bookmark"), this, &Sidebar::onRemoveBookmark);
    } else {
        menu.addAction(tr("Refresh devices"), this, &Sidebar::refreshDevices);
    }
    menu.exec(m_list->viewport()->mapToGlobal(pos));
}
