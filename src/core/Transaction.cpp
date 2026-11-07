#include "core/Transaction.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include <utility>

namespace edu {

Transaction::Transaction(QSqlDatabase db, Mode mode) : m_db(std::move(db))
{
    QSqlQuery query(m_db);
    const QString begin =
        (mode == Mode::Immediate) ? QStringLiteral("BEGIN IMMEDIATE") : QStringLiteral("BEGIN");
    m_active = query.exec(begin);
    if (!m_active)
        qWarning().noquote() << "Transaction: BEGIN failed:" << query.lastError().text();
}

Transaction::~Transaction()
{
    if (m_active)
        rollback();
}

bool Transaction::commit()
{
    if (!m_active)
        return false;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("COMMIT"))) {
        qWarning().noquote() << "Transaction: COMMIT failed:" << query.lastError().text();
        rollback();
        return false;
    }
    m_active = false;
    return true;
}

void Transaction::rollback()
{
    if (!m_active)
        return;
    QSqlQuery query(m_db);
    if (!query.exec(QStringLiteral("ROLLBACK")))
        qWarning().noquote() << "Transaction: ROLLBACK failed:" << query.lastError().text();
    m_active = false;
}

} // namespace edu
