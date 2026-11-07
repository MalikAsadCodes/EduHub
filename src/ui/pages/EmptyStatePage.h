#pragma once

#include <QWidget>

namespace edu::ui {

// Placeholder for a section that a later milestone fills in.
class EmptyStatePage : public QWidget {
    Q_OBJECT

public:
    EmptyStatePage(const QString& iconName, const QString& title, const QString& message,
                   const QString& milestone, QWidget* parent = nullptr);
};

} // namespace edu::ui
