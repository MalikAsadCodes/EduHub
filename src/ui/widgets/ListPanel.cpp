#include "ui/widgets/ListPanel.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/Drawer.h"
#include "ui/widgets/UiHelpers.h"

#include <QAction>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace edu::ui {

ListPanel::ListPanel(QWidget* parent) : QWidget(parent)
{
    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(14);

    m_toolbar = new QHBoxLayout;
    m_toolbar->setSpacing(10);
    m_search = new QLineEdit(this);
    m_search->setObjectName(QStringLiteral("ListSearch"));
    m_search->setPlaceholderText(QStringLiteral("Search this list"));
    m_search->setClearButtonEnabled(true);
    m_search->addAction(Icons::themed(QStringLiteral("search"), ColorRole::TextMuted),
                        QLineEdit::LeadingPosition);
    m_search->setMinimumWidth(220);
    m_search->setMaximumWidth(320);
    m_toolbar->addWidget(m_search);
    m_toolbar->addStretch(1);
    column->addLayout(m_toolbar);

    auto* body = new QHBoxLayout;
    body->setSpacing(16);
    auto* card = makeCard(this);
    card->setObjectName(QStringLiteral("TableCard"));
    auto* cardColumn = new QVBoxLayout(card);
    cardColumn->setContentsMargins(1, 1, 1, 1);
    cardColumn->setSpacing(0);
    m_table = new RecordTable(card);
    cardColumn->addWidget(m_table, 1);
    m_footer = makeLabel(QString(), QStringLiteral("tableFooter"), card);
    cardColumn->addWidget(m_footer);
    body->addWidget(card, 1);

    m_drawer = new Drawer(this);
    body->addWidget(m_drawer);
    column->addLayout(body, 1);

    connect(m_search, &QLineEdit::textChanged, m_table, &RecordTable::setSearchText);
    connect(m_drawer, &Drawer::opened, m_table, [this] { m_table->setCompact(true); });
    connect(m_drawer, &Drawer::closed, m_table, [this] { m_table->setCompact(false); });
}

QComboBox* ListPanel::addFilter(const QString& objectName, int minimumWidth)
{
    auto* combo = new QComboBox(this);
    combo->setObjectName(objectName);
    combo->setMinimumWidth(minimumWidth);
    m_toolbar->insertWidget(m_filterInsertIndex++, combo);
    return combo;
}

QPushButton* ListPanel::addToolbarButton(const QString& text, const QString& iconName,
                                         const QString& variant)
{
    auto* button = new QPushButton(text, this);
    button->setProperty("variant", variant);
    button->setCursor(Qt::PointingHandCursor);
    if (!iconName.isEmpty()) {
        button->setIcon(Icons::themed(iconName, variant == QLatin1String("primary")
                                                    ? ColorRole::PrimaryText
                                                    : ColorRole::Text));
        button->setIconSize(QSize(16, 16));
    }
    m_toolbar->addWidget(button);
    return button;
}

void ListPanel::addToolbarWidget(QWidget* widget)
{
    widget->setParent(this);
    m_toolbar->addWidget(widget);
}

void ListPanel::setFooterText(const QString& text)
{
    m_footer->setText(text);
}

} // namespace edu::ui
