#pragma once

#include "core/Database.h"
#include "repositories/SettingsRepository.h"

#include <QString>

namespace edu {

struct SystemStatus {
    QString databasePath;
    int schemaVersion = 0;
    int latestSchemaVersion = 0;
    QString institutionName;
    int attendanceWarnPercent = 80;
    int attendanceMinPercent = 75;
    int maxCreditHoursPerTerm = 21;
};

// Read-only facts about this installation, for the dashboard and login screen.
// Settings are validated here: a missing or out-of-range value falls back to
// the documented default instead of breaking the UI.
class SystemService {
public:
    explicit SystemService(QString connectionName = Database::defaultConnectionName());

    SystemStatus status() const;
    QString institutionName() const;

    static constexpr int kDefaultWarnPercent = 80;
    static constexpr int kDefaultMinPercent = 75;
    static constexpr int kDefaultMaxCreditHours = 21;

private:
    int intSetting(const char* key, int fallback, int minimum, int maximum) const;

    QString m_connection;
    SettingsRepository m_settings;
};

} // namespace edu
