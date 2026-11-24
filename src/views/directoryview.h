#pragma once
/*
 * Serene File Explorer - per-tab directory view widget
 * Combines: breadcrumb path bar, async file model, list/icon file views,
 * selection handling, and inline search filtering.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QWidget>

#include "core/types.h"

class QLineEdit;
class QListView;
class QStackedWidget;
class QToolButton;
class QVBoxLayout;
class FileModel;
class QStandardItemModel;

class DirectoryView : public QWidget
{
    Q_OBJECT
public:
    explicit DirectoryView(QWidget *parent = nullptr);

    QString path() const;
    FileModel *model() const { return m_model; }

    void setViewMode(int mode);       // Settings::ViewMode
    void setShowHidden(bool on);
    void focusFilter();

    void selectAllFiles();
    void clearSelection();
    QStringList selectedPaths() const;

    /// Navigation
    void go(const QString &absPath);
    bool back();
    bool forward();
    bool up();

    bool canBack() const;
    bool canForward() const;
    bool canUp() const;

signals:
    void pathChanged(const QString &absPath);
    void selectionChanged();
    void folderActivated(const QString &absPath);
    void fileActivated(const QString &absPath);
    void contextMenuRequested(const QPoint &globalPos, const QStringList &selected);

private slots:
    void onPathEditFinished();
    void onActivated(const QModelIndex &index);
    void onModelLoaded(const QString &path, int count);
    void onModelFailed(const QString &path, const QString &error);
    void onContextMenu(const QPoint &pos);

private:
    void rebuildHistoryForNewRoot(const QString &absPath);
    void syncBreadcrumb();

    // History (simple per-tab stack)
    QStringList m_history;
    int         m_histIndex = -1;

    // UI
    QLineEdit     *m_pathEdit   = nullptr;
    class QToolButton *m_upBtn  = nullptr;
    QListView      *m_fileView  = nullptr;
    QVBoxLayout    *m_layout    = nullptr;

    // Model
    FileModel *m_model = nullptr;
};
