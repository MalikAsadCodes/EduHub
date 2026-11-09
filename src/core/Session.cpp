#include "core/Session.h"

namespace edu {

QString roleKey(Role role)
{
    switch (role) {
    case Role::Admin:
        return QStringLiteral("admin");
    case Role::Teacher:
        return QStringLiteral("teacher");
    case Role::Student:
        break;
    }
    return QStringLiteral("student");
}

QString roleLabel(Role role)
{
    switch (role) {
    case Role::Admin:
        return QStringLiteral("Administrator");
    case Role::Teacher:
        return QStringLiteral("Teacher");
    case Role::Student:
        break;
    }
    return QStringLiteral("Student");
}

std::optional<Role> roleFromKey(const QString& key)
{
    if (key == QLatin1String("admin"))
        return Role::Admin;
    if (key == QLatin1String("teacher"))
        return Role::Teacher;
    if (key == QLatin1String("student"))
        return Role::Student;
    return std::nullopt;
}

void Session::signIn(const CurrentUser& user)
{
    m_user = user;
}

void Session::signOut()
{
    m_user.reset();
}

const CurrentUser& Session::user() const
{
    static const CurrentUser nobody;
    return m_user ? *m_user : nobody;
}

void Session::setDisplayName(const QString& name)
{
    if (m_user)
        m_user->displayName = name;
}

QString Session::permissionDenied()
{
    return QStringLiteral("You don't have permission to do that.");
}

QString Session::signedOut()
{
    return QStringLiteral("Please sign in again.");
}

Result Session::requireSignedIn() const
{
    return isSignedIn() ? Result::success() : Result::failure(signedOut());
}

Result Session::requireAdmin() const
{
    if (!isSignedIn())
        return Result::failure(signedOut());
    return isAdmin() ? Result::success() : Result::failure(permissionDenied());
}

} // namespace edu
