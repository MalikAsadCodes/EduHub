#pragma once

#include <QFrame>
#include <QHash>
#include <QList>
#include <QStringList>

class QButtonGroup;
class QHBoxLayout;
class QLabel;
class QPropertyAnimation;
class QVBoxLayout;

namespace edu::ui {

class NavButton;

// Left navigation rail: brand mark, grouped page links, and Collapse /
// Sign out at the bottom. Collapses to icons only (tooltips show the names).
class NavRail : public QFrame {
    Q_OBJECT
    Q_PROPERTY(int railWidth READ railWidth WRITE setRailWidth)

public:
    static constexpr int kExpandedWidth = 236;
    static constexpr int kCollapsedWidth = 76;

    explicit NavRail(QWidget* parent = nullptr);

    void addSection(const QString& caption);
    void addItem(const QString& id, const QString& text, const QString& iconName);

    void setCurrent(const QString& id);
    QString current() const;
    QStringList itemIds() const { return m_order; }

    bool isCollapsed() const { return m_collapsed; }
    void setCollapsed(bool collapsed, bool animate = true);

    int railWidth() const { return width(); }
    void setRailWidth(int width);

signals:
    void currentChanged(const QString& id);
    void collapsedChanged(bool collapsed);
    void signOutRequested();

private:
    void applyCollapsedLook(bool collapsed);

    QVBoxLayout* m_items = nullptr;
    QHBoxLayout* m_brandRow = nullptr;
    QWidget* m_brandText = nullptr;
    QButtonGroup* m_group = nullptr;
    NavButton* m_toggle = nullptr;
    NavButton* m_signOut = nullptr;
    QPropertyAnimation* m_animation = nullptr;
    QHash<QString, NavButton*> m_buttons;
    QStringList m_order;
    QList<QLabel*> m_sections;
    bool m_collapsed = false;
};

} // namespace edu::ui
