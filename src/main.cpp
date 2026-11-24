/*
 * Serene File Explorer - entry point
 *
 * Serene File Explorer — a feature-rich, Qt6 file manager for SereneOS.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>

#include "ui/mainwindow.h"
#include "ui/theme.h"

#ifndef APP_VERSION
#define APP_VERSION "0.1.0-alpha"
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Serene File Explorer"));
    QApplication::setOrganizationName(QStringLiteral("SereneForge"));
    QApplication::setApplicationVersion(QStringLiteral(APP_VERSION));
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("folder"),
                                                 QIcon(":/icons/serene-folder.png")));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Serene File Explorer — feature-rich file manager for SereneOS"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("directory"),
                                 QStringLiteral("Directory to open"));
    parser.process(app);

    QString startDir;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty())
        startDir = args.first();

    Theme::apply();

    MainWindow win;
    win.show();
    if (!startDir.isEmpty())
        win.goToUrl(startDir);

    return QApplication::exec();
}
