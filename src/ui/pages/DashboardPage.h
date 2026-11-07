#pragma once

#include "services/SystemService.h"

#include <QWidget>

class QGridLayout;
class QLabel;
class QVBoxLayout;

namespace edu::ui {

class StatCard;

// M0 dashboard: welcome banner, live facts from the database, the milestone
// checklist and the rules this edition follows. Role dashboards replace the
// body in M1.
class DashboardPage : public QWidget {
    Q_OBJECT

public:
    explicit DashboardPage(QString connectionName, QWidget* parent = nullptr);

    void refresh();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void reflow(int pageWidth);
    QWidget* buildHero();
    QWidget* buildChecklist();
    QWidget* buildRules();
    QLabel* addRuleRow(QVBoxLayout* column, const QString& iconName, const QString& title,
                       const QString& detail);

    SystemService m_system;
    QLabel* m_greeting = nullptr;
    QLabel* m_institution = nullptr;
    StatCard* m_database = nullptr;
    StatCard* m_attendance = nullptr;
    StatCard* m_credits = nullptr;
    StatCard* m_build = nullptr;
    QLabel* m_ruleAttendance = nullptr;
    QLabel* m_ruleCredits = nullptr;
    QGridLayout* m_statsGrid = nullptr;
    QGridLayout* m_lowerGrid = nullptr;
    QWidget* m_checklist = nullptr;
    QWidget* m_rules = nullptr;
    int m_statColumns = 0;
    bool m_lowerStacked = false;
};

} // namespace edu::ui
