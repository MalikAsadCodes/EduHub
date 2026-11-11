#include "ui/widgets/Dialogs.h"

#include "ui/theme/Icons.h"
#include "ui/widgets/IconLabel.h"
#include "ui/widgets/Toast.h"
#include "ui/widgets/UiHelpers.h"

#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace edu::ui {

namespace {

struct ToneLook {
    QString icon;
    ColorRole color;
};

ToneLook lookFor(DialogTone tone)
{
    switch (tone) {
    case DialogTone::Warning:
        return {QStringLiteral("alert"), ColorRole::Warning};
    case DialogTone::Danger:
        return {QStringLiteral("alert"), ColorRole::Danger};
    case DialogTone::Success:
        return {QStringLiteral("check-circle"), ColorRole::Success};
    case DialogTone::Info:
        break;
    }
    return {QStringLiteral("info"), ColorRole::Info};
}

} // namespace

MessageDialog::MessageDialog(QWidget* parent, DialogTone tone, const QString& title,
                             const QString& text, const QString& acceptText,
                             const QString& rejectText)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("MessageDialog"));
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(420);

    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(24, 22, 24, 20);
    column->setSpacing(14);

    auto* top = new QHBoxLayout;
    top->setSpacing(14);
    const ToneLook look = lookFor(tone);
    auto* icon = new IconLabel(Icons::themed(look.icon, look.color), 22, this);
    icon->setBadge(ColorRole::SurfaceAlt, 9, 12);
    top->addWidget(icon, 0, Qt::AlignTop);
    auto* texts = new QVBoxLayout;
    texts->setSpacing(6);
    texts->addWidget(makeLabel(title, QStringLiteral("dialogTitle"), this));
    auto* body = makeLabel(text, QStringLiteral("dialogText"), this);
    body->setWordWrap(true);
    body->setTextInteractionFlags(Qt::TextSelectableByMouse);
    texts->addWidget(body);
    top->addLayout(texts, 1);
    column->addLayout(top);

    auto* buttons = new QHBoxLayout;
    buttons->addStretch(1);
    if (!rejectText.isEmpty()) {
        auto* reject = new QPushButton(rejectText, this);
        reject->setProperty("variant", QStringLiteral("secondary"));
        connect(reject, &QPushButton::clicked, this, &QDialog::reject);
        buttons->addWidget(reject);
    }
    m_accept = new QPushButton(acceptText, this);
    m_accept->setProperty("variant", tone == DialogTone::Danger ? QStringLiteral("dangerSolid")
                                                                : QStringLiteral("primary"));
    m_accept->setDefault(true);
    connect(m_accept, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(m_accept);
    column->addLayout(buttons);
}

bool confirm(QWidget* parent, const QString& title, const QString& text,
             const QString& confirmText, bool danger)
{
    MessageDialog dialog(parent, danger ? DialogTone::Danger : DialogTone::Warning, title, text,
                         confirmText, QStringLiteral("Cancel"));
    return dialog.exec() == QDialog::Accepted;
}

void showError(QWidget* parent, const QString& title, const QString& text)
{
    MessageDialog dialog(parent, DialogTone::Danger, title, text, QStringLiteral("OK"));
    dialog.exec();
}

QDialog* showCredentials(QWidget* parent, const QString& title, const QString& username,
                         const QString& password, const QString& note)
{
    auto* dialog = new QDialog(parent);
    dialog->setObjectName(QStringLiteral("CredentialsDialog"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(title);
    dialog->setModal(true);
    dialog->setMinimumWidth(440);

    auto* column = new QVBoxLayout(dialog);
    column->setContentsMargins(24, 22, 24, 20);
    column->setSpacing(12);

    auto* top = new QHBoxLayout;
    top->setSpacing(14);
    auto* icon = new IconLabel(Icons::themed(QStringLiteral("key"), ColorRole::AccentText), 22,
                               dialog);
    icon->setBadge(ColorRole::AccentSoft, 9, 12);
    top->addWidget(icon, 0, Qt::AlignTop);
    auto* texts = new QVBoxLayout;
    texts->setSpacing(6);
    texts->addWidget(makeLabel(title, QStringLiteral("dialogTitle"), dialog));
    auto* noteLabel = makeLabel(note, QStringLiteral("dialogText"), dialog);
    noteLabel->setWordWrap(true);
    texts->addWidget(noteLabel);
    top->addLayout(texts, 1);
    column->addLayout(top);

    auto* box = new QFrame(dialog);
    box->setObjectName(QStringLiteral("CredentialBox"));
    auto* grid = new QVBoxLayout(box);
    grid->setContentsMargins(16, 12, 16, 12);
    grid->setSpacing(6);
    auto addLine = [&](const QString& label, const QString& value, const char* name) {
        auto* row = new QHBoxLayout;
        auto* l = makeLabel(label, QStringLiteral("detailLabel"), box);
        l->setFixedWidth(150);
        auto* v = makeLabel(value, QStringLiteral("credentialValue"), box);
        v->setObjectName(QString::fromLatin1(name));
        v->setTextInteractionFlags(Qt::TextSelectableByMouse);
        row->addWidget(l);
        row->addWidget(v, 1);
        grid->addLayout(row);
    };
    addLine(QStringLiteral("Username"), username, "CredentialUsername");
    addLine(QStringLiteral("Temporary password"), password, "CredentialPassword");
    column->addWidget(box);

    auto* buttons = new QHBoxLayout;
    auto* copy = new QPushButton(QStringLiteral("Copy"), dialog);
    copy->setProperty("variant", QStringLiteral("secondary"));
    copy->setIcon(Icons::themed(QStringLiteral("copy"), ColorRole::Text));
    QObject::connect(copy, &QPushButton::clicked, dialog, [dialog, username, password] {
        QApplication::clipboard()->setText(
            QStringLiteral("Username: %1\nTemporary password: %2").arg(username, password));
        Toast::notify(dialog, QStringLiteral("Copied"), ToastKind::Success, 1400);
    });
    buttons->addWidget(copy);
    buttons->addStretch(1);
    auto* done = new QPushButton(QStringLiteral("Done"), dialog);
    done->setObjectName(QStringLiteral("CredentialsDone"));
    done->setProperty("variant", QStringLiteral("primary"));
    done->setDefault(true);
    QObject::connect(done, &QPushButton::clicked, dialog, &QDialog::accept);
    buttons->addWidget(done);
    column->addLayout(buttons);

    dialog->open();
    return dialog;
}

} // namespace edu::ui
