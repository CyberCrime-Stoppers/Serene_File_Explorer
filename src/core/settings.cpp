#include "settings.h"

#include <QSettings>

namespace Settings {

static QSettings &store()
{
    static QSettings s(QStringLiteral("SereneForge"),
                      QStringLiteral("SereneFileExplorer"));
    return s;
}

bool showHidden()
{
    return store().value(QStringLiteral("view/showHidden"), false).toBool();
}

void setShowHidden(bool on)
{
    store().setValue(QStringLiteral("view/showHidden"), on);
}

int sortMode()
{
    return store().value(QStringLiteral("view/sortMode"), SortName).toInt();
}

void setSortMode(int mode)
{
    store().setValue(QStringLiteral("view/sortMode"), mode);
}

int viewMode()
{
    return store().value(QStringLiteral("view/viewMode"), ViewIcons).toInt();
}

void setViewMode(int mode)
{
    store().setValue(QStringLiteral("view/viewMode"), mode);
}

int theme()
{
    return store().value(QStringLiteral("look/theme"), ThemeDark).toInt();
}

void setTheme(int themeIdx)
{
    store().setValue(QStringLiteral("look/theme"), themeIdx);
}

int searchBackend()
{
    return store().value(QStringLiteral("search/backend"), SearchAuto).toInt();
}

void setSearchBackend(int mode)
{
    store().setValue(QStringLiteral("search/backend"), mode);
}

QString controlPanelCommand()
{
    return store().value(QStringLiteral("integration/controlPanelCommand"),
                         QStringLiteral("sereneos-control-panel")).toString();
}

void setControlPanelCommand(const QString &cmd)
{
    store().setValue(QStringLiteral("integration/controlPanelCommand"), cmd);
}

} // namespace Settings
