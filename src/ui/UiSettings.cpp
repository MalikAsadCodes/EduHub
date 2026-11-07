#include "ui/UiSettings.h"

#include <QSettings>

namespace edu::ui::UiSettings {

namespace {
const QString kTheme = QStringLiteral("ui/theme");
const QString kRailCollapsed = QStringLiteral("ui/railCollapsed");
const QString kMainGeometry = QStringLiteral("ui/mainWindowGeometry");
} // namespace

ThemeMode themeMode()
{
    return Theme::modeFromName(QSettings().value(kTheme).toString(), ThemeMode::Light);
}

void setThemeMode(ThemeMode mode)
{
    QSettings().setValue(kTheme, Theme::modeName(mode));
}

bool railCollapsed()
{
    return QSettings().value(kRailCollapsed, false).toBool();
}

void setRailCollapsed(bool collapsed)
{
    QSettings().setValue(kRailCollapsed, collapsed);
}

QByteArray mainWindowGeometry()
{
    return QSettings().value(kMainGeometry).toByteArray();
}

void setMainWindowGeometry(const QByteArray& geometry)
{
    QSettings().setValue(kMainGeometry, geometry);
}

} // namespace edu::ui::UiSettings
