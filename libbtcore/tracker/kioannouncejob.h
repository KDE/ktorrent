/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef BT_KIOANNOUNCEJOB_H
#define BT_KIOANNOUNCEJOB_H

#include <btcore_export.h>
#include <kio/jobclasses.h>


namespace bt
{

	class BTCORE_EXPORT KIOAnnounceJob : public KIO::Job
	{
		Q_OBJECT
	public:
		KIOAnnounceJob(const KUrl & url,const KIO::MetaData & md);
		virtual ~KIOAnnounceJob();
		
		/// Get the announce url
		KUrl announceUrl() const {return url;}
		
		/// Get the reply data
		const QByteArray & replyData() const {return reply_data;}
		
		virtual bool doKill();
		
	private slots:
		void data(KIO::Job* j,const QByteArray & data);
		void finished(KJob* j);
	
	private:
		KUrl url;
		QByteArray reply_data;
		KIO::TransferJob* get_job;
	};

}

#endif // BT_KIOANNOUNCEJOB_H
