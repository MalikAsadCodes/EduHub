#include "ui/login/BrandPanel.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/UiHelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QPainter>
#include <QVBoxLayout>

namespace edu::ui {

namespace {
const QColor kNavyTop(0x22, 0x44, 0x7C);
const QColor kNavyBottom(0x0B, 0x16, 0x30);
const QColor kGold(0xE2, 0xB9, 0x4B);
} // namespace

BrandPanel::BrandPanel(const QString& institutionName, QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("BrandPanel"));
    setMinimumWidth(340);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(44, 44, 44, 36);
    column->setSpacing(0);

    auto* top = new QHBoxLayout;
    top->setSpacing(12);
    top->addWidget(new IconLabel(Icons::fixed(QStringLiteral("app")), 44, this));
    auto* wordmark = makeLabel(QStringLiteral("Edu Hub"), QStringLiteral("brandWordmark"), this);
    top->addWidget(wordmark);
    top->addStretch(1);
    column->addLayout(top);
    column->addStretch(3);

    auto* headline = makeLabel(QStringLiteral("Campus records,\nall in one place."),
                               QStringLiteral("brandHeadline"), this);
    column->addWidget(headline);
    column->addSpacing(12);
    auto* institution = makeLabel(institutionName, QStringLiteral("brandInstitution"), this);
    institution->setWordWrap(true);
    column->addWidget(institution);
    column->addSpacing(28);

    const QStringList features = {
        QStringLiteral("Courses, enrollment and prerequisites"),
        QStringLiteral("Attendance with shortage alerts"),
        QStringLiteral("Grades, GPA and printable transcripts"),
    };
    for (const QString& feature : features) {
        auto* row = new QHBoxLayout;
        row->setSpacing(10);
        row->setContentsMargins(0, 5, 0, 5);
        auto* tick = new IconLabel(
            Icons::themed(QStringLiteral("check-circle"), ColorRole::RailActiveText), 18, this);
        row->addWidget(tick);
        row->addWidget(makeLabel(feature, QStringLiteral("brandFeature"), this), 1);
        column->addLayout(row);
    }

    column->addStretch(4);
    column->addWidget(
        makeLabel(QStringLiteral("Version %1  \u00B7  Works offline  \u00B7  Your data stays on this PC")
                      .arg(QStringLiteral(EDUHUB_VERSION)),
                  QStringLiteral("brandFooter"), this));
}

void BrandPanel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QLinearGradient gradient(rect().topLeft(), rect().bottomRight());
    gradient.setColorAt(0.0, kNavyTop);
    gradient.setColorAt(1.0, kNavyBottom);
    painter.fillRect(rect(), gradient);

    // Concentric gold rings anchored at the bottom-right corner.
    QColor ring = kGold;
    painter.setBrush(Qt::NoBrush);
    const QPointF corner(width() + 40, height() + 30);
    for (int i = 0; i < 6; ++i) {
        ring.setAlpha(42 - i * 6);
        painter.setPen(QPen(ring, 1.4));
        const qreal radius = 120 + i * 58;
        painter.drawEllipse(corner, radius, radius);
    }

    // Dot grid in the top-right area.
    QColor dot(255, 255, 255, 28);
    painter.setPen(Qt::NoPen);
    painter.setBrush(dot);
    const int startX = width() - 150;
    for (int row = 0; row < 6; ++row) {
        for (int col = 0; col < 6; ++col)
            painter.drawEllipse(QPointF(startX + col * 20, 40 + row * 20), 1.6, 1.6);
    }

    // Thin gold rule on the inner edge.
    painter.fillRect(QRectF(width() - 3, 0, 3, height()),
                     QColor(kGold.red(), kGold.green(), kGold.blue(), 160));
}

} // namespace edu::ui
