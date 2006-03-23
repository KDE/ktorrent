/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "trackerview.h"
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>

#include <torrent/globals.h>
#include <util/log.h>

namespace kt
{
	TrackerView::TrackerView(TorrentInterface* ti, QWidget *parent, const char *name)
		:TrackerViewBase(parent, name), tc(ti)
	{
		if(!tc)
			return;
		
		const KURL::List trackers = tc->getTrackersList()->getTrackerURLs();
		if(trackers.empty())
			return;
		
		for (KURL::List::const_iterator i = trackers.begin();i != trackers.end();i++)
			new QListViewItem(listTrackers, (*i).prettyURL());
	}

	void TrackerView::btnAdd_clicked()
	{
		if(!tc)
			return;
		
		KURL url(txtTracker->text());
		if(!url.isValid())
		{
			KMessageBox::error(0, i18n("Malformed URL."));
			return;
		}
			
		new QListViewItem(listTrackers, txtTracker->text());
		tc->getTrackersList()->addTracker(url);
	}

	void TrackerView::btnRemove_clicked()
	{
		QListViewItem* current = listTrackers->currentItem();
		if(!current)
			return;
		
		KURL url(current->text(0));
		if(tc->getTrackersList()->removeTracker(url))
			delete current;
		else
			KMessageBox::sorry(0, i18n("Cannot remove torrent default tracker."));
	}

	void TrackerView::btnChange_clicked()
	{
		QListViewItem* current = listTrackers->currentItem();
		if(!current)
			return;
		
		KURL url(current->text(0));
		tc->getTrackersList()->setTracker(url);
		tc->updateTracker();
	}

	void TrackerView::btnRestore_clicked()
	{
		tc->getTrackersList()->restoreDefault();
		tc->updateTracker();
	}

	void TrackerView::btnUpdate_clicked()
	{
		if(!tc)
			return;
		
		tc->updateTracker();
	}
	
	void TrackerView::listTrackers_currentChanged(QListViewItem* item)
	{
		if(!item)
			txtTracker->clear();
		else
			txtTracker->setText(item->text(0));
	}
	
	void TrackerView::update(TorrentInterface* ti)
	{
		tc = ti;
		if(!tc)
			return;
		
		TorrentStats s = tc->getStats();
		if(s.running)
		{
			QTime t;
			t = t.addSecs(tc->getTimeToNextTrackerUpdate());
			lblUpdate->setText(t.toString("mm:ss"));
		}
	
		lblStatus->setText("<b>" + s.trackerstatus + "</b>");
		lblCurrent->setText("<b>" + tc->getTrackerURL(true).prettyURL() + "</b>");
		btnAdd->setEnabled(txtTracker->text() != QString::null);
	}
	
	void TrackerView::torrentChanged(TorrentInterface* ti)
	{
		tc = ti;
		listTrackers->clear();
		if(!tc)
			return;
		
// 		const KURL::List trackers = tc-> getTrackers();
		const KURL::List trackers = tc->getTrackersList()->getTrackerURLs();
		if(trackers.empty())
		{
// 			new QListViewItem(listTrackers, tc->getTrackerURL(true).prettyURL());
			new QListViewItem(listTrackers, tc->getTrackersList()->getTrackerURL(true).prettyURL());
			return;
		}
		
		for (KURL::List::const_iterator i = trackers.begin();i != trackers.end();i++)
			new QListViewItem(listTrackers, (*i).prettyURL());
	}
}

#include "trackerview.moc"
