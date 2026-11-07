#include "core/Database.h"
#include "core/Migrator.h"

#include <QFile>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

using edu::Database;
using edu::Migrator;

namespace {

bool tableExists(const QSqlDatabase& db, const QString& table)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral("SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = ?"));
    query.addBindValue(table);
    return query.exec() && query.next();
}

void writeFile(const QString& path, const QByteArray& content)
{
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(content);
}

} // namespace

class TestMigrator : public QObject {
    Q_OBJECT

private:
    const QString m_name = QStringLiteral("migrator_test");
    QSqlDatabase db() const { return Database::connection(m_name); }

private slots:
    void init() { QVERIFY(Database::open(Database::memoryPath(), m_name).ok()); }
    void cleanup() { Database::close(m_name); }

    void bundledMigrationsAreValid()
    {
        QString error;
        const QList<edu::Migration> migrations = Migrator(db()).available(&error);
        QVERIFY2(error.isEmpty(), qPrintable(error));
        QVERIFY(!migrations.isEmpty());
        QCOMPARE(migrations.first().version, 1);
        QCOMPARE(migrations.first().name, QStringLiteral("init"));
    }

    void appliesBundledMigrations()
    {
        Migrator migrator(db());
        QCOMPARE(migrator.currentVersion(), 0);
        const edu::Result result = migrator.migrate();
        QVERIFY2(result.ok(), qPrintable(result.message()));
        QCOMPARE(migrator.currentVersion(), migrator.latestVersion());
        QVERIFY(tableExists(db(), QStringLiteral("schema_version")));
        QVERIFY(tableExists(db(), QStringLiteral("settings")));
        QVERIFY(tableExists(db(), QStringLiteral("users")));
    }

    void secondRunChangesNothing()
    {
        Migrator migrator(db());
        QVERIFY(migrator.migrate().ok());
        QVERIFY(migrator.migrate().ok());

        QSqlQuery query(db());
        QVERIFY(query.exec(QStringLiteral("SELECT COUNT(*) FROM schema_version")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), migrator.latestVersion());
    }

    void usersTableEnforcesRules()
    {
        QVERIFY(Migrator(db()).migrate().ok());
        QSqlQuery query(db());
        const QString insert = QStringLiteral("INSERT INTO users (username, password_hash, salt, "
                                              "role) VALUES ('%1', 'h', 's', '%2')");

        QVERIFY(query.exec(insert.arg(QStringLiteral("admin"), QStringLiteral("admin"))));
        // Unknown role
        QVERIFY(!query.exec(insert.arg(QStringLiteral("root"), QStringLiteral("superuser"))));
        // Usernames are unique without regard to case
        QVERIFY(!query.exec(insert.arg(QStringLiteral("ADMIN"), QStringLiteral("teacher"))));
        // Too short
        QVERIFY(!query.exec(insert.arg(QStringLiteral("ab"), QStringLiteral("student"))));

        QVERIFY(query.exec(
            QStringLiteral("SELECT is_active, must_change_password, failed_login_count, created_at "
                           "FROM users WHERE username = 'admin'")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
        QCOMPARE(query.value(1).toInt(), 0);
        QCOMPARE(query.value(2).toInt(), 0);
        QVERIFY(query.value(3).toString().endsWith(QLatin1Char('Z')));
    }

    void appliesFolderMigrationsInOrder()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.filePath(QStringLiteral("002_more.sql")), "INSERT INTO a (x) VALUES (2);\n");
        writeFile(dir.filePath(QStringLiteral("001_first.sql")),
                  "CREATE TABLE a (x INTEGER);\nINSERT INTO a (x) VALUES (1);\n");

        Migrator migrator(db(), dir.path());
        QVERIFY(migrator.migrate().ok());
        QCOMPARE(migrator.currentVersion(), 2);

        QSqlQuery query(db());
        QVERIFY(query.exec(QStringLiteral("SELECT group_concat(x) FROM a")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("1,2"));
    }

    void failedMigrationRollsBackCompletely()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        writeFile(dir.filePath(QStringLiteral("001_ok.sql")), "CREATE TABLE a (x INTEGER);\n");
        writeFile(dir.filePath(QStringLiteral("002_broken.sql")),
                  "CREATE TABLE b (y INTEGER);\nTHIS IS NOT SQL;\n");

        Migrator migrator(db(), dir.path());
        const edu::Result result = migrator.migrate();
        QVERIFY(!result.ok());
        QVERIFY(result.message().contains(QStringLiteral("step 2")));
        QVERIFY(!result.message().contains(QStringLiteral("syntax"))); // no SQL details for users
        QCOMPARE(migrator.currentVersion(), 1);
        QVERIFY(tableExists(db(), QStringLiteral("a")));
        QVERIFY(!tableExists(db(), QStringLiteral("b"))); // rolled back
    }

    void refusesDatabaseFromNewerVersion()
    {
        Migrator migrator(db());
        QVERIFY(migrator.migrate().ok());
        QSqlQuery query(db());
        QVERIFY(query.exec(QStringLiteral("INSERT INTO schema_version (version, name, applied_at) "
                                          "VALUES (999, 'future', '2030-01-01T00:00:00Z')")));

        const edu::Result result = migrator.migrate();
        QVERIFY(!result.ok());
        QVERIFY(result.message().contains(QStringLiteral("newer version")));
    }

    void rejectsGapsDuplicatesAndBadNames_data()
    {
        QTest::addColumn<QStringList>("files");
        QTest::newRow("gap") << QStringList{QStringLiteral("001_a.sql"),
                                            QStringLiteral("003_c.sql")};
        QTest::newRow("duplicate")
            << QStringList{QStringLiteral("001_a.sql"), QStringLiteral("001_b.sql")};
        QTest::newRow("starts at 2") << QStringList{QStringLiteral("002_a.sql")};
        QTest::newRow("bad name") << QStringList{QStringLiteral("1_init.sql")};
        QTest::newRow("capitals") << QStringList{QStringLiteral("001_Init.sql")};
    }

    void rejectsGapsDuplicatesAndBadNames()
    {
        QFETCH(QStringList, files);
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        for (const QString& file : files)
            writeFile(dir.filePath(file), "SELECT 1;\n");

        Migrator migrator(db(), dir.path());
        QString error;
        QVERIFY(migrator.available(&error).isEmpty());
        QVERIFY(!error.isEmpty());
        QVERIFY(!migrator.migrate().ok());
        QCOMPARE(migrator.currentVersion(), 0);
    }
};

QTEST_GUILESS_MAIN(TestMigrator)
#include "tst_migrator.moc"
