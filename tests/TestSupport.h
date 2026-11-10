#pragma once

// Shared helpers for the Qt Test suites.

#include "core/Database.h"
#include "core/Migrator.h"
#include "core/PasswordHasher.h"
#include "core/Session.h"
#include "repositories/UserRepository.h"

#include <QSqlQuery>
#include <QString>
#include <QtTest>

namespace edu::test {

// Real hashes are deliberately slow; tests only need them to be correct.
inline void useFastHashing()
{
    PasswordHasher::setDefaultIterations(1000);
}

// A migrated in-memory database that closes itself.
class TestDatabase {
public:
    explicit TestDatabase(QString name) : m_name(std::move(name))
    {
        m_ok = Database::open(Database::memoryPath(), m_name).ok()
               && Migrator(Database::connection(m_name)).migrate().ok();
    }
    ~TestDatabase() { Database::close(m_name); }
    TestDatabase(const TestDatabase&) = delete;
    TestDatabase& operator=(const TestDatabase&) = delete;

    bool ok() const { return m_ok; }
    const QString& name() const { return m_name; }

private:
    QString m_name;
    bool m_ok = false;
};

// Inserts an account directly (bypassing services) and returns its id.
inline int addUser(const QString& connection, const QString& username, Role role,
                   const QString& password = QStringLiteral("Secret123"), bool active = true,
                   bool mustChange = false, const QString& displayName = QString())
{
    User user;
    user.username = username;
    user.displayName = displayName.isEmpty() ? username : displayName;
    user.role = role;
    user.active = active;
    user.mustChangePassword = mustChange;
    const ValueResult<int> id =
        UserRepository(connection).insert(user, PasswordHasher::hash(password));
    return id ? id.value() : 0;
}

inline CurrentUser identity(int id, const QString& username, Role role)
{
    CurrentUser user;
    user.id = id;
    user.username = username;
    user.displayName = username;
    user.role = role;
    return user;
}

// A session signed in as a fresh account of the given role.
inline Session sessionAs(const QString& connection, const QString& username, Role role)
{
    Session session;
    session.signIn(identity(addUser(connection, username, role), username, role));
    return session;
}

} // namespace edu::test
