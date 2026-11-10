#include "services/AuthService.h"

#include "core/PasswordHasher.h"
#include "core/Transaction.h"
#include "repositories/SettingsRepository.h"

#include <QDebug>
#include <QRegularExpression>

#include <cmath>
#include <utility>

namespace edu {

namespace {

QString wrongCredentials()
{
    return QStringLiteral("Wrong username or password.");
}

} // namespace

AuthService::AuthService(QString connectionName, Clock clock)
    : m_connection(std::move(connectionName)), m_users(m_connection), m_clock(std::move(clock))
{
}

QDateTime AuthService::now() const
{
    return m_clock ? m_clock() : QDateTime::currentDateTimeUtc();
}

Result AuthService::checkUsername(const QString& username)
{
    static const QRegularExpression pattern(QStringLiteral("^[A-Za-z0-9][A-Za-z0-9._-]{2,31}$"));
    if (!pattern.match(username.trimmed()).hasMatch())
        return Result::failure(QStringLiteral(
            "Usernames are 3 to 32 characters: letters, numbers, dots, dashes or underscores."));
    return Result::success();
}

Result AuthService::checkDisplayName(const QString& name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.size() < 2 || trimmed.size() > 80)
        return Result::failure(QStringLiteral("Enter a full name of 2 to 80 characters."));
    return Result::success();
}

bool AuthService::needsSetup() const
{
    return m_users.count() == 0;
}

CurrentUser AuthService::identityFor(const User& user) const
{
    CurrentUser identity;
    identity.id = user.id;
    identity.username = user.username;
    identity.displayName = user.displayName.isEmpty() ? user.username : user.displayName;
    identity.role = user.role;
    return identity;
}

ValueResult<CurrentUser> AuthService::createFirstAdmin(const QString& institutionName,
                                                       const QString& displayName,
                                                       const QString& username,
                                                       const QString& password)
{
    if (Result r = checkDisplayName(displayName); !r)
        return ValueResult<CurrentUser>::from(r);
    if (Result r = checkUsername(username); !r)
        return ValueResult<CurrentUser>::from(r);
    if (Result r = PasswordPolicy::check(password, username); !r)
        return ValueResult<CurrentUser>::from(r);
    const QString institution = institutionName.trimmed();
    if (institution.size() > 120)
        return ValueResult<CurrentUser>::failure(
            QStringLiteral("The institution name can be at most 120 characters."));

    const PasswordHasher::Hashed hashed = PasswordHasher::hash(password);

    Transaction transaction(Database::connection(m_connection));
    if (!transaction.isActive())
        return ValueResult<CurrentUser>::failure(
            QStringLiteral("Edu Hub could not save the account. Please try again."));
    if (m_users.count() != 0)
        return ValueResult<CurrentUser>::failure(
            QStringLiteral("Edu Hub is already set up. Please sign in."));

    User user;
    user.username = username.trimmed();
    user.displayName = displayName.trimmed();
    user.role = Role::Admin;
    const ValueResult<int> inserted = m_users.insert(user, hashed);
    if (!inserted)
        return ValueResult<CurrentUser>::from(inserted.toResult());
    user.id = inserted.value();

    if (!institution.isEmpty()) {
        SettingsRepository settings(m_connection);
        if (Result r = settings.setValue(SettingKey::InstitutionName, institution); !r)
            return ValueResult<CurrentUser>::from(r);
    }
    m_users.recordSuccessfulSignIn(user.id, now());
    if (!transaction.commit())
        return ValueResult<CurrentUser>::failure(
            QStringLiteral("Edu Hub could not save the account. Please try again."));

    qInfo() << "Auth: first administrator created";
    return ValueResult<CurrentUser>::success(identityFor(user));
}

ValueResult<SignInOutcome> AuthService::signIn(const QString& username, const QString& password)
{
    if (username.trimmed().isEmpty() || password.isEmpty())
        return ValueResult<SignInOutcome>::failure(
            QStringLiteral("Enter your username and password."));

    const std::optional<User> found = m_users.credentials(username);
    if (!found) {
        // Spend the same time as a real check so timing does not reveal usernames.
        PasswordHasher::verify(password,
                               QStringLiteral("pbkdf2_sha256$%1$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=")
                                   .arg(PasswordHasher::defaultIterations()),
                               QStringLiteral("AAAAAAAAAAAAAAAAAAAAAA=="));
        return ValueResult<SignInOutcome>::failure(wrongCredentials());
    }
    const User& user = *found;
    const QDateTime current = now();

    if (user.isLocked(current)) {
        const qint64 seconds = current.secsTo(user.lockedUntil);
        const int minutes = static_cast<int>(std::max<qint64>(1, (seconds + 59) / 60));
        return ValueResult<SignInOutcome>::failure(
            QStringLiteral("Too many wrong attempts. Try again in %1 minute%2.")
                .arg(minutes)
                .arg(minutes == 1 ? QString() : QStringLiteral("s")));
    }

    if (!PasswordHasher::verify(password, user.passwordHash, user.salt)) {
        // A lock that has expired starts a fresh count.
        const int previous = user.lockedUntil.isValid() ? 0 : user.failedLoginCount;
        const int failures = previous + 1;
        QDateTime lockUntil;
        if (failures >= kMaxFailedAttempts)
            lockUntil = current.addSecs(kLockMinutes * 60);
        m_users.recordFailedSignIn(user.id, lockUntil.isValid() ? 0 : failures, lockUntil);
        qInfo() << "Auth: failed sign-in for account" << user.id;
        if (lockUntil.isValid())
            return ValueResult<SignInOutcome>::failure(
                QStringLiteral("Too many wrong attempts. The account is locked for %1 minutes.")
                    .arg(kLockMinutes));
        return ValueResult<SignInOutcome>::failure(wrongCredentials());
    }

    if (!user.active)
        return ValueResult<SignInOutcome>::failure(
            QStringLiteral("This account is disabled. Please ask your administrator."));

    m_users.recordSuccessfulSignIn(user.id, current);
    qInfo() << "Auth: account" << user.id << "signed in";
    return ValueResult<SignInOutcome>::success({identityFor(user), user.mustChangePassword});
}

Result AuthService::changePassword(const Session& session, const QString& currentPassword,
                                   const QString& newPassword)
{
    if (Result r = session.requireSignedIn(); !r)
        return r;
    const std::optional<User> user = m_users.credentials(session.user().username);
    if (!user || user->id != session.userId())
        return Result::failure(Session::signedOut());
    if (!PasswordHasher::verify(currentPassword, user->passwordHash, user->salt))
        return Result::failure(QStringLiteral("The current password is not correct."));
    if (currentPassword == newPassword)
        return Result::failure(QStringLiteral("Choose a password different from the current one."));
    if (Result r = PasswordPolicy::check(newPassword, user->username); !r)
        return r;
    if (Result r = m_users.setPassword(user->id, PasswordHasher::hash(newPassword), false); !r)
        return r;
    qInfo() << "Auth: account" << user->id << "changed its password";
    return Result::success();
}

} // namespace edu
