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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <qglobal.h>

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#if defined(Q_OS_LINUX) && !defined(__FreeBSD_kernel__)
#include <asm/ioctls.h>
#endif

#ifdef Q_OS_SOLARIS
#include <sys/filio.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <unistd.h>
#include <fcntl.h>

#include <torrent/globals.h>
#include <util/log.h>
#include "socket.h"

using namespace bt;

namespace net
{

	Socket::Socket(int fd) : m_fd(fd),m_state(IDLE)
	{
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && !defined(__DragonFly__) && __FreeBSD_version < 600020)
		int val = 1; 
		if (setsockopt(m_fd,SOL_SOCKET,SO_NOSIGPIPE,&val,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the NOSIGPIPE option : %1").arg(strerror(errno)) << endl;
		}
#endif
		cacheAddress();
	}
	
	Socket::Socket(bool tcp) : m_fd(-1),m_state(IDLE)
	{
		int fd = socket(PF_INET,tcp ? SOCK_STREAM : SOCK_DGRAM,0);
		if (fd < 0)
		{
			Out(SYS_GEN|LOG_IMPORTANT) << QString("Cannot create socket : %1").arg(strerror(errno)) << endl;
		}
		m_fd = fd;
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && !defined(__DragonFly__) && __FreeBSD_version < 600020)
		int val = 1;
		if (setsockopt(m_fd,SOL_SOCKET,SO_NOSIGPIPE,&val,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the NOSIGPIPE option : %1").arg(strerror(errno)) << endl;
		}
#endif	
	}
	
	Socket::~Socket()
	{
		if (m_fd >= 0)
		{
			shutdown(m_fd, SHUT_RDWR);
			::close(m_fd);
		}
	}
	
	void Socket::close()
	{
		if (m_fd >= 0)
		{
			shutdown(m_fd, SHUT_RDWR);
			::close(m_fd);
			m_fd = -1;
			m_state = CLOSED;
		}
	}
	
	void Socket::setNonBlocking()
	{
		fcntl(m_fd, F_SETFL, O_NONBLOCK);
	}
		
	bool Socket::connectTo(const Address & a)
	{
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(a.port());
		addr.sin_addr.s_addr = htonl(a.ip());

		if (::connect(m_fd,(struct sockaddr*)&addr,sizeof(struct sockaddr)) < 0)
		{
			if (errno == EINPROGRESS)
			{
			//	Out(SYS_CON|LOG_DEBUG) << "Socket is connecting" << endl;
				m_state = CONNECTING;
				return false;
			}
			else
			{
				Out(SYS_CON|LOG_NOTICE) << QString("Cannot connect to host %1:%2 : %3")
					.arg(a.toString()).arg(a.port()).arg(strerror(errno)) << endl;
				return false;
			}
		}
		m_state = CONNECTED;
		cacheAddress();
		return true;
	}
	
	bool Socket::bind(Uint16 port,bool also_listen)
	{
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
	
		if (::bind(m_fd,(struct sockaddr*)&addr,sizeof(struct sockaddr)) < 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot bind to port %1 : %2").arg(port).arg(strerror(errno)) << endl;
			return false;
		}

		if (also_listen && listen(m_fd,5) < 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot listen to port %1 : %2").arg(port).arg(strerror(errno)) << endl;
			return false;
		}

		int val = 1;
		if (setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the reuseaddr option : %1").arg(strerror(errno)) << endl;
		}
		m_state = BOUND;
		return true;
	}
	
