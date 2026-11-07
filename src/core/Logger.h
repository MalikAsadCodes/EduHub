#pragma once

#include <QString>

namespace edu::Logger {

// Sends qDebug/qInfo/qWarning/qCritical output to <directory>/eduhub.log as
// well as the console. The file rolls over to eduhub.log.1 at 1 MB.
// Never log passwords, hashes or personal details such as full addresses.
bool install(const QString& directory);

// Stops writing to the file (called before the app exits).
void shutdown();

QString filePath();

} // namespace edu::Logger
