#include "ui/pages/DashboardPage.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/StatCard.h"
#include "ui/widgets/UiHelpers.h"

#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTime>
#include <QVBoxLayout>

#include <utility>

namespace edu::ui {

namespace {

struct MilestoneRow {
    const char* code;
    const char* title;
    const char* detail;
    bool done;
};

constexpr MilestoneRow kMilestones[] = {
    {"M0", "Database and app shell", "Schema, migrations, themes, navigation", true},
    {"M1", "Sign-in and roles", "Admin, teacher and student accounts", false},
    {"M2", "Academic structure", "Departments, programs, courses, terms", false},
    {"M3", "People", "Student and teacher profiles", false},
    {"M4", "Offerings and enrollment", "Sections, seat limits, prerequisites", false},
    {"M5", "Attendance", "Class sessions and shortage alerts", false},
    {"M6", "Marks and grading", "Weighted marks, GPA and CGPA", false},
    {"M7", "Transcripts", "Printable result cards and transcripts", false},
    {"M8", "Release", "Demo data, packaging, v1.0.0", false},
};

QString greetingForNow()
{
    const int hour = QTime::currentTime().hour();
    if (hour < 12)
        return QStringLiteral("GOOD MORNING");
    if (hour < 17)
        return QStringLiteral("GOOD AFTERNOON");
    return QStringLiteral("GOOD EVENING");
}

} // namespace

DashboardPage::DashboardPage(QString connectionName, QWidget* parent)
    : QWidget(parent), m_system(std::move(connectionName))
{
    setProperty("page", true);
    setAttribute(Qt::WA_StyledBackground);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    outer->addWidget(scroll);

    auto* content = new QWidget(scroll);
    content->setObjectName(QStringLiteral("PageContent"));
    scroll->setWidget(content);

    auto* column = new QVBoxLayout(content);
    column->setContentsMargins(28, 24, 28, 28);
    column->setSpacing(18);

    column->addWidget(buildHero());

    m_statsGrid = new QGridLayout;
    m_statsGrid->setSpacing(16);
    m_database = new StatCard(QStringLiteral("Database"), QStringLiteral("database"),
                              ColorRole::Info, content);
    m_attendance = new StatCard(QStringLiteral("Attendance rule"), QStringLiteral("attendance"),
                                ColorRole::Warning, content);
    m_credits = new StatCard(QStringLiteral("Credit-hour cap"), QStringLiteral("layers"),
                             ColorRole::Primary, content);
    m_build =
        new StatCard(QStringLiteral("Build"), QStringLiteral("gauge"), ColorRole::Success, content);
    column->addLayout(m_statsGrid);

    m_lowerGrid = new QGridLayout;
    m_lowerGrid->setSpacing(16);
    m_checklist = buildChecklist();
    m_rules = buildRules();
    column->addLayout(m_lowerGrid);
    column->addStretch(1);

    reflow(width());
    refresh();
}

void DashboardPage::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    reflow(width());
}

void DashboardPage::reflow(int pageWidth)
{
    // Four stat cards in a row on wide windows, two by two on narrow ones;
    // the roadmap and rules cards stack when there is not room for both.
    const int statColumns = pageWidth >= 900 ? 4 : 2;
    const bool stackLower = pageWidth < 860;
    if (statColumns == m_statColumns && stackLower == m_lowerStacked)
        return;
    m_statColumns = statColumns;
    m_lowerStacked = stackLower;

    const QList<StatCard*> cards = {m_database, m_attendance, m_credits, m_build};
    for (StatCard* card : cards)
        m_statsGrid->removeWidget(card);
    for (int column = 0; column < 4; ++column)
        m_statsGrid->setColumnStretch(column, 0);
    for (qsizetype i = 0; i < cards.size(); ++i) {
        const int index = static_cast<int>(i);
        m_statsGrid->addWidget(cards.at(i), index / statColumns, index % statColumns);
    }
    for (int column = 0; column < statColumns; ++column)
        m_statsGrid->setColumnStretch(column, 1);

    m_lowerGrid->removeWidget(m_checklist);
    m_lowerGrid->removeWidget(m_rules);
    m_lowerGrid->setColumnStretch(0, 0);
    m_lowerGrid->setColumnStretch(1, 0);
    if (stackLower) {
        m_lowerGrid->addWidget(m_checklist, 0, 0);
        m_lowerGrid->addWidget(m_rules, 1, 0);
        m_lowerGrid->setColumnStretch(0, 1);
    } else {
        m_lowerGrid->addWidget(m_checklist, 0, 0);
        m_lowerGrid->addWidget(m_rules, 0, 1, Qt::AlignTop);
        m_lowerGrid->setColumnStretch(0, 3);
        m_lowerGrid->setColumnStretch(1, 2);
    }
}

QWidget* DashboardPage::buildHero()
{
    auto* hero = new QFrame(this);
    hero->setObjectName(QStringLiteral("HeroCard"));

    auto* row = new QHBoxLayout(hero);
    row->setContentsMargins(28, 24, 28, 24);
    row->setSpacing(20);

    auto* text = new QVBoxLayout;
    text->setSpacing(4);
    m_greeting = makeLabel(QString(), QStringLiteral("heroEyebrow"), hero);
    QFont eyebrowFont = m_greeting->font();
    eyebrowFont.setLetterSpacing(QFont::AbsoluteSpacing, 1.4);
    m_greeting->setFont(eyebrowFont);
    text->addWidget(m_greeting);
    text->addWidget(
        makeLabel(QStringLiteral("Welcome to Edu Hub"), QStringLiteral("heroTitle"), hero));
    m_institution = makeLabel(QString(), QStringLiteral("heroText"), hero);
    text->addWidget(m_institution);
    row->addLayout(text, 1);

    row->addWidget(new IconLabel(Icons::fixed(QStringLiteral("app")), 64, hero), 0,
                   Qt::AlignVCenter);
    return hero;
}

