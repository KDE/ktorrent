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
#include <QRegExp>
#include <QApplication>
#include <QClipboard>
#include <KLocalizedString>
#include <QLineEdit>

namespace kt
{

    AddTrackersDialog::AddTrackersDialog(QWidget* parent, const QStringList& tracker_hints): KDialog(parent)
    {
        setButtons(KDialog::Ok | KDialog::Cancel);
        showButtonSeparator(true);
        setCaption(i18n("Add Trackers"));
        trackers = new KEditListWidget(this);
        trackers->setButtons(KEditListWidget::Add | KEditListWidget::Remove);

        // If we find any urls on the clipboard, add them
        QClipboard* clipboard = QApplication::clipboard();
        QStringList strings = clipboard->text().split(QRegExp("\\s"));
        foreach (const QString& s, strings)
        {
            KUrl url(s);
            if (url.isValid() && (url.protocol() == "http" || url.protocol() == "https" || url.protocol() == "udp"))
            {
                trackers->insertItem(s);
            }
        }

        KCompletion* completion = new KCompletion();
        completion->insertItems(tracker_hints);
        completion->setCompletionMode(KGlobalSettings::CompletionPopup);

        trackers->lineEdit()->setCompletionObject(completion);

        setMainWidget(trackers);
    }

    AddTrackersDialog::~AddTrackersDialog()
    {
    }

    QStringList AddTrackersDialog::trackerList() const
    {
        return trackers->items();
    }


}
