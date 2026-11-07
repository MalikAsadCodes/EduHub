#include "ui/pages/EmptyStatePage.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/UiHelpers.h"

#include <QLabel>
#include <QVBoxLayout>

namespace edu::ui {

EmptyStatePage::EmptyStatePage(const QString& iconName, const QString& title,
                               const QString& message, const QString& milestone, QWidget* parent)
    : QWidget(parent)
{
    setProperty("page", true);
    setAttribute(Qt::WA_StyledBackground);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(40, 40, 40, 60);
    column->setSpacing(10);
    column->addStretch(1);

    auto* badge = new IconLabel(Icons::themed(iconName, ColorRole::Primary), 34, this);
    badge->setBadge(ColorRole::SurfaceAlt, 20, 26);
    column->addWidget(badge, 0, Qt::AlignHCenter);
    column->addSpacing(8);

    auto* heading = makeLabel(title, QStringLiteral("emptyTitle"), this);
    heading->setAlignment(Qt::AlignCenter);
    column->addWidget(heading);

    auto* text = makeLabel(message, QStringLiteral("muted"), this);
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);
    // Wrapped labels report a one-line height to centred layouts, so reserve
    // the real height for this width up front.
    text->setFixedWidth(440);
    text->ensurePolished();
    text->setMinimumHeight(text->heightForWidth(440));
    column->addWidget(text, 0, Qt::AlignHCenter);

    column->addSpacing(6);
    auto* pill =
        makeLabel(QStringLiteral("Coming in %1").arg(milestone), QStringLiteral("pill"), this);
    column->addWidget(pill, 0, Qt::AlignHCenter);
    column->addStretch(2);
}

} // namespace edu::ui
