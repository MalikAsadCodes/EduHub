#pragma once

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

namespace edu::ui {

// Split-screen sign-in window: brand panel on the left, form on the right.
//
// M0 preview: there are no accounts yet, so Sign in opens the app without
// checking anything. M1 replaces submit() with AuthService.
class LoginWindow : public QWidget {
    Q_OBJECT

public:
    explicit LoginWindow(const QString& institutionName, QWidget* parent = nullptr);

    QLineEdit* usernameField() const { return m_username; }
    QLineEdit* passwordField() const { return m_password; }
    QPushButton* signInButton() const { return m_signIn; }

signals:
    void signedIn(const QString& username);

private:
    void submit();
    void updateThemeButton();

    QLineEdit* m_username = nullptr;
    QLineEdit* m_password = nullptr;
    QPushButton* m_signIn = nullptr;
    QToolButton* m_themeButton = nullptr;
};

} // namespace edu::ui
