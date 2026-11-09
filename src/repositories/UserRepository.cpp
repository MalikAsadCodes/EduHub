#include "repositories/UserRepository.h"

#include "core/Sql.h"

#include <QSqlRecord>

#include <utility>

namespace edu {

namespace {

const QString kColumns = QStringLiteral(
    "id, username, display_name, role, is_active, must_change_password, failed_login_count, "
    "locked_until, created_at, last_login_at");

User readUser(const QSqlQuery& query)
{
    User user;
    user.id = query.value(0).toInt();
    user.username = query.value(1).toString();
    user.displayName = query.value(2).toString();
    user.role = roleFromKey(query.value(3).toString()).value_or(Role::Student);
    user.active = query.value(4).toInt() == 1;
    user.mustChangePassword = query.value(5).toInt() == 1;
    user.failedLoginCount = query.value(6).toInt();
    if (!query.value(7).isNull())
        user.lockedUntil = sql::fromUtcText(query.value(7).toString());
    user.createdAt = sql::fromUtcText(query.value(8).toString());
    if (!query.value(9).isNull())
        user.lastLoginAt = sql::fromUtcText(query.value(9).toString());
    return user;
}

Result saveFailed()
{
    return Result::failure(QStringLiteral("The account could not be saved. Please try again."));
}

} // namespace

UserRepository::UserRepository(QString connectionName) : m_connection(std::move(connectionName)) {}

QSqlDatabase UserRepository::db() const
{
    return Database::connection(m_connection);
}

int UserRepository::count() const
{
    QSqlQuery query(db());
    if (!sql::exec(query, QStringLiteral("SELECT COUNT(*) FROM users"), "UserRepository::count")
        || !query.next())
        return 0;
    return query.value(0).toInt();
}

int UserRepository::countActiveAdmins() const
{
    QSqlQuery query(db());
    if (!sql::exec(query,
                   QStringLiteral(
                       "SELECT COUNT(*) FROM users WHERE role = 'admin' AND is_active = 1"),
                   "UserRepository::countActiveAdmins")
        || !query.next())
        return 0;
    return query.value(0).toInt();
}

std::optional<User> UserRepository::findById(int id) const
{
    QSqlQuery query(db());
    if (!sql::prepare(query, QStringLiteral("SELECT %1 FROM users WHERE id = ?").arg(kColumns),
                      "UserRepository::findById"))
        return std::nullopt;
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::findById") || !query.next())
        return std::nullopt;
    return readUser(query);
}

std::optional<User> UserRepository::findByUsername(const QString& username) const
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("SELECT %1 FROM users WHERE username = ?").arg(kColumns),
                      "UserRepository::findByUsername"))
        return std::nullopt;
    query.addBindValue(username.trimmed());
    if (!sql::exec(query, "UserRepository::findByUsername") || !query.next())
        return std::nullopt;
    return readUser(query);
}

std::optional<User> UserRepository::credentials(const QString& username) const
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("SELECT %1, password_hash, salt FROM users WHERE username = ?")
                          .arg(kColumns),
                      "UserRepository::credentials"))
        return std::nullopt;
    query.addBindValue(username.trimmed());
    if (!sql::exec(query, "UserRepository::credentials") || !query.next())
        return std::nullopt;
    User user = readUser(query);
    user.passwordHash = query.value(10).toString();
    user.salt = query.value(11).toString();
    return user;
}

bool UserRepository::usernameExists(const QString& username, int exceptId) const
{
    QSqlQuery query(db());
    if (!sql::prepare(query, QStringLiteral("SELECT 1 FROM users WHERE username = ? AND id <> ?"),
                      "UserRepository::usernameExists"))
        return false;
    query.addBindValue(username.trimmed());
    query.addBindValue(exceptId);
    return sql::exec(query, "UserRepository::usernameExists") && query.next();
}

