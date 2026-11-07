#pragma once

#include "ui/theme/Theme.h"

#include <QFrame>

class QLabel;

namespace edu::ui {

// Dashboard tile: small label and icon badge, a big value, and a caption.
class StatCard : public QFrame {
    Q_OBJECT

public:
    StatCard(const QString& label, const QString& iconName, ColorRole tint,
             QWidget* parent = nullptr);

    void setValue(const QString& value, const QString& caption);
    QString value() const;

private:
    QLabel* m_value = nullptr;
    QLabel* m_caption = nullptr;
};

} // namespace edu::ui
