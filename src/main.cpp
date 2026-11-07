#include "core/Database.h"
#include "core/Logger.h"
#include "core/Migrator.h"
#include "ui/AppController.h"
#include "ui/Fonts.h"
#include "ui/UiSettings.h"
#include "ui/theme/Theme.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>

namespace {

void showFatal(const QString& message)
{
    QMessageBox box(QMessageBox::Critical, QStringLiteral("Edu Hub"), message, QMessageBox::Ok);
    box.setInformativeText(QStringLiteral("Log file: %1").arg(edu::Logger::filePath()));
    box.exec();
}

} // namespace

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QApplication::setOrganizationName(QStringLiteral("MalikAsadCodes"));
    QApplication::setApplicationName(QStringLiteral("EduHub"));
    QApplication::setApplicationVersion(QStringLiteral(EDUHUB_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Edu Hub - university management"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption dbOption(
        QStringLiteral("db"), QStringLiteral("Use this database file instead of the default."),
        QStringLiteral("file"));
    const QCommandLineOption themeOption(QStringLiteral("theme"),
                                         QStringLiteral("Start with the light or dark theme."),
                                         QStringLiteral("light|dark"));
    parser.addOption(dbOption);
    parser.addOption(themeOption);
    parser.process(app);

    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    edu::Logger::install(QDir(dataDir).filePath(QStringLiteral("logs")));
    qInfo().noquote() << "Edu Hub" << EDUHUB_VERSION << "starting";

    edu::ui::Fonts::install();
    edu::ui::Theme& theme = edu::ui::Theme::instance();
    edu::ui::ThemeMode mode = edu::ui::UiSettings::themeMode();
    if (parser.isSet(themeOption))
        mode = edu::ui::Theme::modeFromName(parser.value(themeOption), mode);
    theme.setMode(mode);
    theme.apply();

    const QString dbPath = parser.isSet(dbOption)
                               ? QFileInfo(parser.value(dbOption)).absoluteFilePath()
                               : edu::Database::defaultFilePath();
    if (const edu::Result opened = edu::Database::open(dbPath); !opened) {
        showFatal(opened.message());
        return 1;
    }
    {
        edu::Migrator migrator(edu::Database::connection());
        if (const edu::Result migrated = migrator.migrate(); !migrated) {
            showFatal(migrated.message());
            edu::Database::close();
            return 1;
        }
        qInfo().noquote() << "Database ready at schema version" << migrator.currentVersion();
    }

    int exitCode = 0;
    {
        edu::ui::AppController controller(edu::Database::defaultConnectionName());
        controller.start();
        exitCode = QApplication::exec();
    }

    edu::Database::close();
    qInfo() << "Edu Hub closed";
    edu::Logger::shutdown();
    return exitCode;
}