	int Socket::send(const bt::Uint8* buf,int len)
	{
		int ret = ::send(m_fd,buf,len,MSG_NOSIGNAL);
		if (ret < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
			//	Out(SYS_CON|LOG_DEBUG) << "Send error : " << QString(strerror(errno)) << endl;
				close();
			}
			return 0;
		}
		return ret;
	}
	
	int Socket::recv(bt::Uint8* buf,int max_len)
	{
		int ret = ::recv(m_fd,buf,max_len,0);
		if (ret < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
			//	Out(SYS_CON|LOG_DEBUG) << "Receive error : " << QString(strerror(errno)) << endl;
				close();
			}
			return 0;
		}
		else if (ret == 0)
		{
			// connection closed
			close();
			return 0;
		}
		return ret;
	}
	
	int Socket::sendTo(const bt::Uint8* buf,int len,const Address & a)
	{
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(a.port());
		addr.sin_addr.s_addr = htonl(a.ip());

		int ns = 0;
		while (ns < len)
		{
			int left = len - ns;
			int ret = ::sendto(m_fd,(char*)buf + ns,left,0,(struct sockaddr*)&addr,sizeof(struct sockaddr));
			if (ret < 0)
			{
				Out(SYS_CON|LOG_DEBUG) << "Send error : " << QString(strerror(errno)) << endl;
				return 0;
			}

			ns += ret;
		}
		return ns;
	}
	
	int Socket::recvFrom(bt::Uint8* buf,int max_len,Address & a)
	{
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		socklen_t sl = sizeof(struct sockaddr);
		
		int ret = ::recvfrom(m_fd,buf,max_len,0,(struct sockaddr*)&addr,&sl);
		if (ret < 0)
		{
			Out(SYS_CON|LOG_DEBUG) << "Receive error : " << QString(strerror(errno)) << endl;
			return 0;
		}

		a.setPort(ntohs(addr.sin_port));
		a.setIP(ntohl(addr.sin_addr.s_addr));
		return ret;
	}
	
	int Socket::accept(Address & a)
	{
		struct sockaddr_in addr;
		memset(&addr,0,sizeof(struct sockaddr_in));
		socklen_t slen = sizeof(struct sockaddr_in);

		int sfd = ::accept(m_fd,(struct sockaddr*)&addr,&slen);
		if (sfd < 0)
		{
			Out(SYS_CON|LOG_DEBUG) << "Accept error : " << QString(strerror(errno)) << endl;
			return -1;
		}
		
		a.setPort(ntohs(addr.sin_port));
		a.setIP(ntohl(addr.sin_addr.s_addr));

		Out(SYS_CON|LOG_DEBUG) << "Accepted connection from " << QString(inet_ntoa(addr.sin_addr)) << endl;
		return sfd;
	}
	
	bool Socket::setTOS(unsigned char type_of_service)
	{
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && __FreeBSD_version < 600020) || defined(Q_OS_NETBSD) || defined(Q_OS_OPENBSD) || defined(Q_OS_BSD4)
		unsigned int c = type_of_service;
#else
		unsigned char c = type_of_service;
#endif
		if (setsockopt(m_fd,IPPROTO_IP,IP_TOS,&c,sizeof(c)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set TOS to %1 : %2")
					.arg(type_of_service).arg(strerror(errno)) << endl;
			return false;
		}
		return true;
	}
	
	Uint32 Socket::bytesAvailable() const
	{
		int ret = 0;
		if (ioctl(m_fd,FIONREAD,&ret) < 0)
			return 0;
		
		return ret;
	}
	
	bool Socket::connectSuccesFull()
	{
		if (m_state != CONNECTING)
			return false;
		
		int err = 0;
		socklen_t len = sizeof(int);
		if (getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&err,&len) < 0)
			return false;
		
		if (err == 0)
		{
			m_state = CONNECTED;
			cacheAddress();
		}
		
		return err == 0;
	}
	
	void Socket::cacheAddress()
	{
		struct sockaddr_in raddr;
		socklen_t slen = sizeof(struct sockaddr_in);
		if (getpeername(m_fd,(struct sockaddr*)&raddr,&slen) == 0)
			addr = Address(inet_ntoa(raddr.sin_addr),ntohs(raddr.sin_port));
	}

	/*
	void Socket::setReadBufferSize(int rbs)
	{
		if (setsockopt(m_fd, SOL_SOCKET, SO_RCVBUF, (char *)&rbs,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_DEBUG) << "Failed to set read buffer size " << endl;
		}
	}
	*/
}
