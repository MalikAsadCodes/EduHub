#pragma once

#include <QDate>
#include <QDateTime>
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
// Eyebrow roles ("eyebrow", "heroEyebrow", "statLabel", ...) are upper-cased
// and letter-spaced, because stylesheets cannot do either.
QLabel* makeLabel(const QString& text, const QString& role = QString(), QWidget* parent = nullptr);

// Upper-cases the text and spreads the letters of a small mono label.
void styleEyebrow(QLabel* label, qreal spacing = 1.6);

// A rounded surface card (QFrame[card="true"]).
QFrame* makeCard(QWidget* parent = nullptr);

// "24 Oct 2026, 2:30 PM" in local time, or the fallback when invalid.
QString formatDateTime(const QDateTime& time, const QString& fallback = QStringLiteral("Never"));
// "24 Oct 2026", or the fallback when invalid.
QString formatDate(const QDate& date, const QString& fallback = QStringLiteral("-"));
// "1 student" / "3 students"
QString plural(int count, const QString& singular, const QString& pluralForm = QString());

} // namespace edu::ui
