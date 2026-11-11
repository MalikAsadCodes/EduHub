#include "ui/theme/Theme.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QPalette>
#include <QStyle>
#include <QStyleFactory>

namespace edu::ui {

namespace {

struct Swatch {
    ColorRole role;
    const char* token;
    const char* light;
    const char* dark;
};

// Ink and gilt: warm charcoal surfaces, parchment text and a gold accent.
// The navigation rail stays ink in both modes. Hex is #RRGGBB or #AARRGGBB.
// clang-format off
constexpr Swatch kSwatches[] = {
    {ColorRole::Background,           "background",           "#F1F0EC",   "#16140F"},
    {ColorRole::Surface,              "surface",              "#FFFFFF",   "#1E1B16"},
    {ColorRole::SurfaceAlt,           "surfaceAlt",           "#F5F3EE",   "#27231C"},
    {ColorRole::Border,               "border",               "#DFDBD1",   "#36312A"},
    {ColorRole::Text,                 "text",                 "#1D1A14",   "#EFE8D8"},
    {ColorRole::TextMuted,            "textMuted",            "#696253",   "#A69D8B"},
    {ColorRole::Primary,              "primary",              "#B8892B",   "#D9AE52"},
    {ColorRole::PrimaryHover,         "primaryHover",         "#C99A3A",   "#E6C06A"},
    {ColorRole::PrimaryText,          "primaryText",          "#1B160C",   "#1B160C"},
    {ColorRole::Accent,               "accent",               "#B8892B",   "#D9AE52"},
    {ColorRole::AccentSoft,           "accentSoft",           "#F4EAD2",   "#30291A"},
    {ColorRole::AccentText,           "accentText",           "#7A5810",   "#E8C878"},
    {ColorRole::RailBackground,       "railBackground",       "#17150F",   "#110F0B"},
    {ColorRole::RailHover,            "railHover",            "#231F18",   "#1D1A14"},
    {ColorRole::RailText,             "railText",             "#A9A08D",   "#A39A87"},
    {ColorRole::RailActiveBackground, "railActiveBackground", "#2D2618",   "#2A2416"},
    {ColorRole::RailActiveText,       "railActiveText",       "#ECCB7A",   "#ECCB7A"},
    {ColorRole::Success,              "success",              "#2E7D4F",   "#7BC49A"},
    {ColorRole::Warning,              "warning",              "#A3650A",   "#E3AE4E"},
    {ColorRole::Danger,               "danger",               "#B5402E",   "#E88A78"},
    {ColorRole::Info,                 "info",                 "#2F5F9E",   "#8DB3E2"},
    {ColorRole::ToastBackground,      "toastBackground",      "#1D1A14",   "#EFE8D8"},
    {ColorRole::ToastText,            "toastText",            "#F4EFE3",   "#1B160C"},
    {ColorRole::RoleAdmin,            "roleAdmin",            "#B8892B",   "#D9AE52"},
    {ColorRole::RoleTeacher,          "roleTeacher",          "#2F8073",   "#6FB7A8"},
    {ColorRole::RoleStudent,          "roleStudent",          "#3F65A3",   "#8DB3E2"},
    {ColorRole::Overlay,              "overlay",              "#66231D12", "#A0000000"},
};
// clang-format on

QString cssColor(const QColor& color)
{
    if (color.alpha() == 255)
        return color.name(QColor::HexRgb);
    return QStringLiteral("rgba(%1, %2, %3, %4)")
        .arg(color.red())
        .arg(color.green())
        .arg(color.blue())
        .arg(color.alpha());
}

} // namespace

Theme& Theme::instance()
{
    static Theme theme;
    return theme;
}

QColor Theme::color(ColorRole role, ThemeMode mode)
{
    for (const Swatch& swatch : kSwatches) {
        if (swatch.role == role)
            return QColor(
                QString::fromLatin1(mode == ThemeMode::Dark ? swatch.dark : swatch.light));
    }
    return QColor(Qt::magenta); // a missing swatch should be impossible to miss
}

QColor Theme::color(ColorRole role) const
{
    return color(role, m_mode);
}

void Theme::setMode(ThemeMode mode)
{
    if (mode == m_mode)
        return;
    m_mode = mode;
    apply();
    emit changed(m_mode);
}

void Theme::toggle()
{
    setMode(m_mode == ThemeMode::Light ? ThemeMode::Dark : ThemeMode::Light);
}

QString Theme::styleSheet() const
{
    QFile file(QStringLiteral(":/styles/app.qss"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Theme: stylesheet resource is missing";
        return QString();
    }
    QString css = QString::fromUtf8(file.readAll());
    for (const Swatch& swatch : kSwatches) {
        const QString placeholder = QStringLiteral("{{%1}}").arg(QString::fromLatin1(swatch.token));
        css.replace(placeholder, cssColor(color(swatch.role)));
    }
    if (css.contains(QStringLiteral("{{")))
        qWarning() << "Theme: stylesheet has an unknown {{token}}";
    return css;
}

void Theme::apply()
{
    auto* app = qobject_cast<QApplication*>(QCoreApplication::instance());
    if (!app)
        return;

    // Fusion draws stylesheets the same way on every Windows version.
    if (QApplication::style()->name().compare(QStringLiteral("fusion"), Qt::CaseInsensitive) != 0) {
        if (QStyle* fusion = QStyleFactory::create(QStringLiteral("Fusion")))
            QApplication::setStyle(fusion);
    }

    QPalette palette;
    palette.setColor(QPalette::Window, color(ColorRole::Background));
    palette.setColor(QPalette::WindowText, color(ColorRole::Text));
    palette.setColor(QPalette::Base, color(ColorRole::Surface));
    palette.setColor(QPalette::AlternateBase, color(ColorRole::SurfaceAlt));
    palette.setColor(QPalette::Text, color(ColorRole::Text));
    palette.setColor(QPalette::Button, color(ColorRole::Surface));
    palette.setColor(QPalette::ButtonText, color(ColorRole::Text));
    palette.setColor(QPalette::Highlight, color(ColorRole::Primary));
    palette.setColor(QPalette::HighlightedText, color(ColorRole::PrimaryText));
    palette.setColor(QPalette::ToolTipBase, color(ColorRole::ToastBackground));
    palette.setColor(QPalette::ToolTipText, color(ColorRole::ToastText));
    palette.setColor(QPalette::PlaceholderText, color(ColorRole::TextMuted));
    palette.setColor(QPalette::Link, color(ColorRole::Info));
    palette.setColor(QPalette::Disabled, QPalette::Text, color(ColorRole::TextMuted));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, color(ColorRole::TextMuted));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, color(ColorRole::TextMuted));
    QApplication::setPalette(palette);

    app->setStyleSheet(styleSheet());
}

QString Theme::modeName(ThemeMode mode)
{
    return mode == ThemeMode::Dark ? QStringLiteral("dark") : QStringLiteral("light");
}

ThemeMode Theme::modeFromName(const QString& name, ThemeMode fallback)
{
    if (name.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0)
        return ThemeMode::Dark;
    if (name.compare(QStringLiteral("light"), Qt::CaseInsensitive) == 0)
        return ThemeMode::Light;
    return fallback;
}

} // namespace edu::ui
