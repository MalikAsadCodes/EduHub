#include "ui/login/LoginWindow.h"

#include "ui/UiSettings.h"
#include "ui/login/BrandPanel.h"
#include "ui/theme/Icons.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/UiHelpers.h"

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

namespace edu::ui {

LoginWindow::LoginWindow(const QString& institutionName, QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("LoginRoot"));
    setAttribute(Qt::WA_StyledBackground);
    setWindowTitle(QStringLiteral("Sign in - Edu Hub"));
    setWindowIcon(Icons::fixed(QStringLiteral("app")));
    setMinimumSize(820, 560);
    resize(980, 620);

    auto* split = new QHBoxLayout(this);
    split->setContentsMargins(0, 0, 0, 0);
    split->setSpacing(0);

    split->addWidget(new BrandPanel(institutionName, this), 9);

    // Right: the form
    auto* formSide = new QWidget(this);
    formSide->setObjectName(QStringLiteral("LoginForm"));
    formSide->setAttribute(Qt::WA_StyledBackground);
    split->addWidget(formSide, 11);

    auto* side = new QVBoxLayout(formSide);
    side->setContentsMargins(32, 20, 24, 28);

    auto* topRow = new QHBoxLayout;
    topRow->addStretch(1);
    m_themeButton = new QToolButton(formSide);
    m_themeButton->setObjectName(QStringLiteral("ThemeButton"));
    m_themeButton->setProperty("iconButton", true);
    m_themeButton->setIconSize(QSize(20, 20));
    m_themeButton->setCursor(Qt::PointingHandCursor);
    topRow->addWidget(m_themeButton);
    side->addLayout(topRow);
    connect(m_themeButton, &QToolButton::clicked, this, [] {
        Theme::instance().toggle();
        UiSettings::setThemeMode(Theme::instance().mode());
    });
    connect(&Theme::instance(), &Theme::changed, this, &LoginWindow::updateThemeButton);
    updateThemeButton();

    side->addStretch(1);

    auto* form = new QWidget(formSide);
    form->setMaximumWidth(380);
    form->setMinimumWidth(300);
    auto* column = new QVBoxLayout(form);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    auto* eyebrow = makeLabel(QStringLiteral("WELCOME BACK"), QStringLiteral("eyebrow"), form);
    QFont eyebrowFont = eyebrow->font();
    eyebrowFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
    eyebrow->setFont(eyebrowFont);
    column->addWidget(eyebrow);
    column->addSpacing(6);
    column->addWidget(makeLabel(QStringLiteral("Sign in"), QStringLiteral("loginTitle"), form));
    column->addSpacing(6);
    auto* subtitle = makeLabel(QStringLiteral("Use the account your administrator gave you."),
                               QStringLiteral("muted"), form);
    subtitle->setWordWrap(true);
    column->addWidget(subtitle);
    column->addSpacing(26);

    column->addWidget(makeLabel(QStringLiteral("Username"), QStringLiteral("fieldLabel"), form));
    column->addSpacing(6);
    m_username = new QLineEdit(form);
    m_username->setObjectName(QStringLiteral("UsernameField"));
    m_username->setPlaceholderText(QStringLiteral("e.g. admin"));
    m_username->addAction(Icons::themed(QStringLiteral("user"), ColorRole::TextMuted),
                          QLineEdit::LeadingPosition);
    column->addWidget(m_username);
    column->addSpacing(16);

    column->addWidget(makeLabel(QStringLiteral("Password"), QStringLiteral("fieldLabel"), form));
    column->addSpacing(6);
    m_password = new QLineEdit(form);
    m_password->setObjectName(QStringLiteral("PasswordField"));
    m_password->setEchoMode(QLineEdit::Password);
    m_password->setPlaceholderText(QStringLiteral("Your password"));
    m_password->addAction(Icons::themed(QStringLiteral("lock"), ColorRole::TextMuted),
                          QLineEdit::LeadingPosition);
    QAction* reveal = m_password->addAction(
        Icons::themed(QStringLiteral("eye"), ColorRole::TextMuted), QLineEdit::TrailingPosition);
    reveal->setToolTip(QStringLiteral("Show password"));
    connect(reveal, &QAction::triggered, this, [this, reveal] {
        const bool hidden = m_password->echoMode() == QLineEdit::Password;
        m_password->setEchoMode(hidden ? QLineEdit::Normal : QLineEdit::Password);
        reveal->setIcon(Icons::themed(hidden ? QStringLiteral("eye-off") : QStringLiteral("eye"),
                                      ColorRole::TextMuted));
        reveal->setToolTip(hidden ? QStringLiteral("Hide password")
                                  : QStringLiteral("Show password"));
    });
    column->addWidget(m_password);
    column->addSpacing(24);

    m_signIn = new QPushButton(QStringLiteral("Sign in"), form);
    m_signIn->setObjectName(QStringLiteral("SignInButton"));
    m_signIn->setProperty("variant", QStringLiteral("primary"));
    m_signIn->setCursor(Qt::PointingHandCursor);
    m_signIn->setDefault(true);
    m_signIn->setMinimumHeight(44);
    column->addWidget(m_signIn);
    column->addSpacing(16);

    auto* note = makeLabel(QStringLiteral("Preview build: accounts arrive in the next milestone, "
                                          "so Sign in opens the app without checking."),
                           QStringLiteral("previewNote"), form);
    note->setWordWrap(true);
    column->addWidget(note);
    column->addSpacing(14);
    auto* help = makeLabel(QStringLiteral("Forgot your password? Ask your administrator."),
                           QStringLiteral("hint"), form);
    column->addWidget(help);

    auto* center = new QHBoxLayout;
    center->addStretch(1);
    center->addWidget(form, 4);
    center->addStretch(1);
    side->addLayout(center);
    side->addStretch(1);

    connect(m_signIn, &QPushButton::clicked, this, &LoginWindow::submit);
    connect(m_username, &QLineEdit::returnPressed, this, [this] { m_password->setFocus(); });
    connect(m_password, &QLineEdit::returnPressed, this, &LoginWindow::submit);

    m_username->setFocus();
}

void LoginWindow::submit()
{
    const QString username = m_username->text().trimmed();
    m_password->clear(); // never keep a typed password around
    emit signedIn(username);
}

void LoginWindow::updateThemeButton()
{
    const bool dark = Theme::instance().mode() == ThemeMode::Dark;
    m_themeButton->setIcon(
        Icons::themed(dark ? QStringLiteral("sun") : QStringLiteral("moon"), ColorRole::TextMuted));
    m_themeButton->setToolTip(dark ? QStringLiteral("Switch to light theme")
                                   : QStringLiteral("Switch to dark theme"));
}

} // namespace edu::ui
