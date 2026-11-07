#include "core/Database.h"
#include "core/Migrator.h"
#include "ui/AppController.h"
#include "ui/Fonts.h"
#include "ui/UiSettings.h"
#include "ui/login/LoginWindow.h"
#include "ui/main/MainWindow.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/HeaderBar.h"
#include "ui/widgets/NavRail.h"
#include "ui/widgets/Toast.h"

#include <QDir>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QtTest>

using namespace edu::ui;

namespace {

void saveShot(QWidget& widget, const QString& name)
{
    const QString dir = QStringLiteral(EDU_SCREENSHOT_DIR);
    QDir().mkpath(dir);
    QVERIFY(widget.grab().save(QDir(dir).filePath(name + QStringLiteral(".png"))));
}

} // namespace

class TestGui : public QObject {
    Q_OBJECT

private:
    const QString m_name = QStringLiteral("gui_test");

private slots:
    void initTestCase()
    {
        // Keep test runs away from the real app's saved preferences.
        QCoreApplication::setOrganizationName(QStringLiteral("MalikAsadCodes-Tests"));
        QCoreApplication::setApplicationName(QStringLiteral("EduHubGuiTests"));
        QSettings().clear();

        Fonts::install();
        Theme::instance().setMode(ThemeMode::Light);
        Theme::instance().apply();

        QVERIFY(edu::Database::open(edu::Database::memoryPath(), m_name).ok());
        QVERIFY(edu::Migrator(edu::Database::connection(m_name)).migrate().ok());
    }

    void cleanupTestCase()
    {
        QSettings().clear();
        edu::Database::close(m_name);
    }

    void init() { Theme::instance().setMode(ThemeMode::Light); }

    void stylesheetFillsEveryToken()
    {
        for (ThemeMode mode : {ThemeMode::Light, ThemeMode::Dark}) {
            Theme::instance().setMode(mode);
            const QString css = Theme::instance().styleSheet();
            QVERIFY(!css.isEmpty());
            QVERIFY2(!css.contains(QStringLiteral("{{")), "unknown {{token}} in app.qss");
        }
    }

    void loginSignsInAndForgetsPassword()
    {
        LoginWindow window(QStringLiteral("Edu Hub University"));
        window.resize(980, 620);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        saveShot(window, QStringLiteral("login_light"));

        QSignalSpy spy(&window, &LoginWindow::signedIn);
        QTest::keyClicks(window.usernameField(), QStringLiteral("  admin "));
        QTest::keyClicks(window.passwordField(), QStringLiteral("secret"));
        QCOMPARE(window.passwordField()->echoMode(), QLineEdit::Password);
        QTest::mouseClick(window.signInButton(), Qt::LeftButton);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("admin"));
        QVERIFY(window.passwordField()->text().isEmpty());

        Theme::instance().setMode(ThemeMode::Dark);
        QCoreApplication::processEvents();
        saveShot(window, QStringLiteral("login_dark"));
    }

    void navigationShowsEveryPage()
    {
        MainWindow window(m_name);
        window.resize(1240, 780);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        window.setSignedInUser(QStringLiteral("Asad Malik"), QStringLiteral("Administrator"));

        QCOMPARE(window.currentPage(), QStringLiteral("dashboard"));
        QCOMPARE(window.header()->title(), QStringLiteral("Dashboard"));
        saveShot(window, QStringLiteral("dashboard_light"));

        const QStringList ids = window.rail()->itemIds();
        QVERIFY(ids.size() >= 8);
        for (const QString& id : ids) {
            auto* button = window.rail()->findChild<QPushButton*>(QStringLiteral("nav_") + id);
            QVERIFY2(button, qPrintable(id));
            QTest::mouseClick(button, Qt::LeftButton);
            QCOMPARE(window.currentPage(), id);
            QVERIFY(!window.header()->title().isEmpty());
        }
        saveShot(window, QStringLiteral("page_settings_light"));

        window.showPage(QStringLiteral("dashboard"));
        Theme::instance().setMode(ThemeMode::Dark);
        QCoreApplication::processEvents();
        saveShot(window, QStringLiteral("dashboard_dark"));
    }

    void railCollapsesAndRemembers()
    {
        MainWindow window(m_name);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        window.rail()->setCollapsed(true, false);
        QCOMPARE(window.rail()->width(), NavRail::kCollapsedWidth);
        QVERIFY(UiSettings::railCollapsed());
        QCoreApplication::processEvents();
        saveShot(window, QStringLiteral("dashboard_collapsed"));

        window.rail()->setCollapsed(false, false);
        QCOMPARE(window.rail()->width(), NavRail::kExpandedWidth);
        QVERIFY(!UiSettings::railCollapsed());
    }

    void themeSwitchIsSaved()
    {
        MainWindow window(m_name);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        emit window.header()->themeToggleRequested();
        QCOMPARE(Theme::instance().mode(), ThemeMode::Dark);
        QCOMPARE(UiSettings::themeMode(), ThemeMode::Dark);
        QVERIFY(window.findChild<Toast*>());

        emit window.header()->themeToggleRequested();
        QCOMPARE(Theme::instance().mode(), ThemeMode::Light);
        QCOMPARE(UiSettings::themeMode(), ThemeMode::Light);
    }

    void fitsLaptopAt150PercentScaling()
    {
        // 1920 x 1080 at 150% is 1280 x 720 logical pixels, minus the taskbar.
        MainWindow window(m_name);
        window.setSignedInUser(QStringLiteral("Asad Malik"), QStringLiteral("Administrator"));
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        const QSize smallest = window.minimumSizeHint().expandedTo(window.minimumSize());
        QVERIFY2(
            smallest.width() <= 1280 && smallest.height() <= 680,
            qPrintable(
                QStringLiteral("minimum is %1 x %2").arg(smallest.width()).arg(smallest.height())));
        window.resize(960, 600);
        QCoreApplication::processEvents();
        QCOMPARE(window.size(), QSize(960, 600));
        saveShot(window, QStringLiteral("dashboard_960x600"));
    }

    void signOutReturnsToLogin()
    {
        AppController controller(m_name);
        controller.start();
        QVERIFY(controller.loginWindow());
        QVERIFY(!controller.mainWindow());

        emit controller.loginWindow()->signedIn(QStringLiteral("admin"));
        QVERIFY(controller.mainWindow());
        QTRY_VERIFY(!controller.loginWindow()); // closed and deleted

        emit controller.mainWindow()->signOutRequested();
        QVERIFY(controller.loginWindow());
        QTRY_VERIFY(!controller.mainWindow());

        controller.loginWindow()->close();
    }
};

QTEST_MAIN(TestGui)
#include "tst_gui.moc"
