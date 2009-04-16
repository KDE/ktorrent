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

#include <QHeaderView>

#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kglobal.h>

#include <torrent/globals.h>
#include <interfaces/trackerinterface.h>
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
		m_tracker_list->setAllColumnsShowFocus(true);
		m_tracker_list->setRootIsDecorated(false);
		m_tracker_list->setAlternatingRowColors(true);
		connect(m_add_tracker,SIGNAL(clicked()),this,SLOT(addClicked()));
		connect(m_remove_tracker,SIGNAL(clicked()),this,SLOT(removeClicked()));
		//connect(m_announce,SIGNAL(clicked()),this,SLOT(updateClicked()));
		//connect(m_scrape,SIGNAL(clicked()),this,SLOT(scrapeClicked()));
		connect(m_change_tracker,SIGNAL(clicked()),this,SLOT(changeClicked()));
		connect(m_restore_defaults,SIGNAL(clicked()),this,SLOT(restoreClicked()));
		connect(m_tracker_list->selectionModel(),SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
			this,SLOT(currentChanged(const QModelIndex&, const QModelIndex&)));
		
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

	void TrackerView::addClicked()
	{
		if (!tc || tc->getStats().priv_torrent)
			return;
		
		/*
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
			KMessageBox::sorry(0,i18n("There already is a tracker named <b>%1</b>.",m_tracker_to_add->text()));
			return;
		}
		
		tc->getTrackersList()->addTracker(url,true);
		model->insertRow(model->rowCount(QModelIndex()));
		m_tracker_to_add->clear();
		*/
	}

	void TrackerView::removeClicked()
	{
		QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
		if (!current.isValid())
			return;
		
		if (tc->getTrackersList()->removeTracker(model->trackerUrl(current)))
			model->removeRow(current.row());
		else
			KMessageBox::sorry(0, i18n("Cannot remove torrent default tracker."));
	}

	void TrackerView::changeClicked()
	{
		/*
		QModelIndex current = m_tracker_list->selectionModel()->currentIndex();
		if (!current.isValid())
			return;
		
		KUrl url = model->trackerUrl(current);
		if (tc->getTrackersList()->isTrackerEnabled(url))
		{
			tc->getTrackersList()->setTracker(url);
			tc->updateTracker();
		}
		*/
	}

	void TrackerView::restoreClicked()
	{
		tc->getTrackersList()->restoreDefault();
		tc->updateTracker();
		model->changeTC(tc); // trigger reset
	}

	void TrackerView::updateClicked()
	{
		if(!tc)
			return;
		
		tc->updateTracker();
	}
	
	void TrackerView::scrapeClicked()
	{
		if(!tc)
			return;
		
		tc->scrapeTracker();
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
		
	}
	
	void TrackerView::torrentChanged(TorrentInterface* ti)
	{
		tc = ti;
		if(!tc)
		{
			m_add_tracker->setEnabled(false);
			m_remove_tracker->setEnabled(false);
			m_restore_defaults->setEnabled(false);
			m_change_tracker->setEnabled(false);
			model->changeTC(0);
			return;
		}
		
		const TorrentStats & s = tc->getStats();
		
		if (s.priv_torrent)
		{
			m_add_tracker->setEnabled(false);
			m_remove_tracker->setEnabled(false);
			m_restore_defaults->setEnabled(false);
		}
		else
		{
			m_add_tracker->setEnabled(true);
			m_remove_tracker->setEnabled(true);
			m_restore_defaults->setEnabled(true);
		}
		
		model->changeTC(tc);
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
	
		bt::TrackerInterface* trk = model->tracker(current);
		m_change_tracker->setEnabled(s.running && model->rowCount(QModelIndex()) > 1 && trk->isEnabled());
	}
	
	void TrackerView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("TrackerView");
		QByteArray s = m_tracker_list->header()->saveState();
		g.writeEntry("state",s.toBase64());
	}
	
	void TrackerView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("TrackerView");
		QByteArray s = QByteArray::fromBase64(g.readEntry("state",QByteArray()));
		if (!s.isNull())
		{
			QHeaderView* v = m_tracker_list->header();
			v->restoreState(s);
		}
	}
}


#include "trackerview.moc"
