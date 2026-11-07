#pragma once

#include <QString>

#include <utility>

namespace edu {

// Outcome of an operation that can fail.
// message() is always safe to show to the user. Technical details (SQL
// errors, file paths) go to the log instead, never into the message.
class Result {
public:
    static Result success() { return Result(true, QString()); }
    static Result failure(QString message) { return Result(false, std::move(message)); }

    bool ok() const { return m_ok; }
    explicit operator bool() const { return m_ok; }
    const QString& message() const { return m_message; }

private:
    Result(bool ok, QString message) : m_ok(ok), m_message(std::move(message)) {}

    bool m_ok;
    QString m_message;
};

} // namespace edu
