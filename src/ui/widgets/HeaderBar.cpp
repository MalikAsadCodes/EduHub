#include "ui/widgets/HeaderBar.h"

#include "ui/theme/Icons.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/UiHelpers.h"

#include <QAction>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QShortcut>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

namespace edu::ui {

namespace {

QString initialsOf(const QString& name)
{
    QString initials;
    const QStringList parts = name.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString& part : parts) {
        initials += part.at(0).toUpper();
        if (initials.size() == 2)
            break;
    }
    return initials.isEmpty() ? QStringLiteral("?") : initials;
}

} // namespace

HeaderBar::HeaderBar(QWidget* parent) : QFrame(parent)
{
    setObjectName(QStringLiteral("HeaderBar"));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(28, 16, 20, 14);
    row->setSpacing(12);

    // Breadcrumb over title
    auto* titles = new QVBoxLayout;
    titles->setSpacing(2);
    m_breadcrumb = new QLabel(this);
    m_breadcrumb->setObjectName(QStringLiteral("Breadcrumb"));
    styleEyebrow(m_breadcrumb, 1.6);
    m_title = new QLabel(this);
    m_title->setObjectName(QStringLiteral("PageTitle"));
    titles->addWidget(m_breadcrumb);
    titles->addWidget(m_title);
    row->addLayout(titles);
    row->addStretch(1);

    // Current term chip
    auto* chip = new QFrame(this);
    chip->setObjectName(QStringLiteral("TermChip"));
    auto* chipRow = new QHBoxLayout(chip);
    chipRow->setContentsMargins(10, 5, 12, 5);
    chipRow->setSpacing(6);
    chipRow->addWidget(new IconLabel(
        Icons::themed(QStringLiteral("attendance"), ColorRole::AccentText), 14, chip));
    m_term = new QLabel(chip);
    m_term->setObjectName(QStringLiteral("TermChipText"));
    chipRow->addWidget(m_term);
    row->addWidget(chip, 0, Qt::AlignVCenter);

    // Global search
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("GlobalSearch"));
    m_search->setPlaceholderText(QStringLiteral("Search   (Ctrl+K)"));
    m_search->setClearButtonEnabled(true);
    m_search->addAction(Icons::themed(QStringLiteral("search"), ColorRole::TextMuted),
                        QLineEdit::LeadingPosition);
    m_search->setFixedWidth(240);
    row->addWidget(m_search, 0, Qt::AlignVCenter);
    connect(m_search, &QLineEdit::returnPressed, this,
            [this] { emit searchSubmitted(m_search->text().trimmed()); });

    auto* focusSearch = new QShortcut(QKeySequence(QStringLiteral("Ctrl+K")), this);
    focusSearch->setContext(Qt::WindowShortcut);
    connect(focusSearch, &QShortcut::activated, this, [this] {
        m_search->setFocus(Qt::ShortcutFocusReason);
        m_search->selectAll();
    });

    // Theme switch
    m_themeButton = new QToolButton(this);
    m_themeButton->setObjectName(QStringLiteral("ThemeButton"));
    m_themeButton->setProperty("iconButton", true);
    m_themeButton->setIconSize(QSize(20, 20));
    m_themeButton->setCursor(Qt::PointingHandCursor);
    row->addWidget(m_themeButton, 0, Qt::AlignVCenter);
    connect(m_themeButton, &QToolButton::clicked, this, &HeaderBar::themeToggleRequested);
    connect(&Theme::instance(), &Theme::changed, this, &HeaderBar::updateThemeButton);
    updateThemeButton();

    // Avatar with account menu
    m_avatar = new QToolButton(this);
    m_avatar->setObjectName(QStringLiteral("Avatar"));
    m_avatar->setPopupMode(QToolButton::InstantPopup);
    m_avatar->setCursor(Qt::PointingHandCursor);
    m_avatar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    row->addWidget(m_avatar, 0, Qt::AlignVCenter);

    auto* menu = new QMenu(m_avatar);
    auto* identity = new QWidget(menu);
    auto* identityColumn = new QVBoxLayout(identity);
    identityColumn->setContentsMargins(12, 8, 12, 8);
    identityColumn->setSpacing(1);
    m_menuName = new QLabel(identity);
    m_menuName->setObjectName(QStringLiteral("MenuName"));
    m_menuRole = new QLabel(identity);
    m_menuRole->setProperty("role", QStringLiteral("muted"));
    identityColumn->addWidget(m_menuName);
    identityColumn->addWidget(m_menuRole);
    auto* identityAction = new QWidgetAction(menu);
    identityAction->setDefaultWidget(identity);
    menu->addAction(identityAction);
    menu->addSeparator();
    QAction* changePassword =
        menu->addAction(Icons::themed(QStringLiteral("key"), ColorRole::Text),
                        QStringLiteral("Change password..."), this,
                        &HeaderBar::changePasswordRequested);
    changePassword->setObjectName(QStringLiteral("ChangePasswordAction"));
    menu->addSeparator();
    menu->addAction(Icons::themed(QStringLiteral("logout"), ColorRole::Text),
                    QStringLiteral("Sign out"), this, &HeaderBar::signOutRequested);
    m_avatar->setMenu(menu);
}

void HeaderBar::setPage(const QString& title, const QString& section)
{
    m_title->setText(title);
    const QString crumb = section.isEmpty() ? QStringLiteral("Edu Hub")
                                            : QStringLiteral("Edu Hub  /  %1").arg(section);
    m_breadcrumb->setText(crumb.toUpper());
}

QString HeaderBar::title() const
{
    return m_title->text();
}

void HeaderBar::setTermText(const QString& text)
{
    m_term->setText(text);
}

void HeaderBar::setUser(const QString& displayName, const QString& roleLabel,
                        const QString& roleKey)
{
    m_avatar->setProperty("accent", roleKey);
    repolish(m_avatar);
    m_avatar->setText(initialsOf(displayName));
    m_avatar->setToolTip(QStringLiteral("%1 (%2)").arg(displayName, roleLabel));
    m_menuName->setText(displayName);
    m_menuRole->setText(roleLabel);
}

void HeaderBar::updateThemeButton()
{
    const bool dark = Theme::instance().mode() == ThemeMode::Dark;
    m_themeButton->setIcon(
        Icons::themed(dark ? QStringLiteral("sun") : QStringLiteral("moon"), ColorRole::TextMuted));
    m_themeButton->setToolTip(dark ? QStringLiteral("Switch to light theme")
                                   : QStringLiteral("Switch to dark theme"));
}

} // namespace edu::ui
