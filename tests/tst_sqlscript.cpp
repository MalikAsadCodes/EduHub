#include "core/SqlScript.h"

#include <QtTest>

using edu::SqlScript::split;

class TestSqlScript : public QObject {
    Q_OBJECT

private slots:
    void emptyInputGivesNoStatements()
    {
        QVERIFY(split(QString()).isEmpty());
        QVERIFY(split(QStringLiteral("  \n ; ;; \n")).isEmpty());
    }

    void splitsOnSemicolons()
    {
        const QStringList parts =
            split(QStringLiteral("CREATE TABLE a (x);\nINSERT INTO a VALUES (1);\n"));
        QCOMPARE(parts.size(), 2);
        QCOMPARE(parts.at(0), QStringLiteral("CREATE TABLE a (x)"));
        QCOMPARE(parts.at(1), QStringLiteral("INSERT INTO a VALUES (1)"));
    }

    void keepsLastStatementWithoutSemicolon()
    {
        const QStringList parts = split(QStringLiteral("SELECT 1; SELECT 2"));
        QCOMPARE(parts, QStringList({QStringLiteral("SELECT 1"), QStringLiteral("SELECT 2")}));
    }

    void ignoresSemicolonsInsideQuotes()
    {
        const QStringList parts = split(QStringLiteral(
            "INSERT INTO t VALUES ('a;b', \"c;d\", [e;f], `g;h`); SELECT 'it''s; fine';"));
        QCOMPARE(parts.size(), 2);
        QVERIFY(parts.at(0).contains(QStringLiteral("'a;b'")));
        QVERIFY(parts.at(0).contains(QStringLiteral("[e;f]")));
        QCOMPARE(parts.at(1), QStringLiteral("SELECT 'it''s; fine'"));
    }

    void removesComments()
    {
        const QStringList parts = split(QStringLiteral("-- header; with semicolon\n"
                                                       "CREATE TABLE a (x); /* block; comment */\n"
                                                       "SELECT 1; -- trailing\n"));
        QCOMPARE(parts.size(), 2);
        QCOMPARE(parts.at(0), QStringLiteral("CREATE TABLE a (x)"));
        QCOMPARE(parts.at(1), QStringLiteral("SELECT 1"));
    }

    void commentMarkersInsideStringsAreText()
    {
        const QStringList parts = split(QStringLiteral("SELECT '-- not a comment', '/* nor */';"));
        QCOMPARE(parts.size(), 1);
        QCOMPARE(parts.at(0), QStringLiteral("SELECT '-- not a comment', '/* nor */'"));
    }

    void keepsTriggerBodyTogether()
    {
        const QStringList parts =
            split(QStringLiteral("CREATE TRIGGER t_ai AFTER INSERT ON a BEGIN\n"
                                 "  INSERT INTO log VALUES (1);\n"
                                 "  UPDATE a SET x = 2;\n"
                                 "END;\n"
                                 "SELECT 1;"));
        QCOMPARE(parts.size(), 2);
        QVERIFY(parts.at(0).startsWith(QStringLiteral("CREATE TRIGGER")));
        QVERIFY(parts.at(0).endsWith(QStringLiteral("END")));
        QCOMPARE(parts.at(1), QStringLiteral("SELECT 1"));
    }

    void handlesCaseInsideTrigger()
    {
        const QStringList parts =
            split(QStringLiteral("create temp trigger t before update on a when new.x > 0 begin\n"
                                 "  update a set y = case when new.x > 5 then 1 else 0 end;\n"
                                 "  select raise(abort, 'no; way') where new.x = 9;\n"
                                 "end;\n"
                                 "select 2;"));
        QCOMPARE(parts.size(), 2);
        QVERIFY(parts.at(0).endsWith(QStringLiteral("end")));
        QCOMPARE(parts.at(1), QStringLiteral("select 2"));
    }

    void wordsContainingKeywordsDoNotConfuseTriggers()
    {
        // "beginner" and "ending" must not count as BEGIN / END.
        const QStringList parts =
            split(QStringLiteral("CREATE TABLE beginner (ending TEXT); CREATE TABLE b (x);"));
        QCOMPARE(parts.size(), 2);
    }
};

QTEST_GUILESS_MAIN(TestSqlScript)
#include "tst_sqlscript.moc"
