#pragma once
/*
 * Serene File Explorer - persistent settings wrapper
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QString>

namespace Settings {

enum SortMode   { SortName = 0, SortSize = 1, SortModified = 2 };
enum ViewMode   { ViewList = 0, ViewIcons = 1 };
enum SearchMode { SearchAuto = 0, SearchLocate = 1, SearchScan = 2 };
enum Theme      { ThemeLight = 0, ThemeDark = 1 };

bool    showHidden();
void    setShowHidden(bool on);

int     sortMode();
void    setSortMode(int mode);

int     viewMode();
void    setViewMode(int mode);

int     theme();
void    setTheme(int themeIdx);

int     searchBackend();
void    setSearchBackend(int mode);

QString controlPanelCommand();
void    setControlPanelCommand(const QString &cmd);

// Sidebar bookmarks: absolute paths, ordered
QStringList bookmarks();
void        setBookmarks(const QStringList &paths);

// Tags: simple label list (per-file tagging arrives with media work)
QStringList tags();
void        setTags(const QStringList &tags);

} // namespace Settings
