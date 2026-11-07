#pragma once

#include "ui/theme/Theme.h"

#include <QIcon>
#include <QString>

#include <optional>

namespace edu::ui::Icons {

// A themed icon from :/icons/<name>.svg. The SVG's "currentColor" is painted
// with the given colour role at paint time, so the icon recolours itself when
// the theme changes and stays sharp at any display scaling.
//   offRole: normal colour
//   onRole:  colour when the button is checked (defaults to offRole)
QIcon themed(const QString& name, ColorRole offRole = ColorRole::Text,
             std::optional<ColorRole> onRole = std::nullopt);

// An SVG drawn with its own colours (the app mark).
QIcon fixed(const QString& name);

} // namespace edu::ui::Icons
