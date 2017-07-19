/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "addtrackersdialog.h"

#include <QApplication>
#include <QClipboard>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QUrl>

#include <KLocalizedString>

namespace kt
{

    AddTrackersDialog::AddTrackersDialog(QWidget* parent, const QStringList& tracker_hints): QDialog(parent)
    {
        setWindowTitle(i18n("Add Trackers"));
        trackers = new KEditListWidget(this);
        trackers->setButtons(KEditListWidget::Add | KEditListWidget::Remove);

        // If we find any urls on the clipboard, add them
        QClipboard* clipboard = QApplication::clipboard();
        const QStringList urlStrings = clipboard->text().split(QRegularExpression(QLatin1String("\\s")));
        for (const QString& s : urlStrings)
        {
            QUrl url(s);
            if (url.isValid() && (url.scheme() == QLatin1String("http")
                               || url.scheme() == QLatin1String("https")
                               || url.scheme() == QLatin1String("udp")))
            {
                trackers->insertItem(s);
            }
        }

        trackers->lineEdit()->setCompleter(new QCompleter(tracker_hints));

        QDialogButtonBox* box=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, this);
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
