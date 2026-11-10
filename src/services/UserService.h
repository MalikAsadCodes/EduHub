#pragma once

#include "core/Database.h"
#include "core/Session.h"
#include "core/ValueResult.h"
#include "repositories/UserRepository.h"

#include <QList>

namespace edu {

struct NewAccount {
    int userId = 0;
    QString username;
    QString temporaryPassword; // shown once, never stored in plain text
};

// Account administration (administrators only).
// Teacher and student accounts are created together with their profiles;
// this service creates administrator accounts and manages every account.
class UserService {
public:
    UserService(QString connectionName, const Session& session);

    ValueResult<QList<User>> list() const;

    ValueResult<NewAccount> createAdmin(const QString& displayName, const QString& username);

    // Issues a temporary password; the user must change it at next sign-in.
    ValueResult<QString> resetPassword(int userId);

    Result setActive(int userId, bool active);
    Result unlock(int userId);
    Result rename(int userId, const QString& displayName);

private:
    QString m_connection;
    const Session& m_session;
    UserRepository m_users;
};

} // namespace edu
