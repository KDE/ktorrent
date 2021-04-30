/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "addtrackersdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QUrl>
#include <QVBoxLayout>

#include <KLocalizedString>

namespace kt
{
AddTrackersDialog::AddTrackersDialog(QWidget *parent, const QStringList &tracker_hints)
    : QDialog(parent)
{
    setWindowTitle(i18n("Add Trackers"));
    trackers = new KEditListWidget(this);
    trackers->setButtons(KEditListWidget::Add | KEditListWidget::Remove);

    // If we find any urls on the clipboard, add them
    QClipboard *clipboard = QApplication::clipboard();
    const QStringList urlStrings = clipboard->text().split(QRegularExpression(QLatin1String("\\s")));
    for (const QString &s : urlStrings) {
        QUrl url(s);
        if (url.isValid() && (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("https") || url.scheme() == QLatin1String("udp"))) {
            trackers->insertItem(s);
        }
    }

    trackers->lineEdit()->setCompleter(new QCompleter(tracker_hints));

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(box, &QDialogButtonBox::accepted, this, &AddTrackersDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &AddTrackersDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(trackers);
    layout->addWidget(box);
}

AddTrackersDialog::~AddTrackersDialog()
{
}

QStringList AddTrackersDialog::trackerList() const
{
    return trackers->items();
}

}
