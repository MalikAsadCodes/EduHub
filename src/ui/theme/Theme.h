#pragma once

#include <QColor>
#include <QObject>
#include <QString>

namespace edu::ui {

enum class ThemeMode { Light, Dark };

// Every colour the UI uses. The stylesheet refers to these as {{token}}.
enum class ColorRole {
    Background,
    Surface,
    SurfaceAlt,
    Border,
    Text,
    TextMuted,
    Primary,
    PrimaryHover,
    PrimaryText,
    Accent,
    AccentSoft,
    AccentText,
    RailBackground,
    RailHover,
    RailText,
    RailActiveBackground,
    RailActiveText,
    Success,
    Warning,
    Danger,
    Info,
    ToastBackground,
    ToastText,
};

// Applies the Edu Hub look: Fusion style, a matching QPalette, and the
// stylesheet template from :/styles/app.qss with colours filled in.
// Icons read colours from here when they paint, so they follow theme changes.
class Theme : public QObject {
    Q_OBJECT

public:
    static Theme& instance();

    ThemeMode mode() const { return m_mode; }
    void setMode(ThemeMode mode);
    void toggle();

    QColor color(ColorRole role) const;
    static QColor color(ColorRole role, ThemeMode mode);

    // Applies the current mode to the whole application.
    void apply();

    // The finished stylesheet for the current mode (exposed for tests).
    QString styleSheet() const;

    static QString modeName(ThemeMode mode);
    static ThemeMode modeFromName(const QString& name, ThemeMode fallback = ThemeMode::Light);

signals:
    void changed(edu::ui::ThemeMode mode);

private:
    Theme() = default;

    ThemeMode m_mode = ThemeMode::Light;
};

} // namespace edu::ui
