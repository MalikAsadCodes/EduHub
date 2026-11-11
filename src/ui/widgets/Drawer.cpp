#include "ui/widgets/Drawer.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/UiHelpers.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

namespace edu::ui {

Drawer::Drawer(QWidget* parent) : QFrame(parent)
{
    setObjectName(QStringLiteral("Drawer"));
    setProperty("card", true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    setFixedWidth(0);
    hide();

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    auto* header = new QFrame(this);
    header->setObjectName(QStringLiteral("DrawerHeader"));
    auto* headerColumn = new QVBoxLayout(header);
    headerColumn->setContentsMargins(20, 16, 20, 14);
    headerColumn->setSpacing(10);

    auto* titleRow = new QHBoxLayout;
    titleRow->setSpacing(8);
    auto* titles = new QVBoxLayout;
    titles->setSpacing(2);
    m_title = makeLabel(QString(), QStringLiteral("drawerTitle"), header);
    m_title->setWordWrap(true);
    m_subtitle = makeLabel(QString(), QStringLiteral("muted"), header);
    m_subtitle->setWordWrap(true);
    titles->addWidget(m_title);
    titles->addWidget(m_subtitle);
    titleRow->addLayout(titles, 1);

    m_closeButton = new QToolButton(header);
    m_closeButton->setObjectName(QStringLiteral("DrawerClose"));
    m_closeButton->setProperty("iconButton", true);
    m_closeButton->setIcon(Icons::themed(QStringLiteral("x"), ColorRole::TextMuted));
    m_closeButton->setIconSize(QSize(18, 18));
    m_closeButton->setToolTip(QStringLiteral("Close (Esc)"));
    m_closeButton->setShortcut(QKeySequence(Qt::Key_Escape));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    titleRow->addWidget(m_closeButton, 0, Qt::AlignTop);
    headerColumn->addLayout(titleRow);

    m_actions = new QHBoxLayout;
    m_actions->setSpacing(8);
    m_actions->addStretch(1);
    headerColumn->addLayout(m_actions);
    column->addWidget(header);

    m_message = new QLabel(this);
    m_message->setWordWrap(true);
    m_message->setProperty("role", QStringLiteral("errorBanner"));
    m_message->hide();
    auto* messageRow = new QHBoxLayout;
    messageRow->setContentsMargins(16, 10, 16, 0);
    messageRow->addWidget(m_message);
    column->addLayout(messageRow);

    m_scroll = new QScrollArea(this);
    m_scroll->setObjectName(QStringLiteral("DrawerScroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    column->addWidget(m_scroll, 1);

    connect(m_closeButton, &QToolButton::clicked, this, &Drawer::close);

    m_animation = new QPropertyAnimation(this, "drawerWidth", this);
    m_animation->setDuration(180);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_animation, &QPropertyAnimation::finished, this, [this] {
        if (!m_open)
            hide();
    });
}

void Drawer::setTitle(const QString& title, const QString& subtitle)
{
    m_title->setText(title);
    m_subtitle->setText(subtitle);
    m_subtitle->setVisible(!subtitle.isEmpty());
}

QString Drawer::title() const
{
    return m_title->text();
}

void Drawer::setContent(QWidget* content)
{
    QWidget* old = m_scroll->takeWidget();
    if (old) {
        old->hide();
        old->deleteLater();
    }
    m_content = content;
    if (content) {
        content->setObjectName(QStringLiteral("DrawerBody"));
        m_scroll->setWidget(content);
    }
}

QPushButton* Drawer::addAction(const QString& text, const QString& variant,
                               const QString& iconName)
{
    auto* button = new QPushButton(text, this);
    button->setProperty("variant", variant);
    button->setCursor(Qt::PointingHandCursor);
    if (!iconName.isEmpty()) {
        const ColorRole role = variant == QLatin1String("primary") ? ColorRole::PrimaryText
                               : variant == QLatin1String("danger") ? ColorRole::Danger
                                                                     : ColorRole::Text;
        button->setIcon(Icons::themed(iconName, role));
        button->setIconSize(QSize(16, 16));
    }
    m_actions->addWidget(button);
    m_actionButtons.append(button);
    return button;
}

void Drawer::clearActions()
{
    for (QPushButton* button : std::as_const(m_actionButtons)) {
        m_actions->removeWidget(button);
        button->hide();
        button->deleteLater();
    }
    m_actionButtons.clear();
}

void Drawer::showError(const QString& message)
{
    m_message->setProperty("role", QStringLiteral("errorBanner"));
    repolish(m_message);
    m_message->setText(message);
    m_message->setVisible(!message.isEmpty());
}

void Drawer::showNotice(const QString& message)
{
    m_message->setProperty("role", QStringLiteral("noticeBanner"));
    repolish(m_message);
    m_message->setText(message);
    m_message->setVisible(!message.isEmpty());
}

void Drawer::clearMessage()
{
    m_message->clear();
    m_message->hide();
}

void Drawer::open()
{
    if (m_open && isVisible())
        return;
    m_open = true;
    show();
    animateTo(kWidth);
    emit opened();
}

void Drawer::close()
{
    if (!m_open)
        return;
    m_open = false;
    clearMessage();
    animateTo(0);
    emit closed();
}

void Drawer::setDrawerWidth(int width)
{
    setFixedWidth(width);
}

void Drawer::animateTo(int target)
{
    m_animation->stop();
    if (!window()->isVisible()) {
        setDrawerWidth(target);
        if (!m_open)
            hide();
        return;
    }
    m_animation->setStartValue(width());
    m_animation->setEndValue(target);
    m_animation->start();
}

} // namespace edu::ui
