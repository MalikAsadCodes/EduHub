#include "ui/widgets/NavRail.h"

#include "ui/theme/Icons.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/UiHelpers.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QVBoxLayout>

#include <utility>

namespace edu::ui {

namespace {
constexpr int kSideMargin = 12;
constexpr int kBrandIcon = 34;
} // namespace

// A rail link. Draws a short gold bar on its left edge while selected.
class NavButton : public QPushButton {
public:
    NavButton(const QString& text, const QIcon& icon, QWidget* parent = nullptr)
        : QPushButton(text, parent), m_text(text)
    {
        setIcon(icon);
        setIconSize(QSize(20, 20));
        setProperty("nav", true);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::TabFocus); // focus ring only for keyboard users
        setMinimumHeight(42);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        setAccessibleName(text);
    }

    void setLabel(const QString& text)
    {
        m_text = text;
        setAccessibleName(text);
        setCollapsed(property("collapsed").toBool());
    }

    void setCollapsed(bool collapsed)
    {
        setText(collapsed ? QString() : m_text);
        setToolTip(collapsed ? m_text : QString());
        setStyleProperty(this, "collapsed", collapsed);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        QPushButton::paintEvent(event);
        if (!isChecked())
            return;
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        QPainterPath bar;
        const qreal barHeight = 18.0;
        bar.addRoundedRect(QRectF(0, (height() - barHeight) / 2.0, 3.5, barHeight), 1.75, 1.75);
        painter.fillPath(bar, Theme::instance().color(ColorRole::Accent));
    }

private:
    QString m_text;
};

NavRail::NavRail(QWidget* parent) : QFrame(parent)
{
    setObjectName(QStringLiteral("NavRail"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setFixedWidth(kExpandedWidth);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(kSideMargin, 18, kSideMargin, 16);
    outer->setSpacing(2);

    // Brand
    m_brandRow = new QHBoxLayout;
    m_brandRow->setContentsMargins(4, 0, 0, 0);
    m_brandRow->setSpacing(10);
    auto* mark = new IconLabel(Icons::fixed(QStringLiteral("app")), kBrandIcon, this);
    m_brandRow->addWidget(mark);
    m_brandText = new QWidget(this);
    auto* textColumn = new QVBoxLayout(m_brandText);
    textColumn->setContentsMargins(0, 0, 0, 0);
    textColumn->setSpacing(0);
    auto* name = new QLabel(QStringLiteral("Edu Hub"), m_brandText);
    name->setObjectName(QStringLiteral("RailBrandName"));
    auto* tag = new QLabel(QStringLiteral("University suite"), m_brandText);
    tag->setObjectName(QStringLiteral("RailBrandTag"));
    textColumn->addWidget(name);
    textColumn->addWidget(tag);
    m_brandRow->addWidget(m_brandText);
    m_brandRow->addStretch(1);
    outer->addLayout(m_brandRow);
    outer->addSpacing(14);

    // Page links
    m_items = new QVBoxLayout;
    m_items->setContentsMargins(0, 0, 0, 0);
    m_items->setSpacing(2);
    outer->addLayout(m_items);
    outer->addStretch(1);

    auto* divider = new QFrame(this);
    divider->setObjectName(QStringLiteral("RailDivider"));
    divider->setFixedHeight(1);
    outer->addWidget(divider);
    outer->addSpacing(8);

    m_toggle = new NavButton(QStringLiteral("Collapse"),
                             Icons::themed(QStringLiteral("panel"), ColorRole::RailText), this);
    m_toggle->setObjectName(QStringLiteral("RailToggle"));
    outer->addWidget(m_toggle);
    connect(m_toggle, &QPushButton::clicked, this, [this] { setCollapsed(!m_collapsed); });

    m_signOut = new NavButton(QStringLiteral("Sign out"),
                              Icons::themed(QStringLiteral("logout"), ColorRole::RailText), this);
    m_signOut->setObjectName(QStringLiteral("RailSignOut"));
    outer->addWidget(m_signOut);
    connect(m_signOut, &QPushButton::clicked, this, &NavRail::signOutRequested);

    m_group = new QButtonGroup(this);
    m_group->setExclusive(true);

    m_animation = new QPropertyAnimation(this, "railWidth", this);
    m_animation->setDuration(180);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_animation, &QPropertyAnimation::finished, this, [this] {
        if (!m_collapsed)
            applyCollapsedLook(false);
    });
}

void NavRail::addSection(const QString& caption)
{
    auto* label = new QLabel(caption.toUpper(), this);
    label->setObjectName(QStringLiteral("RailSection"));
    QFont font = label->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
    label->setFont(font);
    label->setVisible(!m_collapsed);
    if (m_items->count() > 0)
        m_items->addSpacing(10);
    m_items->addWidget(label);
    m_sections.append(label);
}

void NavRail::addItem(const QString& id, const QString& text, const QString& iconName)
{
    auto* button = new NavButton(
        text, Icons::themed(iconName, ColorRole::RailText, ColorRole::RailActiveText), this);
    button->setObjectName(QStringLiteral("nav_") + id);
    button->setCheckable(true);
    button->setCollapsed(m_collapsed);
    m_items->addWidget(button);
    m_group->addButton(button);
    m_buttons.insert(id, button);
    m_order.append(id);

    connect(button, &QPushButton::toggled, this, [this, id](bool checked) {
        if (checked)
            emit currentChanged(id);
    });
    if (m_order.size() == 1)
        button->setChecked(true);
}

void NavRail::setCurrent(const QString& id)
{
    if (NavButton* button = m_buttons.value(id))
        button->setChecked(true);
}

QString NavRail::current() const
{
    for (auto it = m_buttons.cbegin(); it != m_buttons.cend(); ++it) {
        if (it.value()->isChecked())
            return it.key();
    }
    return QString();
}

void NavRail::setCollapsed(bool collapsed, bool animate)
{
    if (collapsed == m_collapsed && m_animation->state() != QAbstractAnimation::Running)
        return;
    m_collapsed = collapsed;
    m_animation->stop();

    // Hide labels before shrinking; show them again once fully expanded.
    if (collapsed)
        applyCollapsedLook(true);

    const int target = collapsed ? kCollapsedWidth : kExpandedWidth;
    if (animate && isVisible()) {
        m_animation->setStartValue(width());
        m_animation->setEndValue(target);
        m_animation->start();
    } else {
        setRailWidth(target);
        if (!collapsed)
            applyCollapsedLook(false);
    }
    emit collapsedChanged(collapsed);
}

void NavRail::setRailWidth(int width)
{
    setFixedWidth(width);
}

void NavRail::applyCollapsedLook(bool collapsed)
{
    m_brandText->setVisible(!collapsed);
    const int brandIndent = collapsed ? (kCollapsedWidth - 2 * kSideMargin - kBrandIcon) / 2 : 4;
    m_brandRow->setContentsMargins(brandIndent, 0, 0, 0);

    for (QLabel* section : std::as_const(m_sections))
        section->setVisible(!collapsed);
    for (NavButton* button : std::as_const(m_buttons))
        button->setCollapsed(collapsed);

    m_toggle->setLabel(collapsed ? QStringLiteral("Expand") : QStringLiteral("Collapse"));
    m_toggle->setIcon(Icons::themed(
        collapsed ? QStringLiteral("panel-open") : QStringLiteral("panel"), ColorRole::RailText));
    m_toggle->setCollapsed(collapsed);
    m_signOut->setCollapsed(collapsed);
}

} // namespace edu::ui
