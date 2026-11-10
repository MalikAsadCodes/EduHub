#pragma once

#include "core/Database.h"
#include "core/Session.h"
#include "core/ValueResult.h"
#include "repositories/UserRepository.h"

#include <QDateTime>

#include <functional>

namespace edu {

struct SignInOutcome {
    CurrentUser user;
    bool mustChangePassword = false;
};

// Sign-in, first-run setup and password changes.
//
// Rules:
//  - 5 wrong passwords in a row lock the account for 5 minutes.
//  - Disabled accounts cannot sign in.
//  - Error messages never say whether a username exists.
class AuthService {
public:
    using Clock = std::function<QDateTime()>;

    static constexpr int kMaxFailedAttempts = 5;
    static constexpr int kLockMinutes = 5;

    explicit AuthService(QString connectionName = Database::defaultConnectionName(),
                         Clock clock = {});

    // True until the first account exists.
    bool needsSetup() const;

    // Creates the first administrator (only allowed while there are no accounts)
    // and stores the institution name. Returns the signed-in user.
    ValueResult<CurrentUser> createFirstAdmin(const QString& institutionName,
                                              const QString& displayName,
                                              const QString& username, const QString& password);

    ValueResult<SignInOutcome> signIn(const QString& username, const QString& password);

    // Changes the signed-in user's own password.
    Result changePassword(const Session& session, const QString& currentPassword,
                          const QString& newPassword);

    // Builds the session identity for an account (adds teacher/student profile ids).
    CurrentUser identityFor(const User& user) const;

    static Result checkUsername(const QString& username);
    static Result checkDisplayName(const QString& name);

private:
    QDateTime now() const;

    QString m_connection;
    UserRepository m_users;
    Clock m_clock;
};

} // namespace edu
