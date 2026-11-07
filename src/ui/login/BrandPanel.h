#pragma once

#include <QWidget>

namespace edu::ui {

// Left half of the sign-in window: navy gradient with a faint gold pattern,
// the Edu Hub mark, the institution's name and what the app covers.
// Uses fixed brand colours, so it looks the same in light and dark themes.
class BrandPanel : public QWidget {
    Q_OBJECT

public:
    explicit BrandPanel(const QString& institutionName, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;
};

} // namespace edu::ui
