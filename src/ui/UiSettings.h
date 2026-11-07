#pragma once

#include "ui/theme/Theme.h"

#include <QByteArray>

namespace edu::ui::UiSettings {

// Per-PC display preferences, stored with QSettings (the Windows registry),
// not in the database, because they belong to this computer and not to the
// institution's records.

ThemeMode themeMode();
void setThemeMode(ThemeMode mode);

bool railCollapsed();
void setRailCollapsed(bool collapsed);

QByteArray mainWindowGeometry();
void setMainWindowGeometry(const QByteArray& geometry);

} // namespace edu::ui::UiSettings
