#pragma once
/*
 * Serene File Explorer - internal clipboard (cut/copy/paste state)
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QStringList>

class Clipboard
{
public:
    enum Mode { None, Cut, Copy };

    static Clipboard &instance();

    Mode mode = None;
    QStringList paths;

    void clear();
    bool isEmpty() const;
};
