#include "core/PasswordHasher.h"

#include <QElapsedTimer>
#include <QSet>
#include <QtTest>

using namespace edu;

class TestPassword : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() { PasswordHasher::setDefaultIterations(1000); }

    void pbkdf2MatchesRfcVectors_data()
    {
        QTest::addColumn<QByteArray>("password");
        QTest::addColumn<QByteArray>("salt");
        QTest::addColumn<int>("iterations");
        QTest::addColumn<int>("length");
        QTest::addColumn<QByteArray>("expectedHex");
        // RFC 7914 section 11
        QTest::newRow("passwd/salt/1")
            << QByteArray("passwd") << QByteArray("salt") << 1 << 64
            << QByteArray("55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc"
                          "49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783");
        QTest::newRow("Password/NaCl/80000")
            << QByteArray("Password") << QByteArray("NaCl") << 80000 << 64
            << QByteArray("4ddcd8f60b98be21830cee5ef22701f9641a4418d04c0414aeff08876b34ab56"
                          "a1d425a1225833549adb841b51c9b3176a272bdebba1d078478f62b397f33c8d");
        QTest::newRow("password/salt/4096")
            << QByteArray("password") << QByteArray("salt") << 4096 << 32
            << QByteArray("c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a");
    }

    void pbkdf2MatchesRfcVectors()
    {
        QFETCH(QByteArray, password);
        QFETCH(QByteArray, salt);
        QFETCH(int, iterations);
        QFETCH(int, length);
        QFETCH(QByteArray, expectedHex);
        QCOMPARE(PasswordHasher::pbkdf2Sha256(password, salt, iterations, length).toHex(),
                 expectedHex);
    }

    void hashThenVerify()
    {
        const PasswordHasher::Hashed hashed = PasswordHasher::hash(QStringLiteral("Secret123"));
        QVERIFY(hashed.hash.startsWith(QStringLiteral("pbkdf2_sha256$1000$")));
        QVERIFY(!hashed.hash.contains(QStringLiteral("Secret123")));
        QVERIFY(PasswordHasher::verify(QStringLiteral("Secret123"), hashed.hash, hashed.salt));
        QVERIFY(!PasswordHasher::verify(QStringLiteral("secret123"), hashed.hash, hashed.salt));
        QVERIFY(!PasswordHasher::verify(QString(), hashed.hash, hashed.salt));
    }

    void saltsAreUnique()
    {
        QSet<QString> salts;
        QSet<QString> hashes;
        for (int i = 0; i < 20; ++i) {
            const PasswordHasher::Hashed hashed = PasswordHasher::hash(QStringLiteral("same1234"));
            salts.insert(hashed.salt);
            hashes.insert(hashed.hash);
        }
        QCOMPARE(salts.size(), 20);
        QCOMPARE(hashes.size(), 20);
    }

    void verifyRejectsDamagedHashes()
    {
        const PasswordHasher::Hashed good = PasswordHasher::hash(QStringLiteral("Secret123"));
        const QString pw = QStringLiteral("Secret123");
        QVERIFY(!PasswordHasher::verify(pw, QString(), good.salt));
        QVERIFY(!PasswordHasher::verify(pw, QStringLiteral("md5$1$abc"), good.salt));
        QVERIFY(!PasswordHasher::verify(pw, QStringLiteral("pbkdf2_sha256$x$abc"), good.salt));
        QVERIFY(!PasswordHasher::verify(pw, QStringLiteral("pbkdf2_sha256$0$abc"), good.salt));
        QVERIFY(!PasswordHasher::verify(pw, good.hash, QStringLiteral("AAAA")));
    }

    void productionWorkFactorIsReasonable()
    {
        // Slow enough to resist guessing, fast enough for a sign-in.
        QElapsedTimer timer;
        timer.start();
        PasswordHasher::pbkdf2Sha256("Secret123", "0123456789abcdef",
                                     PasswordHasher::kProductionIterations, 32);
        const qint64 elapsed = timer.elapsed();
        qInfo() << "one production hash took" << elapsed << "ms";
        QVERIFY(PasswordHasher::kProductionIterations >= 100000);
    }

    void policy_data()
    {
        QTest::addColumn<QString>("password");
        QTest::addColumn<QString>("username");
        QTest::addColumn<bool>("accepted");
        QTest::newRow("good") << QStringLiteral("Campus2026") << QStringLiteral("admin") << true;
        QTest::newRow("short") << QStringLiteral("abc12") << QStringLiteral("admin") << false;
        QTest::newRow("no digit") << QStringLiteral("onlyletters") << QStringLiteral("admin") << false;
        QTest::newRow("no letter") << QStringLiteral("1234567890") << QStringLiteral("admin") << false;
        QTest::newRow("contains username") << QStringLiteral("sana.tariq99") << QStringLiteral("sana.tariq") << false;
        QTest::newRow("username ignoring case") << QStringLiteral("ADMIN12345") << QStringLiteral("admin") << false;
        QTest::newRow("too long") << QString(QString(129, QLatin1Char('a')) + QStringLiteral("1")) << QStringLiteral("admin") << false;
        QTest::newRow("unicode letters")
            << QString(QString::fromUtf16(u"\u0645\u0631\u062D\u0628\u0627") + QStringLiteral("2026xy"))
            << QStringLiteral("admin") << true;
    }

    void policy()
    {
        QFETCH(QString, password);
        QFETCH(QString, username);
        QFETCH(bool, accepted);
        const Result result = PasswordPolicy::check(password, username);
        QCOMPARE(result.ok(), accepted);
        if (!accepted)
            QVERIFY(!result.message().isEmpty());
    }

    void temporaryPasswordsPassThePolicy()
    {
        QSet<QString> seen;
        for (int i = 0; i < 200; ++i) {
            const QString temporary = PasswordPolicy::generateTemporary();
            QCOMPARE(temporary.size(), 9);
            QVERIFY2(PasswordPolicy::check(temporary, QStringLiteral("someone")).ok(),
                     qPrintable(temporary));
            QVERIFY(!temporary.contains(QLatin1Char('l')));
            QVERIFY(!temporary.contains(QLatin1Char('o')));
            QVERIFY(!temporary.contains(QLatin1Char('0')));
            QVERIFY(!temporary.contains(QLatin1Char('1')));
            seen.insert(temporary);
        }
        QVERIFY(seen.size() > 190);
    }
};

QTEST_GUILESS_MAIN(TestPassword)
#include "tst_password.moc"
