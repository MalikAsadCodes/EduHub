#include "ui/widgets/UiHelpers.h"

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
    return label;
}

QFrame* makeCard(QWidget* parent)
{
    auto* card = new QFrame(parent);
    card->setProperty("card", true);
    return card;
}

} // namespace edu::ui
