#pragma once

#include "core/Result.h"

#include <optional>
#include <utility>

namespace edu {

// A value, or a user-safe error message explaining why there is none.
template <typename T>
class ValueResult {
public:
    static ValueResult success(T value) { return ValueResult(std::move(value), QString()); }
    static ValueResult failure(QString message)
    {
        return ValueResult(std::nullopt, std::move(message));
    }
    static ValueResult from(const Result& result)
    {
        return ValueResult(std::nullopt, result.message());
    }

    bool ok() const { return m_value.has_value(); }
    explicit operator bool() const { return ok(); }
    const QString& message() const { return m_message; }

    const T& value() const { return *m_value; }
    T& value() { return *m_value; }

    Result toResult() const { return ok() ? Result::success() : Result::failure(m_message); }

private:
    ValueResult(std::optional<T> value, QString message)
        : m_value(std::move(value)), m_message(std::move(message))
    {
    }

    std::optional<T> m_value;
    QString m_message;
};

} // namespace edu
