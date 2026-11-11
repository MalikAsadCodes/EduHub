#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;

namespace edu::ui {

enum class DialogTone { Info, Warning, Danger, Success };

// Small styled dialog with an icon, a title, text and up to two buttons.
class MessageDialog : public QDialog {
    Q_OBJECT

public:
    MessageDialog(QWidget* parent, DialogTone tone, const QString& title, const QString& text,
                  const QString& acceptText, const QString& rejectText = QString());

    QPushButton* acceptButton() const { return m_accept; }

private:
    QPushButton* m_accept = nullptr;
};

// Asks a yes/no question. Returns true when the user confirms.
bool confirm(QWidget* parent, const QString& title, const QString& text,
             const QString& confirmText, bool danger = false);

// Shows an error that needs attention (not used for simple confirmations).
void showError(QWidget* parent, const QString& title, const QString& text);

// Shows a username and a temporary password once, with a copy button.
// Non-blocking: returns the dialog, which deletes itself when closed.
QDialog* showCredentials(QWidget* parent, const QString& title, const QString& username,
                         const QString& password, const QString& note);

} // namespace edu::ui
