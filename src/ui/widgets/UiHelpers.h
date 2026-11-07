#pragma once

#include <QString>
#include <QVariant>

class QFrame;
class QLabel;
class QWidget;

namespace edu::ui {

// Re-applies the stylesheet after a dynamic property changed.
void repolish(QWidget* widget);

// Sets a style property (used by app.qss selectors such as QLabel[role="muted"]).
void setStyleProperty(QWidget* widget, const char* name, const QVariant& value);

// A label with an app.qss role, e.g. "title", "muted", "pill".
QLabel* makeLabel(const QString& text, const QString& role = QString(), QWidget* parent = nullptr);

// A rounded surface card (QFrame[card="true"]).
QFrame* makeCard(QWidget* parent = nullptr);

} // namespace edu::ui
