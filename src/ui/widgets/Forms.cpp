#include "ui/widgets/Forms.h"

#include "ui/widgets/UiHelpers.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace edu::ui {

FormStack::FormStack(QWidget* parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("FormStack"));
    m_column = new QVBoxLayout(this);
    m_column->setContentsMargins(20, 16, 20, 20);
    m_column->setSpacing(4);
}

QWidget* FormStack::addField(const QString& label, QWidget* field, const QString& hint)
{
    if (m_column->count() > 0)
        m_column->addSpacing(10);
    auto* title = makeLabel(label, QStringLiteral("fieldLabel"), this);
    title->setBuddy(field);
    m_column->addWidget(title);
    m_column->addSpacing(2);
    field->setParent(this);
    m_column->addWidget(field);
    if (!hint.isEmpty()) {
        auto* hintLabel = makeLabel(hint, QStringLiteral("hint"), this);
        hintLabel->setWordWrap(true);
        m_column->addWidget(hintLabel);
    }
    return field;
}

void FormStack::addRow(const QString& leftLabel, QWidget* left, const QString& rightLabel,
                       QWidget* right)
{
    if (m_column->count() > 0)
        m_column->addSpacing(10);
    auto* grid = new QGridLayout;
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(4);
    auto* l = makeLabel(leftLabel, QStringLiteral("fieldLabel"), this);
    auto* r = makeLabel(rightLabel, QStringLiteral("fieldLabel"), this);
    l->setBuddy(left);
    r->setBuddy(right);
    left->setParent(this);
    right->setParent(this);
    grid->addWidget(l, 0, 0);
    grid->addWidget(r, 0, 1);
    grid->addWidget(left, 1, 0);
    grid->addWidget(right, 1, 1);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    m_column->addLayout(grid);
}

QLabel* FormStack::addDetail(const QString& label, const QString& value)
{
    auto* row = new QHBoxLayout;
    row->setContentsMargins(0, 5, 0, 5);
    row->setSpacing(12);
    auto* name = makeLabel(label, QStringLiteral("detailLabel"), this);
    name->setFixedWidth(130);
    name->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    auto* text = makeLabel(value, QStringLiteral("detailValue"), this);
    text->setWordWrap(true);
    text->setTextInteractionFlags(Qt::TextSelectableByMouse);
    row->addWidget(name, 0, Qt::AlignTop);
    row->addWidget(text, 1);
    m_column->addLayout(row);
    return text;
}

void FormStack::addSection(const QString& title, const QString& subtitle)
{
    if (m_column->count() > 0) {
        m_column->addSpacing(14);
        auto* line = new QFrame(this);
        line->setObjectName(QStringLiteral("FormDivider"));
        line->setFixedHeight(1);
        m_column->addWidget(line);
        m_column->addSpacing(10);
    }
    m_column->addWidget(makeLabel(title, QStringLiteral("formSection"), this));
    if (!subtitle.isEmpty()) {
        auto* sub = makeLabel(subtitle, QStringLiteral("hint"), this);
        sub->setWordWrap(true);
        m_column->addWidget(sub);
    }
    m_column->addSpacing(2);
}

void FormStack::addWidget(QWidget* widget)
{
    widget->setParent(this);
    m_column->addWidget(widget);
}

void FormStack::addSpacing(int pixels)
{
    m_column->addSpacing(pixels);
}

void FormStack::addStretch()
{
    m_column->addStretch(1);
}

} // namespace edu::ui
