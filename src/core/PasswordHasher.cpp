#include "core/PasswordHasher.h"

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QStringList>

#include <atomic>

namespace edu::PasswordHasher {

namespace {

constexpr int kSaltBytes = 16;
constexpr int kKeyBytes = 32;
const QString kScheme = QStringLiteral("pbkdf2_sha256");

std::atomic<int> g_defaultIterations{kProductionIterations};

QByteArray randomBytes(int count)
{
    QByteArray bytes(count, Qt::Uninitialized);
    QRandomGenerator* generator = QRandomGenerator::system();
    for (int i = 0; i < count; ++i)
        bytes[i] = static_cast<char>(generator->bounded(256));
    return bytes;
}

bool constantTimeEquals(const QByteArray& a, const QByteArray& b)
{
    if (a.size() != b.size())
        return false;
    unsigned char difference = 0;
    for (qsizetype i = 0; i < a.size(); ++i)
        difference |= static_cast<unsigned char>(a.at(i) ^ b.at(i));
    return difference == 0;
}

} // namespace

QByteArray pbkdf2Sha256(const QByteArray& password, const QByteArray& salt, int iterations,
                        int keyLength)
{
    if (iterations < 1 || keyLength < 1)
        return {};

    constexpr int kBlockBytes = 32; // SHA-256 output
    QMessageAuthenticationCode mac(QCryptographicHash::Sha256, password);
    QByteArray key;
    key.reserve(keyLength);

    for (quint32 block = 1; key.size() < keyLength; ++block) {
        QByteArray counter(4, Qt::Uninitialized);
        counter[0] = static_cast<char>((block >> 24) & 0xFF);
        counter[1] = static_cast<char>((block >> 16) & 0xFF);
        counter[2] = static_cast<char>((block >> 8) & 0xFF);
        counter[3] = static_cast<char>(block & 0xFF);

        mac.reset();
        mac.addData(salt);
        mac.addData(counter);
        QByteArray u = mac.result();
        QByteArray t = u;
        for (int i = 1; i < iterations; ++i) {
            mac.reset();
            mac.addData(u);
            u = mac.result();
            for (int j = 0; j < kBlockBytes; ++j)
                t[j] = static_cast<char>(t.at(j) ^ u.at(j));
        }
        key.append(t);
    }
    key.truncate(keyLength);
    return key;
}

Hashed hash(const QString& password, int iterations)
{
    const QByteArray salt = randomBytes(kSaltBytes);
    const QByteArray key = pbkdf2Sha256(password.toUtf8(), salt, iterations, kKeyBytes);
    return {QStringLiteral("%1$%2$%3")
                .arg(kScheme)
                .arg(iterations)
                .arg(QString::fromLatin1(key.toBase64())),
            QString::fromLatin1(salt.toBase64())};
}

Hashed hash(const QString& password)
{
    return hash(password, defaultIterations());
}

bool verify(const QString& password, const QString& hashText, const QString& salt)
{
    const QStringList parts = hashText.split(QLatin1Char('$'));
    if (parts.size() != 3 || parts.at(0) != kScheme)
        return false;
    bool ok = false;
    const int iterations = parts.at(1).toInt(&ok);
    if (!ok || iterations < 1 || iterations > 10'000'000)
        return false;
    const QByteArray expected = QByteArray::fromBase64(parts.at(2).toLatin1());
    if (expected.isEmpty())
        return false;
    const QByteArray actual = pbkdf2Sha256(
        password.toUtf8(), QByteArray::fromBase64(salt.toLatin1()), iterations,
        static_cast<int>(expected.size()));
    return constantTimeEquals(actual, expected);
}

int defaultIterations()
{
    return g_defaultIterations.load();
}

void setDefaultIterations(int iterations)
{
    g_defaultIterations.store(iterations < 1 ? 1 : iterations);
}

} // namespace edu::PasswordHasher

namespace edu::PasswordPolicy {

QString hint()
{
    return QStringLiteral("At least %1 characters, with letters and numbers.").arg(kMinLength);
}

Result check(const QString& password, const QString& username)
{
    if (password.size() < kMinLength)
        return Result::failure(
            QStringLiteral("The password must be at least %1 characters long.").arg(kMinLength));
    if (password.size() > kMaxLength)
        return Result::failure(
            QStringLiteral("The password can be at most %1 characters long.").arg(kMaxLength));

    bool hasLetter = false;
    bool hasDigit = false;
    for (const QChar c : password) {
        hasLetter = hasLetter || c.isLetter();
        hasDigit = hasDigit || c.isDigit();
    }
    if (!hasLetter || !hasDigit)
        return Result::failure(QStringLiteral("Use both letters and numbers in the password."));

    const QString name = username.trimmed();
    if (name.size() >= 3 && password.contains(name, Qt::CaseInsensitive))
        return Result::failure(QStringLiteral("The password must not contain the username."));
    return Result::success();
}

QString generateTemporary()
{
    // No i, l, o: easy to read aloud and copy by hand.
    static const QString letters = QStringLiteral("abcdefghjkmnpqrstuvwxyz");
    QRandomGenerator* generator = QRandomGenerator::system();
    QString text;
    for (int i = 0; i < 4; ++i)
        text += letters.at(static_cast<qsizetype>(generator->bounded(int(letters.size()))));
    text += QLatin1Char('-');
    for (int i = 0; i < 4; ++i)
        text += QChar(u'2' + generator->bounded(8)); // digits 2-9
    return text;
}

} // namespace edu::PasswordPolicy
