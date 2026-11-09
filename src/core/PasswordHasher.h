#pragma once

#include "core/Result.h"

#include <QByteArray>
#include <QString>

namespace edu::PasswordHasher {

// Passwords are stored only as salted PBKDF2-HMAC-SHA256 hashes.
// hash text format:  pbkdf2_sha256$<iterations>$<base64 key>
// salt:              16 random bytes, base64

struct Hashed {
    QString hash;
    QString salt;
};

Hashed hash(const QString& password);
Hashed hash(const QString& password, int iterations);

// Constant-time check against a stored hash and salt.
bool verify(const QString& password, const QString& hashText, const QString& salt);

// Raw PBKDF2-HMAC-SHA256 (RFC 8018). Exposed for the test vectors.
QByteArray pbkdf2Sha256(const QByteArray& password, const QByteArray& salt, int iterations,
                        int keyLength);

// Work factor for new hashes. Tests lower it to stay fast; stored hashes keep
// the count they were made with, so verification is unaffected.
int defaultIterations();
void setDefaultIterations(int iterations);

inline constexpr int kProductionIterations = 120000;

} // namespace edu::PasswordHasher

namespace edu::PasswordPolicy {

inline constexpr int kMinLength = 8;
inline constexpr int kMaxLength = 128;

// At least 8 characters, with a letter and a digit, and not the username.
Result check(const QString& password, const QString& username = QString());

// Short description for hint labels.
QString hint();

// A readable temporary password that passes the policy, e.g. "kpmt-4821".
QString generateTemporary();

} // namespace edu::PasswordPolicy