QList<User> UserRepository::list() const
{
    QList<User> users;
    QSqlQuery query(db());
    if (!sql::exec(query,
                   QStringLiteral("SELECT %1 FROM users ORDER BY role, username COLLATE NOCASE")
                       .arg(kColumns),
                   "UserRepository::list"))
        return users;
    while (query.next())
        users.append(readUser(query));
    return users;
}

ValueResult<int> UserRepository::insert(const User& user, const PasswordHasher::Hashed& password)
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("INSERT INTO users (username, display_name, password_hash, "
                                     "salt, role, is_active, must_change_password, created_at) "
                                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"),
                      "UserRepository::insert"))
        return ValueResult<int>::from(saveFailed());
    query.addBindValue(user.username.trimmed());
    query.addBindValue(user.displayName.trimmed());
    query.addBindValue(password.hash);
    query.addBindValue(password.salt);
    query.addBindValue(roleKey(user.role));
    query.addBindValue(user.active ? 1 : 0);
    query.addBindValue(user.mustChangePassword ? 1 : 0);
    query.addBindValue(sql::nowUtc());
    if (!sql::exec(query, "UserRepository::insert"))
        return ValueResult<int>::from(saveFailed());
    return ValueResult<int>::success(sql::lastId(query));
}

Result UserRepository::setPassword(int id, const PasswordHasher::Hashed& password, bool mustChange)
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("UPDATE users SET password_hash = ?, salt = ?, "
                                     "must_change_password = ?, failed_login_count = 0, "
                                     "locked_until = NULL WHERE id = ?"),
                      "UserRepository::setPassword"))
        return saveFailed();
    query.addBindValue(password.hash);
    query.addBindValue(password.salt);
    query.addBindValue(mustChange ? 1 : 0);
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::setPassword") || query.numRowsAffected() != 1)
        return saveFailed();
    return Result::success();
}

Result UserRepository::setActive(int id, bool active)
{
    QSqlQuery query(db());
    if (!sql::prepare(query, QStringLiteral("UPDATE users SET is_active = ? WHERE id = ?"),
                      "UserRepository::setActive"))
        return saveFailed();
    query.addBindValue(active ? 1 : 0);
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::setActive") || query.numRowsAffected() != 1)
        return saveFailed();
    return Result::success();
}

Result UserRepository::setDisplayName(int id, const QString& name)
{
    QSqlQuery query(db());
    if (!sql::prepare(query, QStringLiteral("UPDATE users SET display_name = ? WHERE id = ?"),
                      "UserRepository::setDisplayName"))
        return saveFailed();
    query.addBindValue(name.trimmed());
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::setDisplayName"))
        return saveFailed();
    return Result::success();
}

Result UserRepository::recordFailedSignIn(int id, int failedCount, const QDateTime& lockedUntil)
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("UPDATE users SET failed_login_count = ?, locked_until = ? "
                                     "WHERE id = ?"),
                      "UserRepository::recordFailedSignIn"))
        return saveFailed();
    query.addBindValue(failedCount);
    query.addBindValue(lockedUntil.isValid() ? QVariant(sql::toUtcText(lockedUntil))
                                             : QVariant(QMetaType(QMetaType::QString)));
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::recordFailedSignIn"))
        return saveFailed();
    return Result::success();
}

Result UserRepository::recordSuccessfulSignIn(int id, const QDateTime& when)
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("UPDATE users SET failed_login_count = 0, locked_until = NULL, "
                                     "last_login_at = ? WHERE id = ?"),
                      "UserRepository::recordSuccessfulSignIn"))
        return saveFailed();
    query.addBindValue(sql::toUtcText(when));
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::recordSuccessfulSignIn"))
        return saveFailed();
    return Result::success();
}

Result UserRepository::unlock(int id)
{
    QSqlQuery query(db());
    if (!sql::prepare(query,
                      QStringLiteral("UPDATE users SET failed_login_count = 0, locked_until = NULL "
                                     "WHERE id = ?"),
                      "UserRepository::unlock"))
        return saveFailed();
    query.addBindValue(id);
    if (!sql::exec(query, "UserRepository::unlock"))
        return saveFailed();
    return Result::success();
}

} // namespace edu
