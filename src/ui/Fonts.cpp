#include "ui/Fonts.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QFontDatabase>

namespace edu::ui::Fonts {

void install()
{
    bool loaded = false;
    const QDir dir(QStringLiteral(":/fonts"));
    const QStringList files = dir.entryList({QStringLiteral("*.ttf")}, QDir::Files);
    for (const QString& file : files) {
        if (QFontDatabase::addApplicationFont(dir.filePath(file)) >= 0)
            loaded = true;
        else
            qWarning().noquote() << "Fonts: could not load" << file;
    }

    QFont font = QApplication::font();
    if (loaded)
        font.setFamilies({QStringLiteral("Inter"), QStringLiteral("Segoe UI")});
    font.setPointSizeF(10.0);
    font.setHintingPreference(QFont::PreferVerticalHinting);
    QApplication::setFont(font);
}

} // namespace edu::ui::Fonts
