#include "core/Logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>

#include <memory>

namespace edu::Logger {

namespace {

constexpr qint64 kMaxBytes = 1024 * 1024;

QMutex g_mutex;
std::unique_ptr<QFile> g_file;
QString g_path;
QtMessageHandler g_previous = nullptr;

QLatin1Char levelLetter(QtMsgType type)
{
    switch (type) {
    case QtDebugMsg:
        return QLatin1Char('D');
    case QtInfoMsg:
        return QLatin1Char('I');
    case QtWarningMsg:
        return QLatin1Char('W');
    case QtCriticalMsg:
        return QLatin1Char('E');
    case QtFatalMsg:
        return QLatin1Char('F');
    }
    return QLatin1Char('?');
}

void handler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    {
        QMutexLocker lock(&g_mutex);
        if (g_file) {
            const QString line = QStringLiteral("%1 [%2] %3\n")
                                     .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODate),
                                          QString(levelLetter(type)), message);
            g_file->write(line.toUtf8());
            g_file->flush();
        }
    }
    if (g_previous)
        g_previous(type, context, message);
}

} // namespace

bool install(const QString& directory)
{
    if (!QDir().mkpath(directory))
        return false;

    const QString path = QDir(directory).filePath(QStringLiteral("eduhub.log"));
    const QFileInfo info(path);
    if (info.exists() && info.size() > kMaxBytes) {
        QFile::remove(path + QStringLiteral(".1"));
        QFile::rename(path, path + QStringLiteral(".1"));
    }

    auto file = std::make_unique<QFile>(path);
    if (!file->open(QIODevice::Append | QIODevice::Text))
        return false;

    {
        QMutexLocker lock(&g_mutex);
        g_file = std::move(file);
        g_path = path;
    }
    QtMessageHandler previous = qInstallMessageHandler(handler);
    if (previous != handler)
        g_previous = previous;
    return true;
}

void shutdown()
{
    qInstallMessageHandler(g_previous);
    QMutexLocker lock(&g_mutex);
    g_file.reset();
}

QString filePath()
{
    QMutexLocker lock(&g_mutex);
    return g_path;
}

} // namespace edu::Logger
