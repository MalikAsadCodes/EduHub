#include "ui/widgets/Toast.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"

#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

namespace edu::ui {

namespace {

constexpr int kMargin = 24;

QIcon iconFor(ToastKind kind)
{
    switch (kind) {
    case ToastKind::Success:
        return Icons::themed(QStringLiteral("check-circle"), ColorRole::Success);
    case ToastKind::Warning:
        return Icons::themed(QStringLiteral("alert"), ColorRole::Warning);
    case ToastKind::Error:
        return Icons::themed(QStringLiteral("x-circle"), ColorRole::Danger);
    case ToastKind::Info:
        break;
    }
    return Icons::themed(QStringLiteral("info"), ColorRole::Info);
}

} // namespace

Toast* Toast::notify(QWidget* anywhereInWindow, const QString& text, ToastKind kind, int durationMs)
{
    if (!anywhereInWindow)
        return nullptr;
    QWidget* window = anywhereInWindow->window();

    // One toast at a time per window.
    const auto existing = window->findChildren<Toast*>(QString(), Qt::FindDirectChildrenOnly);
    for (Toast* old : existing)
        old->deleteLater();

    auto* toast = new Toast(window, text, kind);
    toast->show();
    toast->raise();
    toast->fadeTo(1.0, 160, false);
    QTimer::singleShot(durationMs, toast, [toast] { toast->fadeTo(0.0, 220, true); });
    return toast;
}

Toast::Toast(QWidget* window, const QString& text, ToastKind kind) : QFrame(window), m_text(text)
{
    setObjectName(QStringLiteral("Toast"));
    setAttribute(Qt::WA_StyledBackground);

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(14, 12, 18, 12);
    row->setSpacing(10);
    row->addWidget(new IconLabel(iconFor(kind), 18, this), 0, Qt::AlignTop);
    auto* label = new QLabel(text, this);
    label->setWordWrap(true);
    label->setMaximumWidth(340);
    row->addWidget(label);

    auto* effect = new QGraphicsOpacityEffect(this);
    effect->setOpacity(0.0);
    setGraphicsEffect(effect);

    window->installEventFilter(this);
    place();
}

void Toast::place()
{
    adjustSize();
    QWidget* window = parentWidget();
    move(window->width() - width() - kMargin, window->height() - height() - kMargin);
}

bool Toast::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize)
        place();
    return QFrame::eventFilter(watched, event);
}

void Toast::fadeTo(qreal opacity, int durationMs, bool deleteAfter)
{
    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(graphicsEffect());
    if (!effect)
        return;
    auto* animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(durationMs);
    animation->setStartValue(effect->opacity());
    animation->setEndValue(opacity);
    if (deleteAfter)
        connect(animation, &QPropertyAnimation::finished, this, &QObject::deleteLater);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace edu::ui
