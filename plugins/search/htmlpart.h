/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef HTMLPART_H
#define HTMLPART_H

#include <khtml_part.h>

class KJob;
class KUrl;

namespace KIO
{
	class Job;
}


namespace kt
{

	/**
	@author Joris Guisson
	*/
	class HTMLPart : public KHTMLPart
	{
		Q_OBJECT
	public:
		HTMLPart(QWidget *parent = 0);
		virtual ~HTMLPart();
		
		bool backAvailable() const {return history.count() > 1;}
		QString title() const;
	
	public slots:
		void back();
		void reload();
		void copy();
		void openUrlRequest(const KUrl &url, const KParts::OpenUrlArguments & arg, const KParts::BrowserArguments & barg);
		void addToHistory(const KUrl & url);
	
	private slots:
		void dataReceived(KIO::Job* job,const QByteArray & data);
		void mimetype(KIO::Job* job,const QString & mt);
		void jobDone(KJob* job);
		
	
	signals:
		void backAvailable(bool yes);
		void openTorrent(const KUrl & url);
		void saveTorrent(const KUrl & url);
		void searchFinished();
	
	private:
		KUrl::List history;
		KJob* active_job;
		QByteArray curr_data;
		QString mime_type;
		KUrl curr_url;
	};
}

#endif
