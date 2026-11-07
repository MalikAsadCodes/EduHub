#pragma once

#include "ui/theme/Theme.h"

#include <QIcon>
#include <QWidget>

#include <optional>

namespace edu::ui {

// Paints a themed icon, optionally on a rounded "badge" background.
// Unlike a QLabel pixmap, it repaints in the new colours after a theme change.
class IconLabel : public QWidget {
    Q_OBJECT

public:
    explicit IconLabel(const QIcon& icon, int iconSize = 20, QWidget* parent = nullptr);

    void setIcon(const QIcon& icon);
    void setBadge(ColorRole background, int padding, int radius);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QIcon m_icon;
    int m_iconSize;
    std::optional<ColorRole> m_badge;
    int m_padding = 0;
    int m_radius = 0;
};

} // namespace edu::ui
