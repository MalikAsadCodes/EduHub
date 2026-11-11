#pragma once

#include <QList>
#include <QWidget>

class QGridLayout;

namespace edu::ui {

// Lays its cards out in as many equal columns as fit (up to maxColumns),
// re-flowing when the window is resized. Used for dashboard cards.
class FlowGrid : public QWidget {
    Q_OBJECT

public:
    FlowGrid(int minCardWidth, int maxColumns, QWidget* parent = nullptr);

    void addCard(QWidget* card, int span = 1);
    // true (default): cards keep their own height; false: cards in a row share one height.
    void setAlignTop(bool alignTop) { m_alignTop = alignTop; }
    int columnCount() const { return m_columns; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void relayout(int width);

    struct Item {
        QWidget* widget;
        int span;
    };

    QGridLayout* m_grid = nullptr;
    QList<Item> m_items;
    int m_minCardWidth;
    int m_maxColumns;
    int m_columns = 0;
    bool m_alignTop = true;
};

} // namespace edu::ui
