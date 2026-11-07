#pragma once

#include <QObject>
#include <QPointer>
#include <QString>

namespace edu::ui {

class LoginWindow;
class MainWindow;

// Switches between the sign-in window and the main window.
class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QString connectionName, QObject* parent = nullptr);

    void start();

    LoginWindow* loginWindow() const;
    MainWindow* mainWindow() const;

private:
    void showLogin();
    void showMain(const QString& username);

    QString m_connection;
    QPointer<LoginWindow> m_login;
    QPointer<MainWindow> m_main;
};

} // namespace edu::ui
