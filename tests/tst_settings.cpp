#include "core/Database.h"
#include "core/Migrator.h"
#include "repositories/SettingsRepository.h"
#include "services/SystemService.h"

#include <QtTest>

using edu::Database;
using edu::SettingsRepository;
using edu::SystemService;
namespace SettingKey = edu::SettingKey;

class TestSettings : public QObject {
    Q_OBJECT

private:
    const QString m_name = QStringLiteral("settings_test");

private slots:
    void init()
    {
        QVERIFY(Database::open(Database::memoryPath(), m_name).ok());
        QVERIFY(edu::Migrator(Database::connection(m_name)).migrate().ok());
    }
    void cleanup() { Database::close(m_name); }

    void seededDefaultsMatchTheDecisions()
    {
        const SettingsRepository settings(m_name);
        QCOMPARE(settings.value(SettingKey::AttendanceWarnPercent).value_or(QString()),
                 QStringLiteral("80"));
        QCOMPARE(settings.value(SettingKey::AttendanceMinPercent).value_or(QString()),
                 QStringLiteral("75"));
        QCOMPARE(settings.value(SettingKey::MaxCreditHoursPerTerm).value_or(QString()),
                 QStringLiteral("21"));
        QVERIFY(settings.value(SettingKey::InstitutionName).has_value());
    }

    void missingKeyIsEmpty()
    {
        const SettingsRepository settings(m_name);
        QVERIFY(!settings.value(QStringLiteral("no_such_key")).has_value());
    }

    void setValueInsertsAndUpdates()
    {
        SettingsRepository settings(m_name);
        QVERIFY(settings.setValue(QStringLiteral("custom"), QStringLiteral("one")).ok());
        QVERIFY(settings.setValue(QStringLiteral("custom"), QStringLiteral("two")).ok());
        QCOMPARE(settings.value(QStringLiteral("custom")).value_or(QString()),
                 QStringLiteral("two"));
        QCOMPARE(settings.all().value(QStringLiteral("custom")), QStringLiteral("two"));
    }

    void statusReadsLiveValues()
    {
        SettingsRepository settings(m_name);
        QVERIFY(settings.setValue(SettingKey::InstitutionName, QStringLiteral("Riverside College"))
                    .ok());
        QVERIFY(settings.setValue(SettingKey::MaxCreditHoursPerTerm, QStringLiteral("18")).ok());

        const edu::SystemStatus status = SystemService(m_name).status();
        QCOMPARE(status.institutionName, QStringLiteral("Riverside College"));
        QCOMPARE(status.maxCreditHoursPerTerm, 18);
        QCOMPARE(status.schemaVersion, status.latestSchemaVersion);
        QVERIFY(status.schemaVersion >= 1);
    }

    void invalidValuesFallBackToDefaults()
    {
        SettingsRepository settings(m_name);
        QVERIFY(settings.setValue(SettingKey::AttendanceMinPercent, QStringLiteral("abc")).ok());
        QVERIFY(settings.setValue(SettingKey::MaxCreditHoursPerTerm, QStringLiteral("500")).ok());
        QVERIFY(settings.setValue(SettingKey::InstitutionName, QStringLiteral("   ")).ok());

        const edu::SystemStatus status = SystemService(m_name).status();
        QCOMPARE(status.attendanceMinPercent, SystemService::kDefaultMinPercent);
        QCOMPARE(status.maxCreditHoursPerTerm, SystemService::kDefaultMaxCreditHours);
        QCOMPARE(status.institutionName, QStringLiteral("Edu Hub University"));
    }

    void warningLineNeverBelowMinimum()
    {
        SettingsRepository settings(m_name);
        QVERIFY(settings.setValue(SettingKey::AttendanceWarnPercent, QStringLiteral("60")).ok());
        QVERIFY(settings.setValue(SettingKey::AttendanceMinPercent, QStringLiteral("75")).ok());
        const edu::SystemStatus status = SystemService(m_name).status();
        QCOMPARE(status.attendanceWarnPercent, 75);
    }
};

QTEST_GUILESS_MAIN(TestSettings)
#include "tst_settings.moc"
