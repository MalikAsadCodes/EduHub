#include "ui/widgets/UiHelpers.h"

#include <QFont>
#include <QFrame>
#include <QLabel>
#include <QStyle>
#include <QWidget>

namespace edu::ui {

void repolish(QWidget* widget)
{
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

void setStyleProperty(QWidget* widget, const char* name, const QVariant& value)
{
    widget->setProperty(name, value);
    repolish(widget);
}

QLabel* makeLabel(const QString& text, const QString& role, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    if (!role.isEmpty())
        label->setProperty("role", role);
    static const QStringList eyebrowRoles = {
        QStringLiteral("eyebrow"), QStringLiteral("heroEyebrow"), QStringLiteral("statLabel"),
        QStringLiteral("planSemester"), QStringLiteral("cardEyebrow")};
    if (eyebrowRoles.contains(role))
        styleEyebrow(label);
    return label;
}

void styleEyebrow(QLabel* label, qreal spacing)
{
    label->setText(label->text().toUpper());
    QFont font = label->font();
    font.setLetterSpacing(QFont::AbsoluteSpacing, spacing);
    label->setFont(font);
}

QFrame* makeCard(QWidget* parent)
{
    auto* card = new QFrame(parent);
    card->setProperty("card", true);
    return card;
}

QString formatDateTime(const QDateTime& time, const QString& fallback)
{
    if (!time.isValid())
        return fallback;
    return time.toLocalTime().toString(QStringLiteral("d MMM yyyy, h:mm AP"));
}

QString formatDate(const QDate& date, const QString& fallback)
{
    if (!date.isValid())
        return fallback;
    return date.toString(QStringLiteral("d MMM yyyy"));
}

QString plural(int count, const QString& singular, const QString& pluralForm)
{
    const QString many = pluralForm.isEmpty() ? singular + QLatin1Char('s') : pluralForm;
    return QStringLiteral("%1 %2").arg(count).arg(count == 1 ? singular : many);
}

} // namespace edu::ui
