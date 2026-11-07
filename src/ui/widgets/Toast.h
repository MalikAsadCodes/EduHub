#pragma once

#include <QFrame>

namespace edu::ui {

enum class ToastKind { Info, Success, Warning, Error };

// A short message that fades in at the bottom-right of a window and fades out
// by itself. Used for confirmations ("Saved"); errors that need a decision
// still use a dialog.
class Toast : public QFrame {
    Q_OBJECT

public:
    static Toast* notify(QWidget* anywhereInWindow, const QString& text,
                         ToastKind kind = ToastKind::Info, int durationMs = 2600);

    QString text() const { return m_text; }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Toast(QWidget* window, const QString& text, ToastKind kind);
    void place();
    void fadeTo(qreal opacity, int durationMs, bool deleteAfter);

    QString m_text;
};

} // namespace edu::ui
