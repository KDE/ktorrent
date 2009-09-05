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

#ifndef NET_REVERSERESOLVER_H
#define NET_REVERSERESOLVER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <btcore_export.h>
#include <net/address.h>


namespace net
{
	class ReverseResolverThread;
	
	/**
		Resolve an IP address into a hostname
		This should be threated as fire and forget objects, when using them asynchronously.
		The worker thread will delete them, when they are done.
	*/
	class ReverseResolver : public QObject
	{
		Q_OBJECT
	public:
		ReverseResolver(QObject* parent = 0);
		virtual ~ReverseResolver();
		
		/**
			Resolve an ip address asynchronously, uses the worker thread 
			Connecting to the resolved signal should be done with Qt::QueuedConnection, seeing
			that it will be emitted from the worker thread.
			@param addr The address
		*/
		void resolveAsync(const net::Address & addr);
		
		/**
			Resolve an ip address synchronously.
		*/
		QString resolve(const net::Address & addr);
		
		/**
			Run the actual resolve and emit the signal when done
		*/
		void run();
		
		/// Shutdown the worker thread
		static void shutdown();
		
	signals:
		/// Emitted when the resolution is complete
		void resolved(const QString & host);
		
	private:
		static ReverseResolverThread* worker;
		net::Address addr_to_resolve;
	};
	
	class ReverseResolverThread : public QThread
	{
		Q_OBJECT
	public:
		ReverseResolverThread();
		virtual ~ReverseResolverThread();
		
		/// Add a ReverseResolver to the todo list
		void add(ReverseResolver* rr);
		
		/// Run the thread
		virtual void run();
		
		/// Stop the thread
		void stop();
		
	private:
		QMutex mutex;
		QWaitCondition more_data;
		QList<ReverseResolver*> todo_list;
		bool stopped;
	};

}

#endif // NET_REVERSERESOLVER_H
