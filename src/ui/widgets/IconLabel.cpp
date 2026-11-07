#include "ui/widgets/IconLabel.h"

#include <QPainter>
#include <QPainterPath>

namespace edu::ui {

IconLabel::IconLabel(const QIcon& icon, int iconSize, QWidget* parent)
    : QWidget(parent), m_icon(icon), m_iconSize(iconSize)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TranslucentBackground);
}

void IconLabel::setIcon(const QIcon& icon)
{
    m_icon = icon;
    update();
}

void IconLabel::setBadge(ColorRole background, int padding, int radius)
{
    m_badge = background;
    m_padding = padding;
    m_radius = radius;
    updateGeometry();
    update();
}

QSize IconLabel::sizeHint() const
{
    const int side = m_iconSize + 2 * m_padding;
    return {side, side};
}

void IconLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRect area = rect();
    if (m_badge) {
        QPainterPath path;
        path.addRoundedRect(QRectF(area), m_radius, m_radius);
        painter.fillPath(path, Theme::instance().color(*m_badge));
    }
    const QRect iconRect(area.center().x() - m_iconSize / 2 + 1,
                         area.center().y() - m_iconSize / 2 + 1, m_iconSize, m_iconSize);
    m_icon.paint(&painter, iconRect, Qt::AlignCenter,
                 isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

} // namespace edu::ui
