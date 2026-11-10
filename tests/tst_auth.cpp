#include "TestSupport.h"

#include "repositories/SettingsRepository.h"
#include "services/AuthService.h"
#include "services/SystemService.h"

#include <QtTest>

using namespace edu;
using edu::test::TestDatabase;

class TestAuth : public QObject {
    Q_OBJECT

private:
    QDateTime m_now;
    AuthService auth(const QString& connection)
    {
        return AuthService(connection, [this] { return m_now; });
    }

private slots:
    void initTestCase() { test::useFastHashing(); }
    void init() { m_now = QDateTime(QDate(2026, 10, 24), QTime(9, 0), QTimeZone::UTC); }

    void firstRunCreatesAdministrator()
    {
        TestDatabase db(QStringLiteral("auth_setup"));
        QVERIFY(db.ok());
        AuthService service = auth(db.name());
        QVERIFY(service.needsSetup());

        const ValueResult<CurrentUser> created = service.createFirstAdmin(
            QStringLiteral("Indus Valley University"), QStringLiteral("Asad Malik"),
            QStringLiteral("admin"), QStringLiteral("Campus2026"));
        QVERIFY2(created.ok(), qPrintable(created.message()));
        QCOMPARE(created.value().role, Role::Admin);
        QCOMPARE(created.value().displayName, QStringLiteral("Asad Malik"));
        QVERIFY(!service.needsSetup());
        QCOMPARE(SystemService(db.name()).institutionName(),
                 QStringLiteral("Indus Valley University"));

        // Only once.
        const ValueResult<CurrentUser> again = service.createFirstAdmin(
            QString(), QStringLiteral("Someone Else"), QStringLiteral("other"),
            QStringLiteral("Campus2026"));
        QVERIFY(!again.ok());
        QVERIFY(again.message().contains(QStringLiteral("already set up")));

        const ValueResult<SignInOutcome> signedIn =
            service.signIn(QStringLiteral("ADMIN"), QStringLiteral("Campus2026"));
        QVERIFY2(signedIn.ok(), qPrintable(signedIn.message()));
        QVERIFY(!signedIn.value().mustChangePassword);
    }

    void setupValidatesInput_data()
    {
        QTest::addColumn<QString>("name");
        QTest::addColumn<QString>("username");
        QTest::addColumn<QString>("password");
        QTest::newRow("short name") << QStringLiteral("A") << QStringLiteral("admin")
                                    << QStringLiteral("Campus2026");
        QTest::newRow("bad username") << QStringLiteral("Asad") << QStringLiteral("a b")
                                      << QStringLiteral("Campus2026");
        QTest::newRow("short username") << QStringLiteral("Asad") << QStringLiteral("ab")
                                        << QStringLiteral("Campus2026");
        QTest::newRow("weak password") << QStringLiteral("Asad") << QStringLiteral("admin")
                                       << QStringLiteral("password");
        QTest::newRow("password has username") << QStringLiteral("Asad") << QStringLiteral("asad")
                                               << QStringLiteral("asad2026x");
    }

    void setupValidatesInput()
    {
        QFETCH(QString, name);
        QFETCH(QString, username);
        QFETCH(QString, password);
        TestDatabase db(QStringLiteral("auth_setup_invalid"));
        AuthService service = auth(db.name());
        const ValueResult<CurrentUser> created =
            service.createFirstAdmin(QString(), name, username, password);
        QVERIFY(!created.ok());
        QVERIFY(!created.message().isEmpty());
        QVERIFY(service.needsSetup());
    }

    void wrongPasswordAndUnknownUserLookTheSame()
    {
        TestDatabase db(QStringLiteral("auth_wrong"));
        test::addUser(db.name(), QStringLiteral("sana"), Role::Admin, QStringLiteral("Campus2026"));
        AuthService service = auth(db.name());

        const auto wrong = service.signIn(QStringLiteral("sana"), QStringLiteral("nope12345"));
        const auto unknown = service.signIn(QStringLiteral("ghost"), QStringLiteral("nope12345"));
        QVERIFY(!wrong.ok());
        QVERIFY(!unknown.ok());
        QCOMPARE(wrong.message(), unknown.message());
        QCOMPARE(wrong.message(), QStringLiteral("Wrong username or password."));

        const auto empty = service.signIn(QString(), QString());
        QVERIFY(!empty.ok());
    }

    void lockAfterFiveFailuresThenUnlockAfterFiveMinutes()
    {
        TestDatabase db(QStringLiteral("auth_lock"));
        const int id = test::addUser(db.name(), QStringLiteral("sana"), Role::Admin,
                                     QStringLiteral("Campus2026"));
        AuthService service = auth(db.name());

        for (int i = 1; i < AuthService::kMaxFailedAttempts; ++i) {
            const auto r = service.signIn(QStringLiteral("sana"), QStringLiteral("bad-guess1"));
            QCOMPARE(r.message(), QStringLiteral("Wrong username or password."));
        }
        const auto fifth = service.signIn(QStringLiteral("sana"), QStringLiteral("bad-guess1"));
        QVERIFY(fifth.message().contains(QStringLiteral("locked")));

        // Even the right password is refused while locked.
        m_now = m_now.addSecs(60);
        const auto locked = service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026"));
        QVERIFY(!locked.ok());
        QVERIFY2(locked.message().contains(QStringLiteral("4 minutes")),
                 qPrintable(locked.message()));
        QVERIFY(UserRepository(db.name()).findById(id)->isLocked(m_now));

        m_now = m_now.addSecs(5 * 60);
        const auto open = service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026"));
        QVERIFY2(open.ok(), qPrintable(open.message()));
        const User after = *UserRepository(db.name()).findById(id);
        QCOMPARE(after.failedLoginCount, 0);
        QVERIFY(!after.lockedUntil.isValid());
        QCOMPARE(after.lastLoginAt, m_now);
    }

