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
#include "socket.h"
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
#include <netdb.h>

#if defined(Q_OS_LINUX) && !defined(__FreeBSD_kernel__)
#include <asm/ioctls.h>
#endif

#ifdef Q_OS_SOLARIS
#include <sys/filio.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#include <fcntl.h>

#include <util/log.h>

#ifdef Q_WS_WIN
#include <util/win32.h>
#define SHUT_RDWR SD_BOTH
#undef errno
#define errno WSAGetLastError()
#endif
#include <kdebug.h>
using namespace bt;

namespace net
{

	Socket::Socket(int fd,int ip_version) : m_fd(fd),m_ip_version(ip_version),r_poll_index(-1),w_poll_index(-1)
	{
		// check if the IP version is 4 or 6
		if (m_ip_version != 4 && m_ip_version != 6)
			m_ip_version = 4;
		
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && __FreeBSD_version < 600020 && !defined(__DragonFly__))
		int val = 1; 
		if (setsockopt(m_fd,SOL_SOCKET,SO_NOSIGPIPE,&val,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the NOSIGPIPE option : %1").arg(strerror(errno)) << endl;
		}
#endif
		cacheAddress();
	}
	
	Socket::Socket(bool tcp,int ip_version) : m_fd(-1),m_ip_version(ip_version),r_poll_index(-1),w_poll_index(-1)
	{
		// check if the IP version is 4 or 6
		if (m_ip_version != 4 && m_ip_version != 6)
			m_ip_version = 4;
		
		int fd = socket(m_ip_version == 4 ? PF_INET : PF_INET6,tcp ? SOCK_STREAM : SOCK_DGRAM,0);
		if (fd < 0)
			Out(SYS_GEN|LOG_IMPORTANT) << QString("Cannot create socket : %1").arg(strerror(errno)) << endl;
		m_fd = fd;
		
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && __FreeBSD_version < 600020 && !defined(__DragonFly__))
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
#ifdef Q_WS_WIN
			::closesocket(m_fd);
#else
			::close(m_fd);
#endif
		}
	}
	
	void Socket::reset()
	{
		close();
		int fd = socket(m_ip_version == 4 ? PF_INET : PF_INET6,SOCK_STREAM,0);
		if (fd < 0)
			Out(SYS_GEN|LOG_IMPORTANT) << QString("Cannot create socket : %1").arg(strerror(errno)) << endl;
		m_fd = fd;
		
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && __FreeBSD_version < 600020 && !defined(__DragonFly__))
		int val = 1;
		if (setsockopt(m_fd,SOL_SOCKET,SO_NOSIGPIPE,&val,sizeof(int)) < 0)
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the NOSIGPIPE option : %1").arg(strerror(errno)) << endl;
		}
#endif	
		m_state = IDLE;
	}
	
	void Socket::close()
	{
		if (m_fd >= 0)
		{
			shutdown(m_fd, SHUT_RDWR);
#ifdef Q_WS_WIN
			::closesocket(m_fd);
#else
			::close(m_fd);
#endif
			m_fd = -1;
			m_state = CLOSED;
		}
	}
	
	void Socket::setBlocking(bool on)
	{
#ifndef Q_WS_WIN
		int flag = fcntl(m_fd, F_GETFL, 0);
		if (!on)
			fcntl(m_fd, F_SETFL, flag | O_NONBLOCK);
		else
			fcntl(m_fd, F_SETFL, flag & ~O_NONBLOCK);
#else
		u_long b = on ? 1 : 0;
		ioctlsocket(m_fd, FIONBIO, &b);
#endif
	}
		
	bool Socket::connectTo(const Address & a)
	{
		if (::connect(m_fd,a.address(),a.length()) < 0)
		{
#ifndef Q_WS_WIN
			if (errno == EINPROGRESS)
#else
			if (errno == WSAEINVAL || errno == WSAEALREADY || errno == WSAEWOULDBLOCK)
#endif
			{
			//	Out(SYS_CON|LOG_DEBUG) << "Socket is connecting" << endl;
				m_state = CONNECTING;
				return false;
			}
			else
			{
				Out(SYS_CON|LOG_NOTICE) << QString("Cannot connect to host %1 : %2")
					.arg(a.toString()).arg(QString::fromLocal8Bit(strerror(errno))) << endl;
				return false;
			}
		}
		m_state = CONNECTED;
		cacheAddress();
		return true;
	}
	
	bool Socket::bind(const QString & ip,Uint16 port,bool also_listen)
	{
		int val = 1;
#ifndef Q_WS_WIN
		if (setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(int)) < 0)
