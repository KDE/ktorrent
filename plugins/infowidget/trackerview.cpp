/***************************************************************************
 *   Copyright (C) 2006-2007 by Joris Guisson, Ivan Vasic                  *
 *   joris.guisson@gmail.com                                               *
 *	 ivasic@gmail.com                                                  *
 *									   *
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
#include "trackerview.h"
#include <interfaces/torrentinterface.h>
#include <interfaces/trackerslist.h>

#include <qdatetime.h>
#include <qstring.h>

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kglobal.h>

#include <torrent/globals.h>
#include <util/log.h>

#include "trackermodel.h"

using namespace bt;

namespace kt
{
	
	
	TrackerView::TrackerView(QWidget *parent)
		: QWidget(parent), tc(0)
	{
		setupUi(this);
		model = new TrackerModel(this);
		m_tracker_list->setModel(model);
		connect(m_add_tracker,SIGNAL(clicked()),this,SLOT(btnAddClicked()));
		connect(m_remove_tracker,SIGNAL(clicked()),this,SLOT(btnRemoveClicked()));
		connect(m_update_tracker,SIGNAL(clicked()),this,SLOT(btnUpdateClicked()));
		connect(m_change_tracker,SIGNAL(clicked()),this,SLOT(btnChangeClicked()));
		connect(m_restore_defaults,SIGNAL(clicked()),this,SLOT(btnRestoreClicked()));
		connect(m_tracker_list->selectionModel(),SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
			this,SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));

		m_url->setTextFormat(Qt::RichText);
		m_url->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_url->clear();
		m_status->setTextFormat(Qt::RichText);
		m_status->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
		m_status->clear();
		m_next_update->setTextFormat(Qt::RichText);
		
		m_update_tracker->setIcon(KIcon("system-software-update"));
		m_add_tracker->setIcon(KIcon("list-add"));
		m_remove_tracker->setIcon(KIcon("list-remove"));
		m_restore_defaults->setIcon(KIcon("kt-restore-defaults"));
		m_change_tracker->setIcon(KIcon("kt-change-tracker"));

		setEnabled(false);
		torrentChanged(0);
	}
	
	TrackerView::~TrackerView()
	{
	}

	void TrackerView::btnAddClicked()
	{
		if (!tc || m_tracker_to_add->text().trimmed().isEmpty())
			return;
		
		if (tc->getStats().priv_torrent)
		{
			KMessageBox::sorry(0, i18n("Cannot add a tracker to a private torrent."));
			return;
		}
		
		KUrl url(m_tracker_to_add->text());
		if (!url.isValid())
		{
			KMessageBox::error(0, i18n("Malformed URL."));
			return;
		}
			
		// check for dupes
		if (model->hasTracker(url))
		{
			KMessageBox::sorry(0,i18n("There already is a tracker named <b>%1</b> !",m_tracker_to_add->text()));
			return;
		}
		
		tc->getTrackersList()->addTracker(url,true);
		model->insertRow(model->rowCount(QModelIndex()));
		m_tracker_to_add->clear();
	}

	void TrackerView::btnRemoveClicked()
	{
		QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
		if (!current.isValid())
			return;
		
		if (tc->getTrackersList()->removeTracker(model->trackerUrl(current)))
			model->removeRow(current.row());
		else
			KMessageBox::sorry(0, i18n("Cannot remove torrent default tracker."));
	}

	void TrackerView::btnChangeClicked()
	{
		QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
		if (!current.isValid())
			return;
		
		KUrl url = model->trackerUrl(current);
		if (tc->getTrackersList()->isTrackerEnabled(url))
		{
			tc->getTrackersList()->setTracker(url);
			tc->updateTracker();
		}
	}

	void TrackerView::btnRestoreClicked()
	{
		tc->getTrackersList()->restoreDefault();
		tc->updateTracker();
		model->changeTC(tc); // trigger reset
	}

	void TrackerView::btnUpdateClicked()
	{
		if(!tc)
			return;
		
		tc->updateTracker();
	}
	
	void TrackerView::changeTC(TorrentInterface* ti)
	{
		if (tc == ti)
			return;
		
		setEnabled(ti != 0);
		torrentChanged(ti);
		update();
	}
	
	void TrackerView::update()
	{
		if(!tc)
			return;
		
		const TorrentStats & s = tc->getStats();
		if (s.running)
		{
			QTime t;
			t = t.addSecs(tc->getTimeToNextTrackerUpdate());
			m_next_update->setText("<b>" + t.toString("mm:ss") + "</b>");
		}
		else
		{
			m_next_update->clear();
		}
		
		m_leechers->setText(QString("<b>%1</b>").arg(s.leechers_total));
		m_seeders->setText(QString("<b>%1</b>").arg(s.seeders_total));
		m_times_downloaded->setText(QString("<b>%1</b>").arg(s.total_times_downloaded));
		
		//Update manual annunce button
		m_update_tracker->setEnabled(s.running && tc->announceAllowed());

		m_status->setText("<b>" + s.trackerstatus + "</b>");
		if (tc->getTrackersList())
			m_url->setText("<b>" + tc->getTrackersList()->getTrackerURL().prettyUrl() + "</b>");
		else
			m_url->clear();
		
		m_add_tracker->setEnabled(m_tracker_to_add->text() != QString::null && !tc->getStats().priv_torrent);
	}
	
	void TrackerView::torrentChanged(TorrentInterface* ti)
	{
		tc = ti;
		if(!tc)
		{
			m_status->clear();
			m_url->clear();
			m_next_update->clear();
			m_tracker_to_add->clear();
			m_leechers->clear();
			m_seeders->clear();
			m_times_downloaded->clear();
			
			m_add_tracker->setEnabled(false);
			m_remove_tracker->setEnabled(false);
			m_restore_defaults->setEnabled(false);
			m_change_tracker->setEnabled(false);
			m_update_tracker->setEnabled(false);
			model->changeTC(0);
			return;
		}
		
		const TorrentStats & s = tc->getStats();
		
		if (s.priv_torrent)
		{
			m_add_tracker->setEnabled(false);
			m_remove_tracker->setEnabled(false);
			m_restore_defaults->setEnabled(false);
			m_tracker_to_add->setText(i18n("You cannot add trackers to a private torrent"));
			m_tracker_to_add->setEnabled(false);
		}
		else
		{
			m_add_tracker->setEnabled(true);
			m_remove_tracker->setEnabled(true);
			m_restore_defaults->setEnabled(true);
			m_tracker_to_add->clear();
			m_tracker_to_add->setEnabled(true);
		}
		
		model->changeTC(tc);
		m_update_tracker->setEnabled(s.running && tc->announceAllowed());
		currentChanged(m_tracker_list->selectionModel()->currentIndex(),QModelIndex());
	}
	
	void TrackerView::currentChanged(const QModelIndex & current,const QModelIndex & previous)
	{
		Q_UNUSED(previous);
		if (!tc)
		{
			m_change_tracker->setEnabled(false);
			return;
		}
		
		const TorrentStats & s = tc->getStats();
	
		KUrl url = model->trackerUrl(current);
		m_change_tracker->setEnabled(s.running && model->rowCount(QModelIndex()) > 1 && tc->getTrackersList()->isTrackerEnabled(url));
	}
}


#include "trackerview.moc"