    void successResetsFailureCount()
    {
        TestDatabase db(QStringLiteral("auth_reset_count"));
        const int id = test::addUser(db.name(), QStringLiteral("sana"), Role::Admin,
                                     QStringLiteral("Campus2026"));
        AuthService service = auth(db.name());
        for (int i = 0; i < 3; ++i)
            service.signIn(QStringLiteral("sana"), QStringLiteral("bad-guess1"));
        QCOMPARE(UserRepository(db.name()).findById(id)->failedLoginCount, 3);
        QVERIFY(service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026")).ok());
        QCOMPARE(UserRepository(db.name()).findById(id)->failedLoginCount, 0);
        // Four more failures do not lock, because the count started again.
        for (int i = 0; i < 4; ++i)
            service.signIn(QStringLiteral("sana"), QStringLiteral("bad-guess1"));
        QVERIFY(service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026")).ok());
    }

    void disabledAccountCannotSignIn()
    {
        TestDatabase db(QStringLiteral("auth_disabled"));
        test::addUser(db.name(), QStringLiteral("sana"), Role::Teacher,
                      QStringLiteral("Campus2026"), false);
        AuthService service = auth(db.name());
        const auto wrong = service.signIn(QStringLiteral("sana"), QStringLiteral("bad-guess1"));
        QCOMPARE(wrong.message(), QStringLiteral("Wrong username or password."));
        const auto right = service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026"));
        QVERIFY(!right.ok());
        QVERIFY(right.message().contains(QStringLiteral("disabled")));
    }

    void mustChangeFlagIsReported()
    {
        TestDatabase db(QStringLiteral("auth_must_change"));
        test::addUser(db.name(), QStringLiteral("sana"), Role::Student,
                      QStringLiteral("kpmt-4821"), true, true);
        const auto r = auth(db.name()).signIn(QStringLiteral("sana"), QStringLiteral("kpmt-4821"));
        QVERIFY(r.ok());
        QVERIFY(r.value().mustChangePassword);
        QCOMPARE(r.value().user.role, Role::Student);
    }

    void changePassword()
    {
        TestDatabase db(QStringLiteral("auth_change"));
        const int id = test::addUser(db.name(), QStringLiteral("sana"), Role::Admin,
                                     QStringLiteral("kpmt-4821"), true, true);
        AuthService service = auth(db.name());
        Session session;
        session.signIn(test::identity(id, QStringLiteral("sana"), Role::Admin));

        QVERIFY(!service.changePassword(session, QStringLiteral("wrong1234"),
                                        QStringLiteral("Campus2026"))
                     .ok());
        QVERIFY(!service.changePassword(session, QStringLiteral("kpmt-4821"),
                                        QStringLiteral("kpmt-4821"))
                     .ok());
        QVERIFY(!service.changePassword(session, QStringLiteral("kpmt-4821"),
                                        QStringLiteral("short1"))
                     .ok());
        const Result changed = service.changePassword(session, QStringLiteral("kpmt-4821"),
                                                      QStringLiteral("Campus2026"));
        QVERIFY2(changed.ok(), qPrintable(changed.message()));

        QVERIFY(!UserRepository(db.name()).findById(id)->mustChangePassword);
        QVERIFY(!service.signIn(QStringLiteral("sana"), QStringLiteral("kpmt-4821")).ok());
        const auto signedIn = service.signIn(QStringLiteral("sana"), QStringLiteral("Campus2026"));
        QVERIFY(signedIn.ok());
        QVERIFY(!signedIn.value().mustChangePassword);

        Session nobody;
        QVERIFY(!service.changePassword(nobody, QStringLiteral("Campus2026"),
                                        QStringLiteral("Another2026"))
                     .ok());
    }

    void storedPasswordIsNeverPlainText()
    {
        TestDatabase db(QStringLiteral("auth_storage"));
        AuthService service = auth(db.name());
        QVERIFY(service
                    .createFirstAdmin(QString(), QStringLiteral("Asad Malik"),
                                      QStringLiteral("admin"), QStringLiteral("Campus2026"))
                    .ok());
        QSqlQuery query(Database::connection(db.name()));
        QVERIFY(query.exec(QStringLiteral("SELECT password_hash, salt FROM users")));
        QVERIFY(query.next());
        QVERIFY(!query.value(0).toString().contains(QStringLiteral("Campus2026")));
        QVERIFY(query.value(0).toString().startsWith(QStringLiteral("pbkdf2_sha256$")));
        QVERIFY(!query.value(1).toString().isEmpty());
    }
};

QTEST_GUILESS_MAIN(TestAuth)
#include "tst_auth.moc"
