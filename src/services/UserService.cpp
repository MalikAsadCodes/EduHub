#include "services/UserService.h"

#include "core/PasswordHasher.h"
#include "core/Transaction.h"
#include "services/AuthService.h"

#include <QDebug>

#include <utility>

namespace edu {

namespace {

QString notFound()
{
    return QStringLiteral("That account no longer exists.");
}

} // namespace

UserService::UserService(QString connectionName, const Session& session)
    : m_connection(std::move(connectionName)), m_session(session), m_users(m_connection)
{
}

ValueResult<QList<User>> UserService::list() const
{
    if (Result r = m_session.requireAdmin(); !r)
        return ValueResult<QList<User>>::from(r);
    return ValueResult<QList<User>>::success(m_users.list());
}

ValueResult<NewAccount> UserService::createAdmin(const QString& displayName,
                                                 const QString& username)
{
    if (Result r = m_session.requireAdmin(); !r)
        return ValueResult<NewAccount>::from(r);
    if (Result r = AuthService::checkDisplayName(displayName); !r)
        return ValueResult<NewAccount>::from(r);
    if (Result r = AuthService::checkUsername(username); !r)
        return ValueResult<NewAccount>::from(r);
    if (m_users.usernameExists(username))
        return ValueResult<NewAccount>::failure(
            QStringLiteral("The username \"%1\" is already taken.").arg(username.trimmed()));

    const QString temporary = PasswordPolicy::generateTemporary();
    User user;
    user.username = username.trimmed();
    user.displayName = displayName.trimmed();
    user.role = Role::Admin;
    user.mustChangePassword = true;
    const ValueResult<int> inserted = m_users.insert(user, PasswordHasher::hash(temporary));
    if (!inserted)
        return ValueResult<NewAccount>::from(inserted.toResult());

    qInfo() << "Users: administrator account" << inserted.value() << "created by"
            << m_session.userId();
    return ValueResult<NewAccount>::success({inserted.value(), user.username, temporary});
}

ValueResult<QString> UserService::resetPassword(int userId)
{
    if (Result r = m_session.requireAdmin(); !r)
        return ValueResult<QString>::from(r);
    if (userId == m_session.userId())
        return ValueResult<QString>::failure(
            QStringLiteral("Use Change password for your own account."));
    if (!m_users.findById(userId))
        return ValueResult<QString>::failure(notFound());

    const QString temporary = PasswordPolicy::generateTemporary();
    if (Result r = m_users.setPassword(userId, PasswordHasher::hash(temporary), true); !r)
        return ValueResult<QString>::from(r);
    qInfo() << "Users: password reset for account" << userId << "by" << m_session.userId();
    return ValueResult<QString>::success(temporary);
}

Result UserService::setActive(int userId, bool active)
{
    if (Result r = m_session.requireAdmin(); !r)
        return r;
    if (!active && userId == m_session.userId())
        return Result::failure(QStringLiteral("You cannot disable your own account."));

    Transaction transaction(Database::connection(m_connection));
    if (!transaction.isActive())
        return Result::failure(QStringLiteral("The account could not be saved. Please try again."));
    const std::optional<User> user = m_users.findById(userId);
    if (!user)
        return Result::failure(notFound());
    if (!active && user->role == Role::Admin && user->active && m_users.countActiveAdmins() <= 1)
        return Result::failure(
            QStringLiteral("Edu Hub needs at least one active administrator."));
    if (Result r = m_users.setActive(userId, active); !r)
        return r;
    if (!transaction.commit())
        return Result::failure(QStringLiteral("The account could not be saved. Please try again."));
    qInfo() << "Users: account" << userId << (active ? "enabled" : "disabled") << "by"
            << m_session.userId();
    return Result::success();
}

Result UserService::unlock(int userId)
{
    if (Result r = m_session.requireAdmin(); !r)
        return r;
    if (!m_users.findById(userId))
        return Result::failure(notFound());
    return m_users.unlock(userId);
}

Result UserService::rename(int userId, const QString& displayName)
{
    if (Result r = m_session.requireAdmin(); !r)
        return r;
    if (Result r = AuthService::checkDisplayName(displayName); !r)
        return r;
    const std::optional<User> user = m_users.findById(userId);
    if (!user)
        return Result::failure(notFound());
    if (user->role != Role::Admin)
        return Result::failure(
            QStringLiteral("Teacher and student names are changed on their profile."));
    return m_users.setDisplayName(userId, displayName);
}

} // namespace edu
