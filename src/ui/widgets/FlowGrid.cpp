#include "ui/widgets/FlowGrid.h"

#include <QGridLayout>
#include <QResizeEvent>

#include <algorithm>

namespace edu::ui {

FlowGrid::FlowGrid(int minCardWidth, int maxColumns, QWidget* parent)
    : QWidget(parent), m_minCardWidth(minCardWidth), m_maxColumns(std::max(1, maxColumns))
{
    m_grid = new QGridLayout(this);
    m_grid->setContentsMargins(0, 0, 0, 0);
    m_grid->setHorizontalSpacing(16);
    m_grid->setVerticalSpacing(16);
}

void FlowGrid::addCard(QWidget* card, int span)
{
    card->setParent(this);
    m_items.append({card, std::max(1, span)});
    m_columns = 0; // force a relayout
    relayout(width() > 0 ? width() : m_minCardWidth * m_maxColumns);
}

void FlowGrid::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    relayout(event->size().width());
}

void FlowGrid::relayout(int width)
{
    const int spacing = m_grid->horizontalSpacing();
    const int fit = std::max(1, (width + spacing) / (m_minCardWidth + spacing));
    const int columns = std::min(fit, m_maxColumns);
    if (columns == m_columns)
        return;
    m_columns = columns;

    for (const Item& item : std::as_const(m_items))
        m_grid->removeWidget(item.widget);
    for (int c = 0; c < m_maxColumns; ++c)
        m_grid->setColumnStretch(c, 0);

    int row = 0;
    int column = 0;
    for (const Item& item : std::as_const(m_items)) {
        const int span = std::min(item.span, columns);
        if (column + span > columns) {
            ++row;
            column = 0;
        }
        m_grid->addWidget(item.widget, row, column, 1, span,
                         m_alignTop ? Qt::AlignTop : Qt::Alignment());
        column += span;
    }
    for (int c = 0; c < columns; ++c)
        m_grid->setColumnStretch(c, 1);
}

} // namespace edu::ui
