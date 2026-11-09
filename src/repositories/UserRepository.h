#pragma once

#include "core/Database.h"
#include "core/PasswordHasher.h"
#include "core/ValueResult.h"
#include "models/User.h"

#include <QList>

#include <optional>

namespace edu {

// SQL for the users table. Never returns password hashes except from
// credentials(), which only AuthService uses.
class UserRepository {
public:
    explicit UserRepository(QString connectionName = Database::defaultConnectionName());

    int count() const;
    int countActiveAdmins() const;

    std::optional<User> findById(int id) const;
    std::optional<User> findByUsername(const QString& username) const;
    std::optional<User> credentials(const QString& username) const;
    bool usernameExists(const QString& username, int exceptId = 0) const;

    QList<User> list() const;

    ValueResult<int> insert(const User& user, const PasswordHasher::Hashed& password);
    Result setPassword(int id, const PasswordHasher::Hashed& password, bool mustChange);
    Result setActive(int id, bool active);
    Result setDisplayName(int id, const QString& name);
    Result recordFailedSignIn(int id, int failedCount, const QDateTime& lockedUntil);
    Result recordSuccessfulSignIn(int id, const QDateTime& when);
    Result unlock(int id);

private:
    QSqlDatabase db() const;

    QString m_connection;
};

} // namespace edu
