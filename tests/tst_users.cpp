#include "TestSupport.h"

#include "services/AuthService.h"
#include "services/UserService.h"

#include <QtTest>

using namespace edu;
using edu::test::TestDatabase;

class TestUsers : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() { test::useFastHashing(); }

    void onlyAdministratorsManageAccounts()
    {
        TestDatabase db(QStringLiteral("users_permissions"));
        const Session teacher = test::sessionAs(db.name(), QStringLiteral("teacher1"),
                                                Role::Teacher);
        UserService service(db.name(), teacher);
        QVERIFY(!service.list().ok());
        QCOMPARE(service.list().message(), Session::permissionDenied());
        QVERIFY(!service.createAdmin(QStringLiteral("New Admin"), QStringLiteral("newadmin")).ok());
        QVERIFY(!service.resetPassword(1).ok());
        QVERIFY(!service.setActive(1, false).ok());

        Session nobody;
        QVERIFY(!UserService(db.name(), nobody).list().ok());
    }

    void createAdministratorWithTemporaryPassword()
    {
        TestDatabase db(QStringLiteral("users_create"));
        const Session admin = test::sessionAs(db.name(), QStringLiteral("admin"), Role::Admin);
        UserService service(db.name(), admin);

        const ValueResult<NewAccount> created =
            service.createAdmin(QStringLiteral("Sana Tariq"), QStringLiteral("sana.tariq"));
        QVERIFY2(created.ok(), qPrintable(created.message()));
        QVERIFY(PasswordPolicy::check(created.value().temporaryPassword).ok());

        const auto signedIn = AuthService(db.name()).signIn(QStringLiteral("sana.tariq"),
                                                            created.value().temporaryPassword);
        QVERIFY(signedIn.ok());
        QVERIFY(signedIn.value().mustChangePassword);
        QCOMPARE(signedIn.value().user.displayName, QStringLiteral("Sana Tariq"));

        const auto duplicate =
            service.createAdmin(QStringLiteral("Other Sana"), QStringLiteral("SANA.TARIQ"));
        QVERIFY(!duplicate.ok());
        QVERIFY(duplicate.message().contains(QStringLiteral("already taken")));
        QVERIFY(!service.createAdmin(QStringLiteral("X"), QStringLiteral("valid.name")).ok());
        QVERIFY(!service.createAdmin(QStringLiteral("Valid Name"), QStringLiteral("no spaces"))
                     .ok());
        QCOMPARE(service.list().value().size(), 2);
    }

    void resetPasswordForcesChangeAndUnlocks()
    {
        TestDatabase db(QStringLiteral("users_reset"));
        const Session admin = test::sessionAs(db.name(), QStringLiteral("admin"), Role::Admin);
        const int target = test::addUser(db.name(), QStringLiteral("ali"), Role::Teacher,
                                         QStringLiteral("Campus2026"));
        UserRepository(db.name()).recordFailedSignIn(
            target, 0, QDateTime::currentDateTimeUtc().addSecs(300));

        UserService service(db.name(), admin);
        const ValueResult<QString> reset = service.resetPassword(target);
        QVERIFY2(reset.ok(), qPrintable(reset.message()));

        AuthService auth(db.name());
        QVERIFY(!auth.signIn(QStringLiteral("ali"), QStringLiteral("Campus2026")).ok());
        const auto signedIn = auth.signIn(QStringLiteral("ali"), reset.value());
        QVERIFY2(signedIn.ok(), qPrintable(signedIn.message()));
        QVERIFY(signedIn.value().mustChangePassword);

        QVERIFY(!service.resetPassword(admin.userId()).ok()); // own account
        QVERIFY(!service.resetPassword(9999).ok());
    }

    void enableDisableRules()
    {
        TestDatabase db(QStringLiteral("users_active"));
        const Session admin = test::sessionAs(db.name(), QStringLiteral("admin"), Role::Admin);
        const int other = test::addUser(db.name(), QStringLiteral("admin2"), Role::Admin);
        const int teacher = test::addUser(db.name(), QStringLiteral("ali"), Role::Teacher);
        UserService service(db.name(), admin);
        UserRepository users(db.name());

        QVERIFY(!service.setActive(admin.userId(), false).ok()); // not yourself
        QVERIFY(service.setActive(teacher, false).ok());
        QVERIFY(!users.findById(teacher)->active);
        QVERIFY(service.setActive(teacher, true).ok());
        QVERIFY(users.findById(teacher)->active);

        // The last active administrator cannot be disabled.
        QVERIFY(users.setActive(admin.userId(), false).ok());
        const Result last = service.setActive(other, false);
        QVERIFY(!last.ok());
        QVERIFY(last.message().contains(QStringLiteral("at least one active administrator")));
        QVERIFY(users.setActive(admin.userId(), true).ok());
        QVERIFY(service.setActive(other, false).ok());
    }

    void unlockAndRename()
    {
        TestDatabase db(QStringLiteral("users_unlock"));
        const Session admin = test::sessionAs(db.name(), QStringLiteral("admin"), Role::Admin);
        const int other = test::addUser(db.name(), QStringLiteral("admin2"), Role::Admin);
        const int teacher = test::addUser(db.name(), QStringLiteral("ali"), Role::Teacher);
        UserRepository users(db.name());
        users.recordFailedSignIn(other, 0, QDateTime::currentDateTimeUtc().addSecs(300));
        QVERIFY(users.findById(other)->isLocked(QDateTime::currentDateTimeUtc()));

        UserService service(db.name(), admin);
        QVERIFY(service.unlock(other).ok());
        QVERIFY(!users.findById(other)->isLocked(QDateTime::currentDateTimeUtc()));

        QVERIFY(service.rename(other, QStringLiteral("Bilal Ahmed")).ok());
        QCOMPARE(users.findById(other)->displayName, QStringLiteral("Bilal Ahmed"));
        QVERIFY(!service.rename(other, QStringLiteral("B")).ok());
        QVERIFY(!service.rename(teacher, QStringLiteral("Ali Raza")).ok());
    }
};

QTEST_GUILESS_MAIN(TestUsers)
#include "tst_users.moc"