QWidget* DashboardPage::buildChecklist()
{
    QFrame* card = makeCard(this);
    auto* column = new QVBoxLayout(card);
    column->setContentsMargins(20, 18, 20, 14);
    column->setSpacing(4);
    column->addWidget(
        makeLabel(QStringLiteral("Build roadmap"), QStringLiteral("sectionTitle"), card));
    column->addWidget(makeLabel(QStringLiteral("What each milestone adds to Edu Hub v1."),
                                QStringLiteral("muted"), card));
    column->addSpacing(8);

    for (const MilestoneRow& milestone : kMilestones) {
        auto* row = new QHBoxLayout;
        row->setSpacing(12);
        row->setContentsMargins(0, 6, 0, 6);
        const QIcon icon = milestone.done
                               ? Icons::themed(QStringLiteral("check-circle"), ColorRole::Success)
                               : Icons::themed(QStringLiteral("circle"), ColorRole::Border);
        row->addWidget(new IconLabel(icon, 20, card), 0, Qt::AlignVCenter);

        auto* text = new QVBoxLayout;
        text->setSpacing(0);
        text->addWidget(
            makeLabel(QString::fromUtf8(milestone.title), QStringLiteral("rowTitle"), card));
        text->addWidget(
            makeLabel(QString::fromUtf8(milestone.detail), QStringLiteral("rowDetail"), card));
        row->addLayout(text, 1);

        QLabel* code = makeLabel(
            QString::fromLatin1(milestone.code),
            milestone.done ? QStringLiteral("pillDone") : QStringLiteral("pillMuted"), card);
        row->addWidget(code, 0, Qt::AlignVCenter);
        column->addLayout(row);
    }
    return card;
}

QLabel* DashboardPage::addRuleRow(QVBoxLayout* column, const QString& iconName,
                                  const QString& title, const QString& detail)
{
    QWidget* parent = column->parentWidget();
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    row->setContentsMargins(0, 6, 0, 6);
    auto* badge = new IconLabel(Icons::themed(iconName, ColorRole::AccentText), 16, parent);
    badge->setBadge(ColorRole::AccentSoft, 7, 8);
    row->addWidget(badge, 0, Qt::AlignTop);

    auto* text = new QVBoxLayout;
    text->setSpacing(1);
    text->addWidget(makeLabel(title, QStringLiteral("rowTitle"), parent));
    QLabel* detailLabel = makeLabel(detail, QStringLiteral("rowDetail"), parent);
    detailLabel->setWordWrap(true);
    text->addWidget(detailLabel);
    row->addLayout(text, 1);
    column->addLayout(row);
    return detailLabel;
}

QWidget* DashboardPage::buildRules()
{
    QFrame* card = makeCard(this);
    auto* column = new QVBoxLayout(card);
    column->setContentsMargins(20, 18, 20, 18);
    column->setSpacing(4);
    column->addWidget(
        makeLabel(QStringLiteral("Rules in this edition"), QStringLiteral("sectionTitle"), card));
    column->addWidget(
        makeLabel(QStringLiteral("Editable later in Settings."), QStringLiteral("muted"), card));
    column->addSpacing(8);

    addRuleRow(column, QStringLiteral("results"), QStringLiteral("Grading"),
               QStringLiteral("HEC-style 4.0 scale with absolute grading"));
    addRuleRow(column, QStringLiteral("enrollment"), QStringLiteral("Enrollment"),
               QStringLiteral("Students self-enroll while registration is open"));
    m_ruleAttendance =
        addRuleRow(column, QStringLiteral("attendance"), QStringLiteral("Attendance"), QString());
    m_ruleCredits =
        addRuleRow(column, QStringLiteral("layers"), QStringLiteral("Credit hours"), QString());
    addRuleRow(column, QStringLiteral("academics"), QStringLiteral("Repeated courses"),
               QStringLiteral("Best attempt counts in CGPA"));
    addRuleRow(column, QStringLiteral("database"), QStringLiteral("Storage"),
               QStringLiteral("One PC, local database, works offline"));
    return card;
}

void DashboardPage::refresh()
{
    const SystemStatus status = m_system.status();

    m_greeting->setText(greetingForNow());
    m_institution->setText(QStringLiteral("%1  \u00B7  Version %2 preview")
                               .arg(status.institutionName, QStringLiteral(EDUHUB_VERSION)));

    const bool current = status.schemaVersion == status.latestSchemaVersion;
    m_database->setValue(
        current ? QStringLiteral("Ready") : QStringLiteral("Needs update"),
        QStringLiteral("%1  \u00B7  schema v%2")
            .arg(QFileInfo(status.databasePath).fileName(), QString::number(status.schemaVersion)));
    m_attendance->setValue(QStringLiteral("%1%").arg(status.attendanceMinPercent),
                           QStringLiteral("Minimum for the final exam"));
    m_credits->setValue(QString::number(status.maxCreditHoursPerTerm),
                        QStringLiteral("Per student, per term"));
    m_build->setValue(QStringLiteral("M0"), QStringLiteral("1 of 9 milestones done"));

    m_ruleAttendance->setText(
        QStringLiteral("Warning below %1%, final blocked below %2% unless exempted")
            .arg(status.attendanceWarnPercent)
            .arg(status.attendanceMinPercent));
    m_ruleCredits->setText(QStringLiteral("Up to %1 per term, checked at enrollment")
                               .arg(status.maxCreditHoursPerTerm));
}

} // namespace edu::ui
