#pragma once

#include "core/Database.h"
#include "core/Result.h"

#include <QHash>
#include <QString>

#include <optional>

namespace edu {

// Keys stored in the settings table (seeded by 001_init.sql).
namespace SettingKey {
inline constexpr const char* InstitutionName = "institution_name";
inline constexpr const char* InstitutionLogo = "institution_logo";
inline constexpr const char* AttendanceWarnPercent = "attendance_warn_percent";
inline constexpr const char* AttendanceMinPercent = "attendance_min_percent";
inline constexpr const char* MaxCreditHoursPerTerm = "max_credit_hours_per_term";
} // namespace SettingKey

// SQL for the settings table. Values are stored as text; services convert
// and validate them.
class SettingsRepository {
public:
    explicit SettingsRepository(QString connectionName = Database::defaultConnectionName());

    // nullopt when the key does not exist or the read failed (failure is logged).
    std::optional<QString> value(const QString& key) const;

    // Inserts or replaces the value.
    Result setValue(const QString& key, const QString& value);

    QHash<QString, QString> all() const;

private:
    QSqlDatabase db() const;

    QString m_connection;
};

} // namespace edu
