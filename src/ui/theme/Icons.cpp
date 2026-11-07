#include "ui/theme/Icons.h"

#include <QFile>
#include <QHash>
#include <QIconEngine>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>

#include <cmath>
#include <utility>

namespace edu::ui::Icons {

namespace {

QByteArray loadSvg(const QString& name)
{
    static QHash<QString, QByteArray> cache;
    const auto it = cache.constFind(name);
    if (it != cache.constEnd())
        return *it;
    QFile file(QStringLiteral(":/icons/%1.svg").arg(name));
    QByteArray data;
    if (file.open(QIODevice::ReadOnly))
        data = file.readAll();
    else
        qWarning("Icons: missing icon '%s'", qPrintable(name));
    cache.insert(name, data);
    return data;
}

class SvgIconEngine final : public QIconEngine {
public:
    SvgIconEngine(QString name, std::optional<ColorRole> offRole, std::optional<ColorRole> onRole)
        : m_name(std::move(name)), m_offRole(offRole), m_onRole(onRole)
    {
    }

    QIconEngine* clone() const override { return new SvgIconEngine(*this); }

    QString key() const override { return QStringLiteral("EduHubSvg"); }

    QSize actualSize(const QSize& size, QIcon::Mode, QIcon::State) override { return size; }

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
    {
        const qreal scale = painter->device() ? painter->device()->devicePixelRatioF() : 1.0;
        painter->drawPixmap(rect, scaledPixmap(rect.size(), mode, state, scale));
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        return scaledPixmap(size, mode, state, 1.0);
    }

    QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state,
                         qreal scale) override
    {
        const QSize physical(qMax(1, int(std::lround(size.width() * scale))),
                             qMax(1, int(std::lround(size.height() * scale))));
        QByteArray svg = loadSvg(m_name);

        QString colorKey;
        qreal opacity = (mode == QIcon::Disabled) ? 0.45 : 1.0;
        if (m_offRole) {
            ColorRole role = *m_offRole;
            if (state == QIcon::On && m_onRole)
                role = *m_onRole;
            const QColor color = Theme::instance().color(role);
            opacity *= color.alphaF();
            colorKey = color.name(QColor::HexRgb);
            svg.replace("currentColor", colorKey.toLatin1());
        }

        const QString cacheKey = QStringLiteral("%1|%2x%3|%4|%5|%6")
                                     .arg(m_name)
                                     .arg(physical.width())
                                     .arg(physical.height())
                                     .arg(colorKey)
                                     .arg(opacity)
                                     .arg(scale);
        const auto cached = m_cache.constFind(cacheKey);
        if (cached != m_cache.constEnd())
            return *cached;

        QPixmap pixmap(physical);
        pixmap.fill(Qt::transparent);
        QSvgRenderer renderer(svg);
        if (renderer.isValid()) {
            QPainter painter(&pixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setOpacity(opacity);
            renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(physical)));
        }
        pixmap.setDevicePixelRatio(scale);
        m_cache.insert(cacheKey, pixmap);
        return pixmap;
    }

private:
    QString m_name;
    std::optional<ColorRole> m_offRole; // nullopt = keep the SVG's own colours
    std::optional<ColorRole> m_onRole;
    QHash<QString, QPixmap> m_cache;
};

} // namespace

QIcon themed(const QString& name, ColorRole offRole, std::optional<ColorRole> onRole)
{
    return QIcon(new SvgIconEngine(name, offRole, onRole));
}

QIcon fixed(const QString& name)
{
    return QIcon(new SvgIconEngine(name, std::nullopt, std::nullopt));
}

} // namespace edu::ui::Icons
