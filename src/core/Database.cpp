#include "core/Database.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

namespace edu {

QString Database::defaultConnectionName()
{
    return QStringLiteral("eduhub");
}

QString Database::defaultFilePath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(dir).filePath(QStringLiteral("eduhub.db"));
}

QString Database::memoryPath()
{
    return QStringLiteral(":memory:");
}

Result Database::open(const QString& filePath, const QString& connectionName)
{
    if (QSqlDatabase::contains(connectionName))
        close(connectionName);

    const bool inMemory = (filePath == memoryPath());
    if (!inMemory) {
        const QString folder = QFileInfo(filePath).absolutePath();
        if (!QDir().mkpath(folder)) {
            qWarning().noquote() << "Database: cannot create folder" << folder;
            return Result::failure(QStringLiteral(
                "Edu Hub could not create its data folder. Check that you can write to it."));
        }
    }

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connectionName);
    if (!db.isValid()) {
        qWarning() << "Database: QSQLITE driver is not available";
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return Result::failure(
            QStringLiteral("The SQLite driver is missing. Please reinstall Edu Hub."));
    }

    db.setDatabaseName(filePath);
    if (!db.open()) {
        qWarning().noquote() << "Database: open failed:" << db.lastError().text();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connectionName);
        return Result::failure(
            QStringLiteral("Edu Hub could not open its database. It may be in use or damaged."));
    }

    QSqlQuery query(db);
    const QStringList pragmas = {
        QStringLiteral("PRAGMA foreign_keys = ON"),
        QStringLiteral("PRAGMA busy_timeout = 5000"),
    };
    for (const QString& pragma : pragmas) {
        if (!query.exec(pragma)) {
            qWarning().noquote() << "Database:" << pragma << "failed:" << query.lastError().text();
            query = QSqlQuery();
            db = QSqlDatabase();
            close(connectionName);
            return Result::failure(QStringLiteral("Edu Hub could not prepare its database."));
        }
    }

    if (!inMemory) {
        // WAL keeps reads fast while a write is in progress. Not fatal if refused.
        if (query.exec(QStringLiteral("PRAGMA journal_mode = WAL")) && query.next()) {
            const QString mode = query.value(0).toString();
            if (mode.compare(QStringLiteral("wal"), Qt::CaseInsensitive) != 0)
                qWarning().noquote() << "Database: journal mode is" << mode << "instead of wal";
        }
        query.exec(QStringLiteral("PRAGMA synchronous = NORMAL"));
    }

    // Foreign keys are the backbone of every rule. Refuse to run without them.
    if (!query.exec(QStringLiteral("PRAGMA foreign_keys")) || !query.next() ||
        query.value(0).toInt() != 1) {
        qWarning() << "Database: foreign keys could not be enabled";
        query = QSqlQuery();
        db = QSqlDatabase();
        close(connectionName);
        return Result::failure(QStringLiteral("Edu Hub could not prepare its database."));
    }

    return Result::success();
}

void Database::close(const QString& connectionName)
{
    {
        QSqlDatabase db = QSqlDatabase::database(connectionName, false);
        if (db.isOpen())
            db.close();
    }
    QSqlDatabase::removeDatabase(connectionName);
}

QSqlDatabase Database::connection(const QString& connectionName)
{
    return QSqlDatabase::database(connectionName, false);
}

} // namespace edu
