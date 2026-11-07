#include "core/Database.h"
#include "core/Transaction.h"

#include <QSqlQuery>
#include <QTemporaryDir>
#include <QtTest>

using edu::Database;
using edu::Transaction;

class TestDatabase : public QObject {
    Q_OBJECT

private slots:
    void opensInMemoryWithForeignKeys()
    {
        const QString name = QStringLiteral("db_memory");
        QVERIFY(Database::open(Database::memoryPath(), name).ok());
        QSqlQuery query(Database::connection(name));
        QVERIFY(query.exec(QStringLiteral("PRAGMA foreign_keys")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 1);
        query = QSqlQuery();
        Database::close(name);
        QVERIFY(!QSqlDatabase::contains(name));
    }

    void fileDatabaseUsesWalAndCreatesFolder()
    {
        QTemporaryDir temp;
        QVERIFY(temp.isValid());
        const QString path = temp.filePath(QStringLiteral("nested/folder/eduhub.db"));
        const QString name = QStringLiteral("db_file");
        QVERIFY(Database::open(path, name).ok());
        QVERIFY(QFileInfo::exists(path));

        QSqlQuery query(Database::connection(name));
        QVERIFY(query.exec(QStringLiteral("PRAGMA journal_mode")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString().toLower(), QStringLiteral("wal"));
        query = QSqlQuery();
        Database::close(name);
    }

    void foreignKeysAreEnforced()
    {
        const QString name = QStringLiteral("db_fk");
        QVERIFY(Database::open(Database::memoryPath(), name).ok());
        QSqlQuery query(Database::connection(name));
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE parent (id INTEGER PRIMARY KEY)")));
        QVERIFY(query.exec(
            QStringLiteral("CREATE TABLE child (id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL "
                           "REFERENCES parent(id))")));
        QVERIFY(!query.exec(QStringLiteral("INSERT INTO child (parent_id) VALUES (42)")));
        query = QSqlQuery();
        Database::close(name);
    }

    void transactionRollsBackUnlessCommitted()
    {
        const QString name = QStringLiteral("db_tx");
        QVERIFY(Database::open(Database::memoryPath(), name).ok());
        QSqlDatabase db = Database::connection(name);
        QSqlQuery query(db);
        QVERIFY(query.exec(QStringLiteral("CREATE TABLE t (x INTEGER)")));

        {
            Transaction tx(db);
            QVERIFY(tx.isActive());
            QVERIFY(query.exec(QStringLiteral("INSERT INTO t VALUES (1)")));
        } // no commit: rolled back

        {
            Transaction tx(db);
            QVERIFY(query.exec(QStringLiteral("INSERT INTO t VALUES (2)")));
            QVERIFY(tx.commit());
            QVERIFY(!tx.isActive());
        }

        QVERIFY(query.exec(QStringLiteral("SELECT group_concat(x) FROM t")));
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QStringLiteral("2"));
        query = QSqlQuery();
        db = QSqlDatabase();
        Database::close(name);
    }
};

QTEST_GUILESS_MAIN(TestDatabase)
#include "tst_database.moc"
