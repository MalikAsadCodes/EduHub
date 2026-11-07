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

// Deep navy and academic gold. Hex is #RRGGBB or #AARRGGBB.
// clang-format off
constexpr Swatch kSwatches[] = {
    {ColorRole::Background,           "background",           "#F3F5FA",   "#0A1222"},
    {ColorRole::Surface,              "surface",              "#FFFFFF",   "#111B30"},
    {ColorRole::SurfaceAlt,           "surfaceAlt",           "#EDF1F7",   "#17233C"},
    {ColorRole::Border,               "border",               "#DCE2EC",   "#24324D"},
    {ColorRole::Text,                 "text",                 "#13203A",   "#E7ECF5"},
    {ColorRole::TextMuted,            "textMuted",            "#5D6B84",   "#94A3BD"},
    {ColorRole::Primary,              "primary",              "#1D3D72",   "#3F6FD0"},
    {ColorRole::PrimaryHover,         "primaryHover",         "#26508F",   "#557FDA"},
    {ColorRole::PrimaryText,          "primaryText",          "#FFFFFF",   "#FFFFFF"},
    {ColorRole::Accent,               "accent",               "#C59A1E",   "#E2B94B"},
    {ColorRole::AccentSoft,           "accentSoft",           "#FBF2D9",   "#2B2514"},
    {ColorRole::AccentText,           "accentText",           "#7A5C05",   "#F0CF77"},
    {ColorRole::RailBackground,       "railBackground",       "#0E1C38",   "#070D1A"},
    {ColorRole::RailHover,            "railHover",            "#182A4D",   "#111C33"},
    {ColorRole::RailText,             "railText",             "#AEBBD3",   "#93A2BD"},
    {ColorRole::RailActiveBackground, "railActiveBackground", "#223A66",   "#17284A"},
    {ColorRole::RailActiveText,       "railActiveText",       "#F2D27A",   "#F2D27A"},
    {ColorRole::Success,              "success",              "#1C8556",   "#3DBE85"},
    {ColorRole::Warning,              "warning",              "#B4770E",   "#E5A93B"},
    {ColorRole::Danger,               "danger",               "#C23A3A",   "#F07171"},
    {ColorRole::Info,                 "info",                 "#2F6FD0",   "#6B9CF0"},
    {ColorRole::ToastBackground,      "toastBackground",      "#13203A",   "#E7ECF5"},
    {ColorRole::ToastText,            "toastText",            "#FFFFFF",   "#0F1A2E"},
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
