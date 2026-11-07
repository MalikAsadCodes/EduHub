#pragma once

#include <QSqlDatabase>
#include <QtGlobal>

namespace edu {

// RAII transaction. Rolls back automatically unless commit() succeeded.
//
// Immediate mode (the default) takes SQLite's write lock at BEGIN, so a
// "check then insert" sequence (for example the last free seat in a section)
// cannot interleave with another writer.
class Transaction {
public:
    enum class Mode { Deferred, Immediate };

    explicit Transaction(QSqlDatabase db, Mode mode = Mode::Immediate);
    ~Transaction();

    Q_DISABLE_COPY_MOVE(Transaction)

    bool isActive() const { return m_active; }
    bool commit();
    void rollback();

private:
    QSqlDatabase m_db;
    bool m_active = false;
};

} // namespace edu
