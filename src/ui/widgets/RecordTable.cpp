#include "ui/widgets/RecordTable.h"

#include <QHeaderView>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>

namespace edu::ui {

namespace {

ColorRole roleFromName(const QString& name, ColorRole fallback)
{
    static const QHash<QString, ColorRole> names = {
        {QStringLiteral("success"), ColorRole::Success},
        {QStringLiteral("warning"), ColorRole::Warning},
        {QStringLiteral("danger"), ColorRole::Danger},
        {QStringLiteral("info"), ColorRole::Info},
        {QStringLiteral("muted"), ColorRole::TextMuted},
        {QStringLiteral("primary"), ColorRole::Primary},
        {QStringLiteral("accent"), ColorRole::AccentText},
        {QStringLiteral("admin"), ColorRole::RoleAdmin},
        {QStringLiteral("teacher"), ColorRole::RoleTeacher},
        {QStringLiteral("student"), ColorRole::RoleStudent},
    };
    return names.value(name, fallback);
}

} // namespace

// ---------------------------------------------------------------- RecordModel

RecordModel::RecordModel(QObject* parent) : QAbstractTableModel(parent) {}

void RecordModel::setColumns(const QList<TableColumn>& columns)
{
    beginResetModel();
    m_columns = columns;
    endResetModel();
}

void RecordModel::setRecords(const QList<Record>& records)
{
    beginResetModel();
    m_records = records;
    endResetModel();
}

int RecordModel::rowOfId(int id) const
{
    for (qsizetype i = 0; i < m_records.size(); ++i) {
        if (m_records.at(i).value(QStringLiteral("id")).toInt() == id)
            return static_cast<int>(i);
    }
    return -1;
}

int RecordModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_records.size());
}

int RecordModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_columns.size());
}

QVariant RecordModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_records.size() || index.column() >= m_columns.size())
        return {};
    const Record& record = m_records.at(index.row());
    const TableColumn& column = m_columns.at(index.column());
    const QVariant value = record.value(column.key);

    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return value.toString();
    case Qt::TextAlignmentRole:
        return column.kind == TableColumn::Kind::Number
                   ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
                   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    case RecordIdRole:
        return record.value(QStringLiteral("id"));
    case SortRole: {
        const QString sortKey = column.key + QStringLiteral("#sort");
        if (record.contains(sortKey))
            return record.value(sortKey);
        if (column.kind == TableColumn::Kind::Number)
            return value.toDouble();
        return value.toString().toLower();
    }
    case PillColorRole: {
        if (column.kind != TableColumn::Kind::Pill)
            return {};
        const QString override = record.value(column.key + QStringLiteral("#pill")).toString();
        const ColorRole fallback = column.pillColors.value(value.toString(), ColorRole::TextMuted);
        return static_cast<int>(override.isEmpty() ? fallback : roleFromName(override, fallback));
    }
    default:
        return {};
    }
}

QVariant RecordModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || section < 0 || section >= m_columns.size())
        return {};
    if (role == Qt::DisplayRole)
        return m_columns.at(section).title;
    if (role == Qt::TextAlignmentRole)
        return m_columns.at(section).kind == TableColumn::Kind::Number
                   ? QVariant(Qt::AlignRight | Qt::AlignVCenter)
                   : QVariant(Qt::AlignLeft | Qt::AlignVCenter);
    return {};
}

// --------------------------------------------------------------- PillDelegate

void PillDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const
{
    const QVariant colorValue = index.data(RecordModel::PillColorRole);
    const QString text = index.data(Qt::DisplayRole).toString();
    if (!colorValue.isValid() || text.isEmpty()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // Selection background first, without the text.
    QStyleOptionViewItem background = option;
    initStyleOption(&background, index);
    background.text.clear();
    QStyle* style = background.widget ? background.widget->style() : nullptr;
    if (style)
        style->drawControl(QStyle::CE_ItemViewItem, &background, painter, background.widget);

    const QColor color = Theme::instance().color(static_cast<ColorRole>(colorValue.toInt()));
    QColor fill = color;
    fill.setAlpha(Theme::instance().mode() == ThemeMode::Dark ? 52 : 30);

    QFont font = option.font;
    font.setPointSizeF(font.pointSizeF() * 0.9);
    font.setWeight(QFont::DemiBold);
    const QFontMetrics metrics(font);
    const int textWidth = metrics.horizontalAdvance(text);
    const int pillHeight = metrics.height() + 6;
    const int pillWidth = textWidth + 20;
    const QRect cell = option.rect.adjusted(10, 0, -6, 0);
    const QRectF pill(cell.left(), cell.center().y() - pillHeight / 2.0 + 1,
                      std::min(pillWidth, cell.width()), pillHeight);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(pill, pillHeight / 2.0, pillHeight / 2.0);
    painter->fillPath(path, fill);
    painter->setFont(font);
    painter->setPen(color);
    painter->drawText(pill, Qt::AlignCenter, metrics.elidedText(text, Qt::ElideRight,
                                                                int(pill.width()) - 12));
    painter->restore();
}

QSize PillDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setWidth(size.width() + 28);
    return size;
}

// ---------------------------------------------------------------- RecordTable

