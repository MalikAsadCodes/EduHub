#include "repositories/SettingsRepository.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include <utility>

namespace edu {

SettingsRepository::SettingsRepository(QString connectionName)
    : m_connection(std::move(connectionName))
{
}

QSqlDatabase SettingsRepository::db() const
{
    return Database::connection(m_connection);
}

std::optional<QString> SettingsRepository::value(const QString& key) const
{
    QSqlQuery query(db());
    query.prepare(QStringLiteral("SELECT value FROM settings WHERE key = ?"));
    query.addBindValue(key);
    if (!query.exec()) {
        qWarning().noquote() << "SettingsRepository: read" << key
                             << "failed:" << query.lastError().text();
        return std::nullopt;
    }
    if (!query.next())
        return std::nullopt;
    return query.value(0).toString();
}

Result SettingsRepository::setValue(const QString& key, const QString& value)
{
    QSqlQuery query(db());
    query.prepare(QStringLiteral("INSERT INTO settings (key, value) VALUES (?, ?) "
                                 "ON CONFLICT(key) DO UPDATE SET value = excluded.value"));
    query.addBindValue(key);
    query.addBindValue(value);
    if (!query.exec()) {
        qWarning().noquote() << "SettingsRepository: write" << key
                             << "failed:" << query.lastError().text();
        return Result::failure(QStringLiteral("The setting could not be saved."));
    }
    return Result::success();
}

QHash<QString, QString> SettingsRepository::all() const
{
    QHash<QString, QString> settings;
    QSqlQuery query(db());
    if (!query.exec(QStringLiteral("SELECT key, value FROM settings"))) {
        qWarning().noquote() << "SettingsRepository: read all failed:" << query.lastError().text();
        return settings;
    }
    while (query.next())
        settings.insert(query.value(0).toString(), query.value(1).toString());
    return settings;
}

} // namespace edu
