#pragma once

#include <QFrame>
#include <QList>

class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QPushButton;
class QScrollArea;
class QToolButton;

namespace edu::ui {

// Panel that slides in on the right of a list page to show or edit the
// selected record. Its actions sit in the header, never in a bottom row,
// so they stay visible on small screens.
class Drawer : public QFrame {
    Q_OBJECT
    Q_PROPERTY(int drawerWidth READ drawerWidth WRITE setDrawerWidth)

public:
    static constexpr int kWidth = 400;

    explicit Drawer(QWidget* parent = nullptr);

    void setTitle(const QString& title, const QString& subtitle = QString());
    QString title() const;

    // Replaces the body. The drawer takes ownership.
    void setContent(QWidget* content);
    QWidget* content() const { return m_content; }

    // Header buttons, laid out right to left before the close button.
    // variant: "primary", "secondary", "danger" or "ghost".
    QPushButton* addAction(const QString& text, const QString& variant = QStringLiteral("secondary"),
                           const QString& iconName = QString());
    void clearActions();

    void showError(const QString& message);
    void showNotice(const QString& message);
    void clearMessage();

    void open();
    void close();
    bool isOpen() const { return m_open; }

    int drawerWidth() const { return width(); }
    void setDrawerWidth(int width);

signals:
    void opened();
    void closed();

private:
    void animateTo(int width);

    QLabel* m_title = nullptr;
    QLabel* m_subtitle = nullptr;
    QLabel* m_message = nullptr;
    QHBoxLayout* m_actions = nullptr;
    QToolButton* m_closeButton = nullptr;
    QScrollArea* m_scroll = nullptr;
    QWidget* m_content = nullptr;
    QList<QPushButton*> m_actionButtons;
    QPropertyAnimation* m_animation = nullptr;
    bool m_open = false;
};

} // namespace edu::ui
