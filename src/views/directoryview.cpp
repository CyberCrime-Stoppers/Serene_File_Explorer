#include "directoryview.h"

#include <QLineEdit>
#include <QListView>
#include <QMessageBox>
#include <QDir>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileSystemModel>
#include <QContextMenuEvent>
#include <QMenu>

#include "core/settings.h"
#include "core/types.h"
#include "models/filemodel.h"

DirectoryView::DirectoryView(QWidget *parent)
    : QWidget(parent)
{
    // ----- layout: path bar on top, view below
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    auto *topRow = new QHBoxLayout;
    topRow->setContentsMargins(6, 4, 6, 4);
    topRow->setSpacing(4);

    m_upBtn = new QToolButton(this);
    m_upBtn->setText(QStringLiteral("\u2191")); // ↑ arrow
    m_upBtn->setToolTip(tr("Parent folder"));
    m_upBtn->setAutoRaise(true);
    topRow->addWidget(m_upBtn);

    m_pathEdit = new QLineEdit(this);
    m_pathEdit->setPlaceholderText(tr("Enter a path…"));
    topRow->addWidget(m_pathEdit, /*stretch=*/1);

    m_layout->addLayout(topRow);

    m_fileView = new QListView(this);
    m_fileView->setViewMode(QListView::IconMode);
    m_fileView->setResizeMode(QListView::Adjust);
    m_fileView->setWrapping(true);
    m_fileView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_fileView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_fileView->setUniformItemSizes(true);
    m_fileView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_layout->addWidget(m_fileView, /*stretch=*/1);

    // ----- model
    m_model = new FileModel(this);
    m_fileView->setModel(m_model);
    m_fileView->setModelColumn(0);

    connect(m_fileView, &QListView::doubleClicked,
            this, &DirectoryView::onActivated);
    connect(m_fileView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &DirectoryView::selectionChanged);
    connect(m_fileView, &QListView::customContextMenuRequested,
            this, &DirectoryView::onContextMenu);

    connect(m_pathEdit, &QLineEdit::returnPressed,
            this, &DirectoryView::onPathEditFinished);
    connect(m_upBtn, &QToolButton::clicked, this, [this]() { up(); });

    connect(m_model, &FileModel::loaded,  this, &DirectoryView::onModelLoaded);
    connect(m_model, &FileModel::loadFailed, this, &DirectoryView::onModelFailed);

    connect(this, &DirectoryView::pathChanged, this, &DirectoryView::syncBreadcrumb);
}

QString DirectoryView::path() const
{
    return m_model->path();
}

void DirectoryView::setViewMode(int mode)
{
    m_fileView->setViewMode(mode == Settings::ViewList ? QListView::ListMode
                                                       : QListView::IconMode);
    if (mode == Settings::ViewList)
        m_fileView->setFlow(QListView::TopToBottom);
    else
        m_fileView->setFlow(QListView::LeftToRight);
}

void DirectoryView::setShowHidden(bool on)
{
    m_model->setShowHidden(on);
}

void DirectoryView::focusFilter()
{
    m_pathEdit->setFocus();
    m_pathEdit->selectAll();
}

void DirectoryView::selectAllFiles()
{
    m_fileView->selectAll();
}

void DirectoryView::clearSelection()
{
    m_fileView->clearSelection();
}

QStringList DirectoryView::selectedPaths() const
{
    QStringList out;
    const QModelIndexList sel = m_fileView->selectionModel()->selectedIndexes();
    for (const QModelIndex &idx : sel)
        out << idx.data(FileModel::PathRole).toString();
    return out;
}

void DirectoryView::go(const QString &absPath)
{
    if (absPath.isEmpty())
        return;
    const QString norm = QDir(absPath).absolutePath();
    if (norm == m_model->path())
        return;

    // push to history
    if (m_histIndex >= 0 && m_histIndex + 1 < m_history.size())
        m_history = m_history.mid(0, m_histIndex + 1);
    m_history.append(norm);
    m_histIndex = m_history.size() - 1;

    m_model->setPath(norm);
    emit pathChanged(norm);
}

bool DirectoryView::back()
{
    if (m_histIndex <= 0)
        return false;
    --m_histIndex;
    const QString target = m_history.at(m_histIndex);
    m_model->setPath(target);
    emit pathChanged(target);
    return true;
}

bool DirectoryView::forward()
{
    if (m_histIndex + 1 >= m_history.size())
        return false;
    ++m_histIndex;
    const QString target = m_history.at(m_histIndex);
    m_model->setPath(target);
    emit pathChanged(target);
    return true;
}

bool DirectoryView::up()
{
    const QDir cur(m_model->path());
    if (!cur.exists() || cur.isRoot())
        return false;
    go(cur.absolutePath() + QStringLiteral("/.."));
    return true;
}

bool DirectoryView::canBack() const    { return m_histIndex > 0; }
bool DirectoryView::canForward() const { return m_histIndex + 1 < m_history.size(); }
bool DirectoryView::canUp() const      { return !m_model->path().isEmpty()
                                                && !QDir(m_model->path()).isRoot(); }

void DirectoryView::onPathEditFinished()
{
    QString txt = m_pathEdit->text().trimmed();
    if (txt.isEmpty())
        return;
    if (!QDir::isAbsolutePath(txt))
        txt = QDir::homePath() + QLatin1Char('/') + txt;
    go(txt);
    m_fileView->setFocus();
}

void DirectoryView::onActivated(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    const bool isDir = index.data(FileModel::IsDirRole).toBool();
    const QString p  = index.data(FileModel::PathRole).toString();
    if (isDir) {
        go(p);
        emit folderActivated(p);
    } else {
        emit fileActivated(p);
    }
}

void DirectoryView::onModelLoaded(const QString &path, int count)
{
    Q_UNUSED(count);
    if (path != m_model->path())
        return;
    m_fileView->scrollToTop();
}

void DirectoryView::onModelFailed(const QString &path, const QString &error)
{
    if (path != m_model->path())
        return;
    QMessageBox::warning(this, tr("Cannot open folder"), error);
}

void DirectoryView::onContextMenu(const QPoint &pos)
{
    const QModelIndex idx = m_fileView->indexAt(pos);
    QStringList selected;
    if (idx.isValid())
        selected = selectedPaths();

    emit contextMenuRequested(mapToGlobal(pos), selected);
}

void DirectoryView::syncBreadcrumb()
{
    if (m_pathEdit->hasFocus())
        return;
    m_pathEdit->setText(m_model->path());
    m_upBtn->setEnabled(canUp());
}
