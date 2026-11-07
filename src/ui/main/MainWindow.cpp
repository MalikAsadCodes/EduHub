#include "ui/main/MainWindow.h"

#include "ui/UiSettings.h"
#include "ui/pages/DashboardPage.h"
#include "ui/pages/EmptyStatePage.h"
#include "ui/theme/Icons.h"
#include "ui/theme/Theme.h"
#include "ui/widgets/HeaderBar.h"
#include "ui/widgets/NavRail.h"
#include "ui/widgets/Toast.h"

#include <QCloseEvent>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QScreen>
#include <QShortcut>
#include <QStackedWidget>
#include <QVBoxLayout>

#include <utility>

namespace edu::ui {

namespace {
// Fits a 1080p laptop at 150% scaling (1280 x 720 logical, minus the taskbar).
constexpr int kMinWidth = 960;
constexpr int kMinHeight = 600;
} // namespace

MainWindow::MainWindow(QString connectionName, QWidget* parent)
    : QWidget(parent), m_connection(std::move(connectionName))
{
    setObjectName(QStringLiteral("AppRoot"));
    setAttribute(Qt::WA_StyledBackground);
    setWindowTitle(QStringLiteral("Edu Hub"));
    setWindowIcon(Icons::fixed(QStringLiteral("app")));
    setMinimumSize(kMinWidth, kMinHeight);

    auto* shell = new QHBoxLayout(this);
    shell->setContentsMargins(0, 0, 0, 0);
    shell->setSpacing(0);

    m_rail = new NavRail(this);
    shell->addWidget(m_rail);

    auto* right = new QVBoxLayout;
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(0);
    m_header = new HeaderBar(this);
    right->addWidget(m_header);
    m_stack = new QStackedWidget(this);
    m_stack->setObjectName(QStringLiteral("PageStack"));
    right->addWidget(m_stack, 1);
    shell->addLayout(right, 1);

    // Pages. Later milestones swap the placeholders for real screens.
    m_rail->addSection(QStringLiteral("Overview"));
    m_dashboard = new DashboardPage(m_connection, this);
    addPage(QStringLiteral("dashboard"), QStringLiteral("Dashboard"), QStringLiteral("Dashboard"),
            QStringLiteral("Overview"), QStringLiteral("dashboard"), m_dashboard);

    m_rail->addSection(QStringLiteral("Academics"));
    addPage(QStringLiteral("academics"), QStringLiteral("Academics"),
            QStringLiteral("Academic structure"), QStringLiteral("Academics"),
            QStringLiteral("academics"),
            new EmptyStatePage(QStringLiteral("academics"),
                               QStringLiteral("Departments, programs, courses and terms"),
                               QStringLiteral("Set up departments and degree programs, add courses "
                                              "with their prerequisites, and map each course to "
                                              "a semester of the program."),
                               QStringLiteral("M2"), this));
    addPage(QStringLiteral("people"), QStringLiteral("People"), QStringLiteral("People"),
            QStringLiteral("Academics"), QStringLiteral("people"),
            new EmptyStatePage(QStringLiteral("people"), QStringLiteral("Students and teachers"),
                               QStringLiteral("Profiles linked to sign-in accounts, with search, "
                                              "filters and student status."),
                               QStringLiteral("M3"), this));
    addPage(QStringLiteral("enrollment"), QStringLiteral("Enrollment"),
            QStringLiteral("Enrollment"), QStringLiteral("Academics"), QStringLiteral("enrollment"),
            new EmptyStatePage(QStringLiteral("enrollment"),
                               QStringLiteral("Course sections and registration"),
                               QStringLiteral("Open sections with seat limits. Students register "
                                              "within the credit-hour cap and prerequisites are "
                                              "checked automatically."),
                               QStringLiteral("M4"), this));

    m_rail->addSection(QStringLiteral("Teaching"));
    addPage(QStringLiteral("attendance"), QStringLiteral("Attendance"),
            QStringLiteral("Attendance"), QStringLiteral("Teaching"), QStringLiteral("attendance"),
            new EmptyStatePage(QStringLiteral("attendance"), QStringLiteral("Class attendance"),
                               QStringLiteral("Teachers mark each class from the keyboard with "
                                              "P, A and L. Students below the limit are "
                                              "flagged before the final exam."),
                               QStringLiteral("M5"), this));
    addPage(QStringLiteral("results"), QStringLiteral("Results"), QStringLiteral("Results"),
            QStringLiteral("Teaching"), QStringLiteral("results"),
            new EmptyStatePage(QStringLiteral("results"), QStringLiteral("Marks and grades"),
                               QStringLiteral("Weighted assessments, marks entry, letter grades, "
                                              "GPA and CGPA. Printable transcripts follow in M7."),
                               QStringLiteral("M6"), this));

    m_rail->addSection(QStringLiteral("System"));
    addPage(QStringLiteral("users"), QStringLiteral("Users"), QStringLiteral("Users"),
            QStringLiteral("System"), QStringLiteral("users"),
            new EmptyStatePage(QStringLiteral("users"), QStringLiteral("Accounts and roles"),
                               QStringLiteral("Create admin, teacher and student accounts, reset "
                                              "passwords and deactivate users."),
                               QStringLiteral("M1"), this));
    addPage(QStringLiteral("settings"), QStringLiteral("Settings"), QStringLiteral("Settings"),
            QStringLiteral("System"), QStringLiteral("settings"),
            new EmptyStatePage(QStringLiteral("settings"), QStringLiteral("Institution settings"),
                               QStringLiteral("Institution name and logo, attendance limits, the "
                                              "credit-hour cap and the grading scale."),
                               QStringLiteral("M2"), this));

    connect(m_rail, &NavRail::currentChanged, this, &MainWindow::showPage);
    connect(m_rail, &NavRail::signOutRequested, this, &MainWindow::signOutRequested);
    connect(m_rail, &NavRail::collapsedChanged, this,
            [](bool collapsed) { UiSettings::setRailCollapsed(collapsed); });
    connect(m_header, &HeaderBar::signOutRequested, this, &MainWindow::signOutRequested);
    connect(m_header, &HeaderBar::themeToggleRequested, this, &MainWindow::toggleTheme);
    connect(m_header, &HeaderBar::searchSubmitted, this, [this](const QString& text) {
        if (!text.isEmpty())
            Toast::notify(this, QStringLiteral("Search arrives with student and course records "
                                               "in M3."));
    });

    auto* toggleRail = new QShortcut(QKeySequence(QStringLiteral("Ctrl+B")), this);
    connect(toggleRail, &QShortcut::activated, this,
            [this] { m_rail->setCollapsed(!m_rail->isCollapsed()); });

    m_header->setTermText(QStringLiteral("No active term"));
    m_rail->setCollapsed(UiSettings::railCollapsed(), false);
    showPage(QStringLiteral("dashboard"));
    restoreWindowGeometry();

    // Start with focus on the page, not on a rail button or the search box.
    m_stack->setFocusPolicy(Qt::ClickFocus);
    m_stack->setFocus();
}

void MainWindow::addPage(const QString& id, const QString& navText, const QString& title,
                         const QString& section, const QString& iconName, QWidget* widget)
{
    widget->setObjectName(QStringLiteral("page_") + id);
    m_stack->addWidget(widget);
    m_pages.insert(id, Page{title, section, widget});
    m_rail->addItem(id, navText, iconName);
}

void MainWindow::showPage(const QString& id)
{
    const auto it = m_pages.constFind(id);
    if (it == m_pages.constEnd() || id == m_current)
        return;
    m_current = id;
    m_stack->setCurrentWidget(it->widget);
    m_header->setPage(it->title, it->section);
    m_rail->setCurrent(id);
    if (it->widget == m_dashboard)
        m_dashboard->refresh();
}

void MainWindow::setSignedInUser(const QString& displayName, const QString& roleLabel)
{
    m_header->setUser(displayName, roleLabel);
}

void MainWindow::toggleTheme()
{
    Theme::instance().toggle();
    const ThemeMode mode = Theme::instance().mode();
    UiSettings::setThemeMode(mode);
    Toast::notify(this,
                  mode == ThemeMode::Dark ? QStringLiteral("Dark theme on")
                                          : QStringLiteral("Light theme on"),
                  ToastKind::Success, 1600);
}

void MainWindow::restoreWindowGeometry()
{
    if (restoreGeometry(UiSettings::mainWindowGeometry()))
        return;
    QSize size(1240, 780);
    if (const QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        size = size.boundedTo(QSize(available.width() * 92 / 100, available.height() * 90 / 100))
                   .expandedTo(minimumSize());
        resize(size);
        move(available.center() - rect().center());
        return;
    }
    resize(size);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    UiSettings::setMainWindowGeometry(saveGeometry());
    QWidget::closeEvent(event);
}

} // namespace edu::ui
