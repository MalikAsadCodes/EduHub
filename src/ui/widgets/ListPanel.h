#pragma once

#include "ui/widgets/RecordTable.h"

#include <QWidget>

class QComboBox;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QVBoxLayout;

namespace edu::ui {

class Drawer;

// The standard list layout: a toolbar (search, filters, actions) above a
// table card, with a drawer that slides in on the right for the selected row.
class ListPanel : public QWidget {
    Q_OBJECT

public:
    explicit ListPanel(QWidget* parent = nullptr);

    RecordTable* table() const { return m_table; }
    Drawer* drawer() const { return m_drawer; }
    QLineEdit* searchBox() const { return m_search; }

    QComboBox* addFilter(const QString& objectName, int minimumWidth = 150);
    QPushButton* addToolbarButton(const QString& text, const QString& iconName,
                                  const QString& variant = QStringLiteral("primary"));
    void addToolbarWidget(QWidget* widget);

    // Text under the table, e.g. "12 students".
    void setFooterText(const QString& text);

private:
    QHBoxLayout* m_toolbar = nullptr;
    QLineEdit* m_search = nullptr;
    RecordTable* m_table = nullptr;
    Drawer* m_drawer = nullptr;
    QLabel* m_footer = nullptr;
    int m_filterInsertIndex = 1;
};

} // namespace edu::ui
