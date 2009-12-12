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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "functions.h"
#include <qdir.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <qdatetime.h>
#include <QNetworkInterface>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kglobal.h>
#include <interfaces/torrentinterface.h>
#include "error.h"
#include "log.h"

namespace bt
{

	bool IsMultimediaFile(const QString & filename)
	{
		KMimeType::Ptr ptr = KMimeType::findByPath(filename);
		QString name = ptr->name();
		return name.startsWith("audio") || name.startsWith("video") || name == "application/ogg";
	}
	
	QString DirSeparator()
	{
		//QString tmp;
		//tmp.append(QDir::separator());
		return "/";
	}

	void WriteUint64(Uint8* buf,Uint32 off,Uint64 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF00000000000000ULL) >> 56);
		buf[off + 1] = (Uint8) ((val & 0x00FF000000000000ULL) >> 48);
		buf[off + 2] = (Uint8) ((val & 0x0000FF0000000000ULL) >> 40);
		buf[off + 3] = (Uint8) ((val & 0x000000FF00000000ULL) >> 32);
		buf[off + 4] = (Uint8) ((val & 0x00000000FF000000ULL) >> 24);
		buf[off + 5] = (Uint8) ((val & 0x0000000000FF0000ULL) >> 16);
		buf[off + 6] = (Uint8) ((val & 0x000000000000FF00ULL) >> 8);
		buf[off + 7] = (Uint8) ((val & 0x00000000000000FFULL) >> 0);
	}
	
	Uint64 ReadUint64(const Uint8* buf,Uint64 off)
	{
		Uint64 tmp =
				((Uint64)buf[off] << 56) |
				((Uint64)buf[off+1] << 48) |
				((Uint64)buf[off+2] << 40) |
				((Uint64)buf[off+3] << 32) |
				((Uint64)buf[off+4] << 24) |
				((Uint64)buf[off+5] << 16) |
				((Uint64)buf[off+6] << 8) |
				((Uint64)buf[off+7] << 0);

		return tmp;
	}

	void WriteUint32(Uint8* buf,Uint32 off,Uint32 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF000000) >> 24);
		buf[off + 1] = (Uint8) ((val & 0x00FF0000) >> 16);
		buf[off + 2] = (Uint8) ((val & 0x0000FF00) >> 8);
		buf[off + 3] = (Uint8) (val & 0x000000FF);
	}
	
	Uint32 ReadUint32(const Uint8* buf,Uint32 off)
	{
		return (buf[off] << 24) | (buf[off+1] << 16) | (buf[off+2] << 8) | buf[off + 3];
	}
	
	void WriteUint16(Uint8* buf,Uint32 off,Uint16 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF00) >> 8);
		buf[off + 1] = (Uint8) (val & 0x000FF);
	}

	Uint16 ReadUint16(const Uint8* buf,Uint32 off)
	{
		return (buf[off] << 8) | buf[off + 1];
	}
	
	
	void WriteInt64(Uint8* buf,Uint32 off,Int64 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF00000000000000ULL) >> 56);
		buf[off + 1] = (Uint8) ((val & 0x00FF000000000000ULL) >> 48);
		buf[off + 2] = (Uint8) ((val & 0x0000FF0000000000ULL) >> 40);
		buf[off + 3] = (Uint8) ((val & 0x000000FF00000000ULL) >> 32);
		buf[off + 4] = (Uint8) ((val & 0x00000000FF000000ULL) >> 24);
		buf[off + 5] = (Uint8) ((val & 0x0000000000FF0000ULL) >> 16);
		buf[off + 6] = (Uint8) ((val & 0x000000000000FF00ULL) >> 8);
		buf[off + 7] = (Uint8) ((val & 0x00000000000000FFULL) >> 0);
	}
	
	Int64 ReadInt64(const Uint8* buf,Uint32 off)
	{
		Int64 tmp =
				((Int64)buf[off] << 56) |
				((Int64)buf[off+1] << 48) |
				((Int64)buf[off+2] << 40) |
				((Int64)buf[off+3] << 32) |
				((Int64)buf[off+4] << 24) |
				((Int64)buf[off+5] << 16) |
				((Int64)buf[off+6] << 8) |
				((Int64)buf[off+7] << 0);

		return tmp;
	}
	
	void WriteInt32(Uint8* buf,Uint32 off,Int32 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF000000) >> 24);
		buf[off + 1] = (Uint8) ((val & 0x00FF0000) >> 16);
		buf[off + 2] = (Uint8) ((val & 0x0000FF00) >> 8);
		buf[off + 3] = (Uint8) (val & 0x000000FF);
	}
	
	Int32 ReadInt32(const Uint8* buf,Uint32 off)
	{
		return (Int32)(buf[off] << 24) | (buf[off+1] << 16) | (buf[off+2] << 8) | buf[off + 3];
	}
	
	void WriteInt16(Uint8* buf,Uint32 off,Int16 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF00) >> 8);
		buf[off + 1] = (Uint8) (val & 0x000FF);
	}
	
	Int16 ReadInt16(const Uint8* buf,Uint32 off)
	{
		return (Int16)(buf[off] << 8) | buf[off + 1];
	}
	
	void UpdateCurrentTime()
	{
		global_time_stamp = Now();
	}
	
	TimeStamp global_time_stamp = 0;
	
	Uint64 Now()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		global_time_stamp = (Uint64)tv.tv_sec * 1000 + (Uint64)tv.tv_usec * 0.001;
		return global_time_stamp;
	}
	
	Uint32 MaxOpenFiles()
	{
		struct rlimit lim;
		getrlimit(RLIMIT_NOFILE,&lim);
		return lim.rlim_cur;
	}

	bool MaximizeLimits()
	{
		// first get the current limits
		struct rlimit lim;
		getrlimit(RLIMIT_NOFILE,&lim);
		
		if (lim.rlim_cur != lim.rlim_max)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Current limit for number of files : " << lim.rlim_cur 
					<< " (" << lim.rlim_max << " max)" << endl;
			lim.rlim_cur = lim.rlim_max;
			if (setrlimit(RLIMIT_NOFILE,&lim) < 0)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Failed to maximize file limit : " 
						<< QString(strerror(errno)) << endl;
				return false;
			}
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "File limit already at maximum " << endl;
		}
		
		getrlimit(RLIMIT_DATA,&lim);
		if (lim.rlim_cur != lim.rlim_max)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Current limit for data size : " << lim.rlim_cur 
					<< " (" << 	lim.rlim_max << " max)" << endl;
			lim.rlim_cur = lim.rlim_max;
			if (setrlimit(RLIMIT_DATA,&lim) < 0)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Failed to maximize data limit : " 
						<< QString(strerror(errno)) << endl;
				return false;
			}
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Data limit already at maximum " << endl;
		}
		
		return true;
	}

	static QString net_iface = QString();
	
	void SetNetworkInterface(const QString & iface)
	{
		net_iface = iface;
	}

	QString NetworkInterface()
	{
		return net_iface;
		
	}
	
	QString NetworkInterfaceIPAddress(const QString & iface)
	{
		QNetworkInterface ni = QNetworkInterface::interfaceFromName(iface);
		if (!ni.isValid())
			return QString();

		QList<QNetworkAddressEntry> addr_list = ni.addressEntries();
		if (addr_list.count() == 0)
			return QString();
		else
			return addr_list.front().ip().toString();
	}	

		
	QString BytesToString(Uint64 bytes)
	{
		KLocale* loc = KGlobal::locale();
		return loc->formatByteSize(bytes);
	}

	QString BytesPerSecToString(double speed)
	{
		KLocale* loc = KGlobal::locale();
		return i18n("%1/s",loc->formatByteSize(speed));
	}

	QString DurationToString(Uint32 nsecs)
	{
		KLocale* loc = KGlobal::locale();
		QTime t;
		int ndays = nsecs / 86400;
		t = t.addSecs(nsecs % 86400);
		QString s = loc->formatTime(t,true,true);
		if (ndays > 0)
			s = i18np("1 day ","%1 days ",ndays) + s;

		return s;
	}
	
	double Percentage(const TorrentStats & s)
	{
		if (s.bytes_left_to_download == 0)
		{
			return 100.0;
		}
		else
		{
			if (s.total_bytes_to_download == 0)
			{
				return 100.0;
			}
			else
			{
				double perc = 100.0 - ((double)s.bytes_left_to_download / s.total_bytes_to_download) * 100.0;
				if (perc > 100.0)
					perc = 100.0;
				else if (perc > 99.9)
					perc = 99.9;
				else if (perc < 0.0)
					perc = 0.0;
				
				return perc;
			}
		}
	}

#ifdef Q_WS_WIN
	bool InitWindowsSocketsAPI()
	{
		static bool initialized = false;
		if (initialized)
			return true;

		WSADATA wsaData;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int err = WSAStartup( wVersionRequested, &wsaData );
		if (err != 0) 
			return false;

		initialized = true;
		return true;
	}
#endif
}
