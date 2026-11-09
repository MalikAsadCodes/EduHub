#include "core/Sql.h"

#include <QDebug>
#include <QSqlError>
#include <QTimeZone>

namespace edu::sql {

bool prepare(QSqlQuery& query, const QString& statement, const char* context)
{
    if (query.prepare(statement))
        return true;
    qWarning().noquote() << context << "- prepare failed:" << query.lastError().text();
    return false;
}

bool exec(QSqlQuery& query, const char* context)
{
    if (query.exec())
        return true;
    qWarning().noquote() << context << "- query failed:" << query.lastError().text();
    return false;
}

bool exec(QSqlQuery& query, const QString& statement, const char* context)
{
    if (query.exec(statement))
        return true;
    qWarning().noquote() << context << "- query failed:" << query.lastError().text();
    return false;
}

QVariant textOrNull(const QString& text)
{
    const QString trimmed = text.trimmed();
    return trimmed.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : QVariant(trimmed);
}

QVariant intOrNull(const std::optional<int>& value)
{
    return value ? QVariant(*value) : QVariant(QMetaType(QMetaType::Int));
}

std::optional<int> optionalInt(const QVariant& value)
{
    if (value.isNull())
        return std::nullopt;
    return value.toInt();
}

QString toUtcText(const QDateTime& time)
{
    return time.toUTC().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ssZ"));
}

QString nowUtc()
{
    return toUtcText(QDateTime::currentDateTimeUtc());
}

QDateTime fromUtcText(const QString& text)
{
    QDateTime time = QDateTime::fromString(text, Qt::ISODate);
    if (time.isValid() && !text.endsWith(QLatin1Char('Z')))
        time = QDateTime(time.date(), time.time(), QTimeZone::UTC); // stored times are UTC
    return time.toUTC();
}

int lastId(const QSqlQuery& query)
{
    return query.lastInsertId().toInt();
}

} // namespace edu::sql
