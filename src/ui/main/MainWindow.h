#pragma once

#include <QHash>
#include <QString>
#include <QWidget>

class QStackedWidget;

namespace edu::ui {

class DashboardPage;
class HeaderBar;
class NavRail;

// The app shell: navigation rail on the left, header and page stack on the
// right. Every milestone adds pages here; the shell itself stays the same.
class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QString connectionName, QWidget* parent = nullptr);

    void setSignedInUser(const QString& displayName, const QString& roleLabel);
    void showPage(const QString& id);
    QString currentPage() const { return m_current; }

    NavRail* rail() const { return m_rail; }
    HeaderBar* header() const { return m_header; }

signals:
    void signOutRequested();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    struct Page {
        QString title;
        QString section;
        QWidget* widget = nullptr;
    };

    void addPage(const QString& id, const QString& navText, const QString& title,
                 const QString& section, const QString& iconName, QWidget* widget);
    void toggleTheme();
    void restoreWindowGeometry();

    QString m_connection;
    NavRail* m_rail = nullptr;
    HeaderBar* m_header = nullptr;
    QStackedWidget* m_stack = nullptr;
    DashboardPage* m_dashboard = nullptr;
    QHash<QString, Page> m_pages;
    QString m_current;
};

} // namespace edu::ui
