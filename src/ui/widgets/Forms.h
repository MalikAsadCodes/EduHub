#pragma once

#include <QWidget>

class QLabel;
class QVBoxLayout;

namespace edu::ui {

// A vertical form: label above each field, optional hint below.
// Used inside drawers and dialogs so every form looks the same.
class FormStack : public QWidget {
    Q_OBJECT

public:
    explicit FormStack(QWidget* parent = nullptr);

    // Adds "label / field / hint". Returns the field for chaining.
    QWidget* addField(const QString& label, QWidget* field, const QString& hint = QString());

    // Two fields side by side.
    void addRow(const QString& leftLabel, QWidget* left, const QString& rightLabel,
                QWidget* right);

    // A read-only "label: value" line. Returns the value label.
    QLabel* addDetail(const QString& label, const QString& value);

    void addSection(const QString& title, const QString& subtitle = QString());
    void addWidget(QWidget* widget);
    void addSpacing(int pixels);
    void addStretch();

    QVBoxLayout* layout() const { return m_column; }

private:
    QVBoxLayout* m_column = nullptr;
};

} // namespace edu::ui
