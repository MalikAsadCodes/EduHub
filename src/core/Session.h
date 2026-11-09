#pragma once

#include "core/Result.h"

#include <QString>

#include <optional>

namespace edu {

enum class Role { Admin, Teacher, Student };

QString roleKey(Role role);   // "admin", "teacher", "student" (as stored in the database)
QString roleLabel(Role role); // "Administrator", "Teacher", "Student"
std::optional<Role> roleFromKey(const QString& key);

struct CurrentUser {
    int id = 0;
    QString username;
    QString displayName;
    Role role = Role::Student;
    std::optional<int> teacherId; // set when the account belongs to a teacher profile
    std::optional<int> studentId; // set when the account belongs to a student profile
};

// Who is signed in. Every service checks it before reading or changing
// records, so hiding a button in the UI is never the only protection.
class Session {
public:
    void signIn(const CurrentUser& user);
    void signOut();

    bool isSignedIn() const { return m_user.has_value(); }
    const CurrentUser& user() const;
    int userId() const { return m_user ? m_user->id : 0; }

    bool isAdmin() const { return m_user && m_user->role == Role::Admin; }
    bool isTeacher() const { return m_user && m_user->role == Role::Teacher; }
    bool isStudent() const { return m_user && m_user->role == Role::Student; }

    void setDisplayName(const QString& name);

    Result requireSignedIn() const;
    Result requireAdmin() const;

    static QString permissionDenied();
    static QString signedOut();

private:
    std::optional<CurrentUser> m_user;
};

} // namespace edu
