#include "ui/widgets/StatCard.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/UiHelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace edu::ui {

StatCard::StatCard(const QString& label, const QString& iconName, ColorRole tint, QWidget* parent)
    : QFrame(parent)
{
    setProperty("card", true);
    setMinimumWidth(150);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(18, 16, 18, 16);
    column->setSpacing(6);

    auto* top = new QHBoxLayout;
    top->setSpacing(8);
    top->addWidget(makeLabel(label, QStringLiteral("statLabel"), this), 1);
    auto* badge = new IconLabel(Icons::themed(iconName, tint), 18, this);
    badge->setBadge(ColorRole::SurfaceAlt, 7, 9);
    top->addWidget(badge, 0, Qt::AlignTop);
    column->addLayout(top);

    m_value = makeLabel(QStringLiteral("-"), QStringLiteral("statValue"), this);
    column->addWidget(m_value);
    m_caption = makeLabel(QString(), QStringLiteral("statCaption"), this);
    m_caption->setWordWrap(true);
    column->addWidget(m_caption);
    column->addStretch(1);
}

void StatCard::setValue(const QString& value, const QString& caption)
{
    m_value->setText(value);
    m_caption->setText(caption);
}

QString StatCard::value() const
{
    return m_value->text();
}

} // namespace edu::ui
