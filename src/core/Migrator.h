#pragma once

#include "core/Result.h"

#include <QList>
#include <QSqlDatabase>
#include <QString>

namespace edu {

struct Migration {
    int version = 0;
    QString name;     // e.g. "init" from 001_init.sql
    QString filePath; // e.g. :/sql/migrations/001_init.sql
};

// Brings a database up to the newest schema.
//
// Migrations are files named NNN_name.sql (001_init.sql, 002_academics.sql...).
// Versions must start at 1 with no gaps or duplicates. Each one runs in its
// own transaction and is recorded in schema_version, so it runs exactly once.
// An applied migration is never edited: schema changes always go in a new file.
class Migrator {
public:
    explicit Migrator(QSqlDatabase db, QString migrationsDir = QStringLiteral(":/sql/migrations"));

    Result migrate();

    // Highest version recorded in schema_version (0 for a new database).
    int currentVersion() const;

    // Highest version available in the migrations folder (0 if none or invalid).
    int latestVersion() const;

    // Reads and validates the migrations folder. Sets *error on a bad folder.
    QList<Migration> available(QString* error = nullptr) const;

private:
    Result ensureVersionTable();
    Result apply(const Migration& migration);

    QSqlDatabase m_db;
    QString m_dir;
};

} // namespace edu
