#include "services/SystemService.h"

#include "core/Migrator.h"

#include <QDebug>

#include <utility>

namespace edu {

SystemService::SystemService(QString connectionName)
    : m_connection(std::move(connectionName)), m_settings(m_connection)
{
}

QString SystemService::institutionName() const
{
    const QString name =
        m_settings.value(SettingKey::InstitutionName).value_or(QString()).trimmed();
    return name.isEmpty() ? QStringLiteral("Edu Hub University") : name;
}

int SystemService::intSetting(const char* key, int fallback, int minimum, int maximum) const
{
    const std::optional<QString> raw = m_settings.value(QString::fromLatin1(key));
    if (!raw)
        return fallback;
    bool ok = false;
    const int value = raw->trimmed().toInt(&ok);
    if (!ok || value < minimum || value > maximum) {
        qWarning().noquote() << "SystemService: setting" << key << "has an invalid value, using"
                             << fallback;
        return fallback;
    }
    return value;
}

SystemStatus SystemService::status() const
{
    SystemStatus status;
    const QSqlDatabase db = Database::connection(m_connection);
    const Migrator migrator(db);

    status.databasePath = db.databaseName();
    status.schemaVersion = migrator.currentVersion();
    status.latestSchemaVersion = migrator.latestVersion();
    status.institutionName = institutionName();
    status.attendanceWarnPercent =
        intSetting(SettingKey::AttendanceWarnPercent, kDefaultWarnPercent, 1, 100);
    status.attendanceMinPercent =
        intSetting(SettingKey::AttendanceMinPercent, kDefaultMinPercent, 1, 100);
    status.maxCreditHoursPerTerm =
        intSetting(SettingKey::MaxCreditHoursPerTerm, kDefaultMaxCreditHours, 1, 40);

    // The warning line must sit at or above the eligibility line.
    if (status.attendanceWarnPercent < status.attendanceMinPercent)
        status.attendanceWarnPercent = status.attendanceMinPercent;
    return status;
}

} // namespace edu