RecordTable::RecordTable(QWidget* parent) : QTableView(parent)
{
    setObjectName(QStringLiteral("RecordTable"));
    m_model = new RecordModel(this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setSortRole(RecordModel::SortRole);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1);
    setModel(m_proxy);

    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setShowGrid(false);
    setWordWrap(false);
    setSortingEnabled(true);
    setAlternatingRowColors(false);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    verticalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(42);
    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    horizontalHeader()->setMinimumSectionSize(60);
    sortByColumn(0, Qt::AscendingOrder);

    connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            [this](const QModelIndex& current) { emit selectionChangedTo(recordAt(current)); });
    connect(this, &QTableView::activated, this, [this](const QModelIndex& index) {
        const Record record = recordAt(index);
        if (!record.isEmpty())
            emit recordActivated(record);
    });
    connect(this, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        const QModelIndex index = indexAt(pos);
        if (!index.isValid())
            return;
        selectRow(index.row());
        emit contextMenuForRecord(recordAt(index), viewport()->mapToGlobal(pos));
    });
    connect(m_proxy, &QAbstractItemModel::modelReset, viewport(), qOverload<>(&QWidget::update));
}

void RecordTable::setColumns(const QList<TableColumn>& columns)
{
    m_model->setColumns(columns);
    QHeaderView* header = horizontalHeader();
    for (int i = 0; i < columns.size(); ++i) {
        const TableColumn& column = columns.at(i);
        if (column.kind == TableColumn::Kind::Pill)
            setItemDelegateForColumn(i, new PillDelegate(this));
        if (column.stretch) {
            header->setSectionResizeMode(i, QHeaderView::Stretch);
        } else if (column.width > 0) {
            header->setSectionResizeMode(i, QHeaderView::Interactive);
            header->resizeSection(i, column.width);
        } else {
            header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
        }
    }
}

void RecordTable::setCompact(bool compact)
{
    const QList<TableColumn>& columns = m_model->columns();
    for (int i = 0; i < columns.size(); ++i) {
        if (columns.at(i).optional)
            setColumnHidden(i, compact);
    }
}

void RecordTable::setRecords(const QList<Record>& records)
{
    const int keepId = selectedId();
    m_model->setRecords(records);
    if (keepId > 0 && selectId(keepId))
        return;
    emit selectionChangedTo(Record());
}

void RecordTable::setSearchText(const QString& text)
{
    const int keepId = selectedId();
    m_proxy->setFilterRegularExpression(QRegularExpression(
        QRegularExpression::escape(text.trimmed()), QRegularExpression::CaseInsensitiveOption));
    if (keepId > 0 && !selectId(keepId))
        emit selectionChangedTo(Record());
    viewport()->update();
}

void RecordTable::setEmptyText(const QString& title, const QString& hint)
{
    m_emptyTitle = title;
    m_emptyHint = hint;
    viewport()->update();
}

Record RecordTable::recordAt(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid())
        return {};
    const QModelIndex source = m_proxy->mapToSource(proxyIndex);
    if (!source.isValid())
        return {};
    return m_model->record(source.row());
}

bool RecordTable::hasSelection() const
{
    return selectionModel()->currentIndex().isValid()
           && selectionModel()->isRowSelected(selectionModel()->currentIndex().row(), QModelIndex());
}

Record RecordTable::selectedRecord() const
{
    return hasSelection() ? recordAt(selectionModel()->currentIndex()) : Record();
}

int RecordTable::selectedId() const
{
    return selectedRecord().value(QStringLiteral("id")).toInt();
}

bool RecordTable::selectId(int id)
{
    const int sourceRow = m_model->rowOfId(id);
    if (sourceRow < 0)
        return false;
    const QModelIndex proxyIndex = m_proxy->mapFromSource(m_model->index(sourceRow, 0));
    if (!proxyIndex.isValid())
        return false;
    setCurrentIndex(proxyIndex);
    selectRow(proxyIndex.row());
    scrollTo(proxyIndex);
    emit selectionChangedTo(recordAt(proxyIndex));
    return true;
}

void RecordTable::clearCurrent()
{
    const bool had = hasSelection();
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(QModelIndex(), QItemSelectionModel::NoUpdate);
    if (had)
        emit selectionChangedTo(Record());
}

int RecordTable::visibleRowCount() const
{
    return m_proxy->rowCount();
}

Record RecordTable::recordAtRow(int visibleRow) const
{
    return recordAt(m_proxy->index(visibleRow, 0));
}

void RecordTable::paintEvent(QPaintEvent* event)
{
    QTableView::paintEvent(event);
    if (m_proxy->rowCount() > 0 || m_emptyTitle.isEmpty())
        return;

    QPainter painter(viewport());
    const QRect area = viewport()->rect().adjusted(24, 24, -24, -24);
    QFont titleFont = font();
    titleFont.setWeight(QFont::DemiBold);
    titleFont.setPointSizeF(titleFont.pointSizeF() * 1.1);
    painter.setFont(titleFont);
    painter.setPen(Theme::instance().color(ColorRole::Text));
    const bool filtered = !m_proxy->filterRegularExpression().pattern().isEmpty()
                          && m_model->rowCount() > 0;
    const QString title = filtered ? QStringLiteral("No matches") : m_emptyTitle;
    const QString hint =
        filtered ? QStringLiteral("Try a different search.") : m_emptyHint;
    const QRect titleRect(area.left(), area.top() + area.height() / 3 - 24, area.width(), 28);
    painter.drawText(titleRect, Qt::AlignCenter, title);
    if (!hint.isEmpty()) {
        painter.setFont(font());
        painter.setPen(Theme::instance().color(ColorRole::TextMuted));
        painter.drawText(QRect(area.left(), titleRect.bottom() + 4, area.width(), 60),
                         Qt::AlignHCenter | Qt::AlignTop | Qt::TextWordWrap, hint);
    }
}

} // namespace edu::ui
