#pragma once

#include "core/Session.h"

#include <QDateTime>
#include <QString>

namespace edu {

struct User {
    int id = 0;
    QString username;
    QString displayName;
    Role role = Role::Student;
    bool active = true;
    bool mustChangePassword = false;
    int failedLoginCount = 0;
    QDateTime lockedUntil;   // invalid when not locked
    QDateTime createdAt;
    QDateTime lastLoginAt;   // invalid when never signed in
    QString passwordHash;    // only filled by UserRepository::credentials()
    QString salt;            // only filled by UserRepository::credentials()

    bool isLocked(const QDateTime& now) const
    {
        return lockedUntil.isValid() && lockedUntil > now;
    }
};

} // namespace edu