#else
		if (setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&val,sizeof(int)) < 0)
#endif 
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the reuseaddr option : %1").arg(strerror(errno)) << endl;
		}

		net::Address addr(ip,port);
		if (::bind(m_fd,addr.address(),addr.length()) != 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot bind to port %1:%2 : %3").arg(ip).arg(port).arg(strerror(errno)) << endl;
			return false;
		}

		if (also_listen && listen(m_fd,SOMAXCONN) < 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot listen to port %1:%2 : %3").arg(ip).arg(port).arg(strerror(errno)) << endl;
			return false;
		}

		m_state = BOUND;
		return true;
	}
	
	bool Socket::bind(const net::Address& addr, bool also_listen)
	{
		int val = 1;
#ifndef Q_WS_WIN
		if (setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(int)) < 0)
#else
		if (setsockopt(m_fd,SOL_SOCKET,SO_REUSEADDR,(char *)&val,sizeof(int)) < 0)
#endif 
		{
			Out(SYS_CON|LOG_NOTICE) << QString("Failed to set the reuseaddr option : %1").arg(strerror(errno)) << endl;
		}
		
		if (::bind(m_fd,addr.address(),addr.length()) != 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot bind to port %1:%2 : %3").arg(addr.ipAddress().toString()).arg(addr.port()).arg(strerror(errno)) << endl;
			return false;
		}
		
		if (also_listen && listen(m_fd,5) < 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << QString("Cannot listen to port %1:%2 : %3").arg(addr.ipAddress().toString()).arg(addr.port()).arg(strerror(errno)) << endl;
			return false;
		}
		
		m_state = BOUND;
		return true;
	}

	int Socket::send(const bt::Uint8* buf,int len)
	{
#ifndef Q_WS_WIN        
		int ret = ::send(m_fd,buf,len,MSG_NOSIGNAL);
#else
		int ret = ::send(m_fd,(char *)buf,len,MSG_NOSIGNAL);
#endif
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
#ifndef Q_WS_WIN
		int ret = ::recv(m_fd,buf,max_len,0);
#else
		int ret = ::recv(m_fd,(char *)buf,max_len,0);
#endif
		if (ret < 0)
		{
			if (errno != EAGAIN && errno != EWOULDBLOCK)
			{
			//	Out(SYS_CON|LOG_DEBUG) << "Receive error : " << QString(strerror(errno)) << endl;
				close();
				return 0;
			}
			
			return ret;
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
		int ret = ::sendto(m_fd,(char*)buf,len,0,a.address(),a.length());
		if (ret < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return SEND_WOULD_BLOCK;
			
			Out(SYS_CON|LOG_DEBUG) << "Send error : " << QString(strerror(errno)) << endl;
			return SEND_FAILURE;
		}
		
		return ret;
	}
	
	int Socket::recvFrom(bt::Uint8* buf,int max_len,Address & a)
	{
		struct sockaddr_storage ss;
		socklen_t slen = sizeof(ss);
#ifndef Q_WS_WIN
		int ret = ::recvfrom(m_fd,buf,max_len,0,(struct sockaddr*)&ss,&slen);
#else
		int ret = ::recvfrom(m_fd,(char *)buf,max_len,0,(struct sockaddr*)&ss,&slen);
#endif
		if (ret < 0)
		{
			Out(SYS_CON|LOG_DEBUG) << "Receive error : " << QString(strerror(errno)) << endl;
			return 0;
		}
		a = KNetwork::KInetSocketAddress((const struct sockaddr*) &ss,slen);
		return ret;
	}
	
	int Socket::accept(Address & a)
	{
		struct sockaddr_storage ss;
		socklen_t slen = sizeof(ss);
		int sfd = ::accept(m_fd,(struct sockaddr*)&ss,&slen);
			
		if (sfd < 0)
		{
			Out(SYS_CON|LOG_DEBUG) << "Accept error : " << QString(strerror(errno)) << endl;
			return -1;
		}
		a = KNetwork::KInetSocketAddress((const struct sockaddr*) &ss,slen);
		
		Out(SYS_CON|LOG_DEBUG) << "Accepted connection from " << a.toString() << endl;
		return sfd;
	}
	
	bool Socket::setTOS(unsigned char type_of_service)
	{
		// If type of service is 0, do nothing
		if (type_of_service == 0)
			return true;

		if (m_ip_version == 4)
		{
#if defined(Q_OS_MACX) || defined(Q_OS_DARWIN) || (defined(Q_OS_FREEBSD) && __FreeBSD_version < 600020) || defined(Q_OS_NETBSD) || defined(Q_OS_BSD4)
			unsigned int c = type_of_service;
#else
			unsigned char c = type_of_service;
#endif
#ifndef Q_WS_WIN
			if (setsockopt(m_fd,IPPROTO_IP,IP_TOS,&c,sizeof(c)) < 0)
#else
			if (setsockopt(m_fd,IPPROTO_IP,IP_TOS,(char *)&c,sizeof(c)) < 0)
#endif
			{
				Out(SYS_CON|LOG_NOTICE) << QString("Failed to set TOS to %1 : %2")
						.arg((int)type_of_service).arg(strerror(errno)) << endl;
				return false;
			}
		}
		else
		{
#ifdef __GNUC__
#warning "Find way to set IPv6 traffic class"
#endif
			return true;
		}
		return true;
	}
	
	Uint32 Socket::bytesAvailable() const
	{
		int ret = 0;
#ifndef Q_WS_WIN		
		if (ioctl(m_fd,FIONREAD,&ret) < 0)
#else
		if (ioctlsocket(m_fd,FIONREAD,(u_long*)&ret) < 0)
#endif
			return 0;
		
		return ret;
	}
	
	bool Socket::connectSuccesFull()
	{
		if (m_state != CONNECTING)
			return false;
		
		int err = 0;
		socklen_t len = sizeof(int);
#ifndef Q_WS_WIN
		if (getsockopt(m_fd,SOL_SOCKET,SO_ERROR,&err,&len) < 0)
#else
		if (getsockopt(m_fd,SOL_SOCKET,SO_ERROR,(char *)&err,&len) < 0)
#endif
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
		struct sockaddr_storage ss;           /* Where the peer adr goes. */
		socklen_t sslen = sizeof(ss);
		
		if (getpeername(m_fd,(struct sockaddr*)&ss,&sslen) == 0)
		{
			// If it is a IPv6 mapped address convert to IPv4
			KNetwork::KInetSocketAddress tmp((struct sockaddr*)&ss,sslen);
			if (tmp.ipVersion() == 6 && tmp.ipAddress().isV4Mapped())
				tmp.setHost(KNetwork::KIpAddress(tmp.ipAddress().IPv4Addr(true)));
			addr = tmp;
		}
	}

	Address Socket::getSockName() const
	{
		struct sockaddr_storage ss;           /* Where the peer adr goes. */
		socklen_t sslen = sizeof(ss);
		
		if (getsockname(m_fd,(struct sockaddr*)&ss,&sslen) == 0)
		{
			// If it is a IPv6 mapped address convert to IPv4
			KNetwork::KInetSocketAddress tmp((struct sockaddr*)&ss,sslen);
			if (tmp.ipVersion() == 6 && tmp.ipAddress().isV4Mapped())
				tmp.setHost(KNetwork::KIpAddress(tmp.ipAddress().IPv4Addr(true)));
			return tmp;
		}

		return Address();
	}

	int Socket::take()
	{
		int ret = m_fd;
		m_fd = -1;
		return ret;
	}
	
	
	void Socket::prepare(Poll* p,Poll::Mode mode)
	{
		if (mode == Poll::OUTPUT)
			w_poll_index = p->add(m_fd,mode);
		else
			r_poll_index = p->add(m_fd,mode);
	}

	bool Socket::ready(const Poll* p,Poll::Mode mode) const
	{
		return p->ready(mode == Poll::OUTPUT ? w_poll_index : r_poll_index,mode);
	}

}
