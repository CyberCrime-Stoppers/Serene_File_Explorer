#include "theme.h"

#include <QApplication>
#include "core/settings.h"

namespace Theme {

QString styleSheet(int theme)
{
    const bool dark = (theme == Dark);
    const QString bg         = dark ? QStringLiteral("#23262b") : QStringLiteral("#f5f7f4");
    const QString surface    = dark ? QStringLiteral("#2b3036") : QStringLiteral("#ffffff");
    const QString surfaceAlt = dark ? QStringLiteral("#333a41") : QStringLiteral("#eef1ec");
    const QString text       = dark ? QStringLiteral("#e6e9e6") : QStringLiteral("#243024");
    const QString dim        = dark ? QStringLiteral("#9aa39c") : QStringLiteral("#66756a");
    const QString accent     = QStringLiteral("#ad5ab4");
    const QString accentSoft = dark ? QStringLiteral("#46304d") : QStringLiteral("#f1e3f2");
    const QString border     = dark ? QStringLiteral("#3a4046") : QStringLiteral("#dde3dc");

    QString css = QStringLiteral(
        "QWidget { background: %1; color: %2; font-size: 10pt; }"
        "QToolBar { background: %3; border: none; border-bottom: 1px solid %4; padding: 3px; spacing: 3px; }"
        "QToolButton { background: transparent; border: none; border-radius: 6px; padding: 5px; color: %2; }"
        "QToolButton:hover { background: %5; }"
        "QToolButton:pressed, QToolButton:checked { background: %6; }"
        "QLineEdit { background: %3; border: 1px solid %4; border-radius: 6px; padding: 4px 8px;"
        " selection-background-color: %6; }"
        "QLineEdit:focus { border: 1px solid %6; }"
        "QListWidget { background: %3; border: none; outline: 0; }"
        "QListWidget::item { border-radius: 6px; padding: 5px 8px; color: %2; }"
        "QListWidget::item:hover { background: %5; }"
        "QListWidget::item:selected { background: %6; color: #ffffff; }"
        "QListView, QTreeView { background: %1; border: none; outline: 0; }"
        "QListView::item, QTreeView::item { border-radius: 8px; padding: 6px; color: %2; }"
        "QListView::item:hover, QTreeView::item:hover { background: %5; }"
        "QListView::item:selected, QTreeView::item:selected { background: %6; color: #ffffff; }"
        "QTabWidget::pane { border: none; }"
        "QTabBar::tab { background: %3; color: %8; padding: 6px 14px; border-top-left-radius: 6px;"
        " border-top-right-radius: 6px; margin-right: 2px; }"
        "QTabBar::tab:selected { background: %1; color: %2; border-top: 2px solid %6; }"
        "QTabBar::tab:hover { background: %5; }"
        "QStatusBar { background: %3; border-top: 1px solid %4; color: %8; }"
        "QMenu { background: %3; border: 1px solid %4; border-radius: 8px; padding: 4px; }"
        "QMenu::item { padding: 6px 24px 6px 12px; border-radius: 6px; }"
        "QMenu::item:selected { background: %6; color: #ffffff; }"
        "QMenu::separator { height: 1px; background: %4; margin: 4px 8px; }"
        "QProgressBar { background: %5; border: none; border-radius: 5px; height: 10px;"
        " text-align: center; color: %2; }"
        "QProgressBar::chunk { background: %6; border-radius: 5px; }"
        "QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }"
        "QScrollBar::handle:vertical { background: %8; border-radius: 5px; min-height: 30px; }"
        "QScrollBar::handle:vertical:hover { background: %6; }"
        "QScrollBar:horizontal { background: transparent; height: 10px; margin: 0; }"
        "QScrollBar::handle:horizontal { background: %8; border-radius: 5px; min-width: 30px; }"
        "QScrollBar::handle:horizontal:hover { background: %6; }"
        "QScrollBar::add-line, QScrollBar::sub-line { height: 0; width: 0; }"
        "QToolTip { background: %3; color: %2; border: 1px solid %4; padding: 4px; }"
        "QDialog { background: %1; }"
        "QPushButton { background: %3; border: 1px solid %4; border-radius: 6px; padding: 5px 14px; }"
        "QPushButton:hover { background: %5; }"
        "QPushButton:default { background: %6; color: #ffffff; border: none; }"
        "QCheckBox { spacing: 7px; }"
        "QComboBox { background: %3; border: 1px solid %4; border-radius: 6px; padding: 3px 8px; }"
        "QLabel#dim { color: %8; }"
    );

    return css.arg(bg, text, surface, border, surfaceAlt, accent, accentSoft, dim);
}

void apply()
{
    qApp->setStyleSheet(styleSheet(Settings::theme()));
}

} // namespace Theme
