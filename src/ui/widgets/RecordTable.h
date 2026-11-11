#pragma once

#include "ui/theme/Theme.h"

#include <QAbstractTableModel>
#include <QHash>
#include <QList>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVariantMap>

namespace edu::ui {

// One row of a table: column key -> value. Optional extras:
//   "<key>#sort"  value used for sorting instead of the shown text
//   "<key>#pill"  colour role name for a Pill column (overrides the column map)
//   "id"          record id (used to keep the selection across reloads)
using Record = QVariantMap;

struct TableColumn {
    enum class Kind { Text, Pill, Number };

    QString key;
    QString title;
    Kind kind = Kind::Text;
    int width = 0;        // initial width in pixels; 0 = size to contents
    bool stretch = false; // takes the remaining width
    QHash<QString, ColorRole> pillColors; // value -> colour, for Pill columns
    bool optional = false; // hidden while the side drawer is open

    TableColumn& hideWhenCompact()
    {
        optional = true;
        return *this;
    }

    static TableColumn text(const QString& key, const QString& title, int width = 0)
    {
        return {key, title, Kind::Text, width, false, {}, false};
    }
    static TableColumn stretchText(const QString& key, const QString& title)
    {
        return {key, title, Kind::Text, 0, true, {}, false};
    }
    static TableColumn number(const QString& key, const QString& title, int width = 0)
    {
        return {key, title, Kind::Number, width, false, {}, false};
    }
    static TableColumn pill(const QString& key, const QString& title,
                            QHash<QString, ColorRole> colors, int width = 0)
    {
        return {key, title, Kind::Pill, width, false, std::move(colors), false};
    }
};

class RecordModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Roles { RecordIdRole = Qt::UserRole + 1, SortRole, PillColorRole };

    explicit RecordModel(QObject* parent = nullptr);

    void setColumns(const QList<TableColumn>& columns);
    const QList<TableColumn>& columns() const { return m_columns; }

    void setRecords(const QList<Record>& records);
    const QList<Record>& records() const { return m_records; }
    const Record& record(int row) const { return m_records.at(row); }
    int rowOfId(int id) const;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

private:
    QList<TableColumn> m_columns;
    QList<Record> m_records;
};

// Draws a rounded status pill ("Active", "Locked"...).
class PillDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

// Sortable, searchable table of records with an empty-state message.
class RecordTable : public QTableView {
    Q_OBJECT

public:
    explicit RecordTable(QWidget* parent = nullptr);

    void setColumns(const QList<TableColumn>& columns);
    // Replaces the rows, keeping the selected record (by "id") when possible.
    void setRecords(const QList<Record>& records);

    void setSearchText(const QString& text);
    // Compact mode hides the columns marked optional (used while the drawer is open).
    void setCompact(bool compact);
    void setEmptyText(const QString& title, const QString& hint = QString());

    bool hasSelection() const;
    Record selectedRecord() const;
    int selectedId() const;
    bool selectId(int id);
    void clearCurrent(); // deselects; emits selectionChangedTo(empty)
    int visibleRowCount() const;
    Record recordAtRow(int visibleRow) const;

    RecordModel* recordModel() const { return m_model; }

signals:
    void selectionChangedTo(const edu::ui::Record& record); // empty when nothing is selected
    void recordActivated(const edu::ui::Record& record);
    void contextMenuForRecord(const edu::ui::Record& record, const QPoint& globalPos);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Record recordAt(const QModelIndex& proxyIndex) const;

    RecordModel* m_model = nullptr;
    QSortFilterProxyModel* m_proxy = nullptr;
    QString m_emptyTitle;
    QString m_emptyHint;
};

} // namespace edu::ui
