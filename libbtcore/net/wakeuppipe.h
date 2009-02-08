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
#ifndef KTWAKEUPPIPE_H
#define KTWAKEUPPIPE_H



namespace net
{

	/**
		A WakeUpPipe's purpose is to wakeup a select or poll call.
		It works by using a pipe
		One end needs to be part of the poll or select, and the other end will send dummy data to it.
		Waking up the select or poll call.
	*/
	class WakeUpPipe
	{
	public:
		WakeUpPipe();
		virtual ~WakeUpPipe();
		
		/// Wake up the other socket
		void wakeUp();
		
		/// Get the reader socket
		int readerSocket() const {return reader;}
		
		/// Read all the dummy data
		void handleData();
	private:
		int reader;
		int writer;
	};

}

#endif
