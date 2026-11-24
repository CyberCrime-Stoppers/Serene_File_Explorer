#pragma once
/*
 * Serene File Explorer - "Serene" theme (green/purple palette)
 * Colors follow the shared Serene web theme: #9db29a (green) / #ad5ab4 (purple).
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QString>

namespace Theme {

enum { Light = 0, Dark = 1 };

QString styleSheet(int theme);
void    apply();          // reads Settings::theme() and applies to qApp

} // namespace Theme
