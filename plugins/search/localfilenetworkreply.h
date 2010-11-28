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


#ifndef KT_LOCALFILENETWORKREPLY_H
#define KT_LOCALFILENETWORKREPLY_H

#include <QNetworkReply>
#include <QFile>


namespace kt 
{
	/**
	 * QNetworkReply which reads a local file.
	 */
	class LocalFileNetworkReply : public QNetworkReply
	{
	public:
		LocalFileNetworkReply(const QString & file,QObject* parent = 0);
		virtual ~LocalFileNetworkReply();
		
		virtual void abort();
		virtual bool isSequential() const {return true;}
		virtual bool atEnd() const;
		virtual qint64 bytesAvailable() const;
		
	protected:
		virtual qint64 readData(char* data, qint64 maxlen);
		
	private:
		QFile* fptr;
	};

}

#endif // KT_LOCALFILENETWORKREPLY_H
