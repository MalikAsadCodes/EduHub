#include "core/Migrator.h"

#include "core/SqlScript.h"
#include "core/Transaction.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

#include <algorithm>
#include <utility>

namespace edu {

namespace {

QString genericFailure()
{
    return QStringLiteral("Edu Hub could not update its database. Details are in the log file.");
}

} // namespace

Migrator::Migrator(QSqlDatabase db, QString migrationsDir)
    : m_db(std::move(db)), m_dir(std::move(migrationsDir))
{
}

QList<Migration> Migrator::available(QString* error) const
{
    static const QRegularExpression pattern(QStringLiteral("^(\\d{3})_([a-z0-9_]+)\\.sql$"));

    auto fail = [error](const QString& message) {
        if (error)
            *error = message;
        return QList<Migration>();
    };

    const QDir dir(m_dir);
    if (!dir.exists())
        return fail(QStringLiteral("migrations folder not found: %1").arg(m_dir));

    QList<Migration> migrations;
    const QStringList files = dir.entryList({QStringLiteral("*.sql")}, QDir::Files, QDir::Name);
    for (const QString& file : files) {
        const QRegularExpressionMatch match = pattern.match(file);
        if (!match.hasMatch())
            return fail(
                QStringLiteral("bad migration file name '%1' (expected NNN_name.sql)").arg(file));
        migrations.append({match.captured(1).toInt(), match.captured(2), dir.filePath(file)});
    }

    std::sort(migrations.begin(), migrations.end(),
              [](const Migration& a, const Migration& b) { return a.version < b.version; });

    for (qsizetype i = 0; i < migrations.size(); ++i) {
        if (migrations.at(i).version != static_cast<int>(i) + 1)
            return fail(QStringLiteral("migration versions must run 1, 2, 3... with no gaps or "
                                       "duplicates (problem at %1)")
                            .arg(QFileInfo(migrations.at(i).filePath).fileName()));
    }

    if (error)
        error->clear();
    return migrations;
}

int Migrator::latestVersion() const
{
    const QList<Migration> migrations = available();
    return migrations.isEmpty() ? 0 : migrations.constLast().version;
}

int Migrator::currentVersion() const
{
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral(
            "SELECT 1 FROM sqlite_master WHERE type = 'table' AND name = 'schema_version'")) ||
        !query.next())
        return 0;
    if (!query.exec(QStringLiteral("SELECT COALESCE(MAX(version), 0) FROM schema_version")) ||
        !query.next())
        return 0;
    return query.value(0).toInt();
}

Result Migrator::migrate()
{
    QString error;
    const QList<Migration> migrations = available(&error);
    if (!error.isEmpty()) {
        qWarning().noquote() << "Migrator:" << error;
        return Result::failure(genericFailure());
    }

    if (Result created = ensureVersionTable(); !created)
        return created;

    const int current = currentVersion();
    const int latest = migrations.isEmpty() ? 0 : migrations.constLast().version;
    if (current > latest) {
        qWarning().noquote() << "Migrator: database is at version" << current
                             << "but this build only knows" << latest;
        return Result::failure(
            QStringLiteral("This database was created by a newer version of Edu Hub. "
                           "Please update the app before opening it."));
    }

    for (const Migration& migration : migrations) {
        if (migration.version <= current)
            continue;
        if (Result applied = apply(migration); !applied)
            return applied;
        qInfo().noquote() << "Migrator: applied" << QFileInfo(migration.filePath).fileName();
    }
    return Result::success();
}

Result Migrator::ensureVersionTable()
{
    QSqlQuery query(m_db);
    const bool ok = query.exec(QStringLiteral("CREATE TABLE IF NOT EXISTS schema_version ("
                                              " version    INTEGER PRIMARY KEY,"
                                              " name       TEXT NOT NULL,"
                                              " applied_at TEXT NOT NULL)"));
    if (!ok) {
        qWarning().noquote() << "Migrator: cannot create schema_version:"
                             << query.lastError().text();
        return Result::failure(genericFailure());
    }
    return Result::success();
}

Result Migrator::apply(const Migration& migration)
{
    const QString fileName = QFileInfo(migration.filePath).fileName();
    const QString stepFailure = QStringLiteral("Edu Hub could not update its database (step %1). "
                                               "Details are in the log file.")
                                    .arg(migration.version);

    QFile file(migration.filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning().noquote() << "Migrator: cannot read" << fileName;
        return Result::failure(stepFailure);
    }
    const QStringList statements = SqlScript::split(QString::fromUtf8(file.readAll()));

    Transaction transaction(m_db);
    if (!transaction.isActive())
        return Result::failure(stepFailure);

    QSqlQuery query(m_db);
    for (qsizetype i = 0; i < statements.size(); ++i) {
        if (!query.exec(statements.at(i))) {
            qWarning().noquote() << QStringLiteral("Migrator: %1 failed at statement %2: %3")
                                        .arg(fileName)
                                        .arg(i + 1)
                                        .arg(query.lastError().text());
            return Result::failure(stepFailure); // transaction rolls back
        }
    }

    query.prepare(
        QStringLiteral("INSERT INTO schema_version (version, name, applied_at) VALUES (?, ?, ?)"));
    query.addBindValue(migration.version);
    query.addBindValue(migration.name);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    if (!query.exec()) {
        qWarning().noquote() << "Migrator: cannot record" << fileName << ":"
                             << query.lastError().text();
        return Result::failure(stepFailure);
    }
    query.finish();

    if (!transaction.commit())
        return Result::failure(stepFailure);
    return Result::success();
}

} // namespace edu
