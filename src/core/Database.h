#pragma once

#include "core/Result.h"

#include <QSqlDatabase>
#include <QString>

namespace edu {

// Opens and closes named SQLite connections.
// Every connection gets the same settings: foreign keys ON, a busy timeout,
// and WAL journal mode for file databases.
class Database {
public:
    static QString defaultConnectionName();

    // <AppData>/eduhub.db, e.g. C:/Users/<you>/AppData/Roaming/MalikAsadCodes/EduHub/eduhub.db
    static QString defaultFilePath();

    // Path that gives a private in-memory database (used by tests).
    static QString memoryPath();

    static Result open(const QString& filePath,
                       const QString& connectionName = defaultConnectionName());
    static void close(const QString& connectionName = defaultConnectionName());
    static QSqlDatabase connection(const QString& connectionName = defaultConnectionName());
};

} // namespace edu
