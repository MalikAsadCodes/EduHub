#pragma once

#include <QFrame>

class QLabel;
class QLineEdit;
class QToolButton;

namespace edu::ui {

// Top bar of the main window: breadcrumb and page title on the left; current
// term chip, global search (Ctrl+K), theme switch and avatar menu on the right.
class HeaderBar : public QFrame {
    Q_OBJECT

public:
    explicit HeaderBar(QWidget* parent = nullptr);

    void setPage(const QString& title, const QString& section);
    void setTermText(const QString& text);
    // roleKey ("admin", "teacher", "student") picks the avatar colour.
    void setUser(const QString& displayName, const QString& roleLabel,
                 const QString& roleKey = QStringLiteral("admin"));

    QLineEdit* searchField() const { return m_search; }
    QString title() const;

signals:
    void searchSubmitted(const QString& text);
    void themeToggleRequested();
    void signOutRequested();
    void changePasswordRequested();

private:
    void updateThemeButton();

    QLabel* m_breadcrumb = nullptr;
    QLabel* m_title = nullptr;
    QLabel* m_term = nullptr;
    QLineEdit* m_search = nullptr;
    QToolButton* m_themeButton = nullptr;
    QToolButton* m_avatar = nullptr;
    QLabel* m_menuName = nullptr;
    QLabel* m_menuRole = nullptr;
};

} // namespace edu::ui
