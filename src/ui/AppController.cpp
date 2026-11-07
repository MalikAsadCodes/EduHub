#include "ui/AppController.h"

#include "services/SystemService.h"
#include "ui/login/LoginWindow.h"
#include "ui/main/MainWindow.h"
#include "ui/widgets/Toast.h"

#include <utility>

namespace edu::ui {

AppController::AppController(QString connectionName, QObject* parent)
    : QObject(parent), m_connection(std::move(connectionName))
{
}

LoginWindow* AppController::loginWindow() const
{
    return m_login;
}

MainWindow* AppController::mainWindow() const
{
    return m_main;
}

void AppController::start()
{
    showLogin();
}

void AppController::showLogin()
{
    const SystemService system(m_connection);
    auto* login = new LoginWindow(system.institutionName());
    login->setAttribute(Qt::WA_DeleteOnClose);
    connect(login, &LoginWindow::signedIn, this, &AppController::showMain);
    m_login = login;
    login->show();

    // Show the new window before closing the old one so the app keeps running.
    if (m_main)
        m_main->close();
}

void AppController::showMain(const QString& username)
{
    auto* window = new MainWindow(m_connection);
    window->setAttribute(Qt::WA_DeleteOnClose);
    // M0 preview has no accounts yet; M1 passes the real user from AuthService.
    window->setSignedInUser(username.isEmpty() ? QStringLiteral("Preview User") : username,
                            QStringLiteral("Administrator (preview)"));
    connect(window, &MainWindow::signOutRequested, this, &AppController::showLogin);
    m_main = window;
    window->show();

    if (m_login)
        m_login->close();
    Toast::notify(window, QStringLiteral("Welcome to Edu Hub"), ToastKind::Success);
}

} // namespace edu::ui
