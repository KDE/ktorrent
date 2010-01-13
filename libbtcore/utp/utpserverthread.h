/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef UTP_UTPSERVERTHREAD_H
#define UTP_UTPSERVERTHREAD_H

#include <QThread>
#include <btcore_export.h>

namespace utp
{
	class UTPServer;

	class BTCORE_EXPORT UTPServerThread : public QThread
	{
		Q_OBJECT
	public:
		UTPServerThread(UTPServer* srv);
		virtual ~UTPServerThread();
		
		virtual void run();
		
	protected:
		UTPServer* srv;
	};

}

#endif // UTP_UTPSERVERTHREAD_H
