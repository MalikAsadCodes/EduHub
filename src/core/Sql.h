#pragma once

#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QVariant>

#include <optional>

namespace edu::sql {

// Small helpers used by repositories. Errors are logged with a context string
// (never with bound values, which may hold personal data).

// Prepares a statement; logs and returns false on failure.
bool prepare(QSqlQuery& query, const QString& statement, const char* context);

// Executes a prepared statement; logs and returns false on failure.
bool exec(QSqlQuery& query, const char* context);

// Executes a plain statement; logs and returns false on failure.
bool exec(QSqlQuery& query, const QString& statement, const char* context);

// Empty text becomes SQL NULL.
QVariant textOrNull(const QString& text);
QVariant intOrNull(const std::optional<int>& value);
std::optional<int> optionalInt(const QVariant& value);

// Current time as ISO-8601 UTC text, e.g. 2026-10-24T09:30:00Z.
QString nowUtc();
QString toUtcText(const QDateTime& time);
QDateTime fromUtcText(const QString& text);

// Last inserted row id of a query.
int lastId(const QSqlQuery& query);

} // namespace edu::sql
