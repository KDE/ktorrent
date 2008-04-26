/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "socks.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <util/log.h>
#include <util/constants.h>
#include <mse/streamsocket.h>

using namespace bt;

namespace net 
{
	
	bool Socks::socks_enabled = false;
	int Socks::socks_version = 4;
	QString Socks::socks_server_host;
	bt::Uint16 Socks::socks_server_port = 1080;
	QString Socks::socks_username;
	QString Socks::socks_password;
	
	static Address socks_server_addr_v4;
	static Address socks_server_addr_v6;
	static bool socks_server_addr_resolved = false;

	Socks::Socks(mse::StreamSocket* sock,const Address & dest) : sock(sock),dest(dest),state(IDLE),internal_state(NONE)
	{
		version = socks_version; // copy version in case it changes
	}
	
	
	Socks::~Socks()
	{
	}
	
	void Socks::setSocksAuthentication(const QString & username,const QString & password)
	{
		socks_username = username;
		socks_password = password;
	}
	
	SocksResolver::SocksResolver(const QString & host,bt::Uint16 port)
	{
		KNetwork::KResolver::resolveAsync(this, SLOT(hostResolved(KNetwork::KResolverResults)),host, QString::number(port));
	}
	
	SocksResolver::~SocksResolver()
	{}
		
	void SocksResolver::hostResolved(KNetwork::KResolverResults res)
	{
		if (res.count() > 0)
		{
			foreach (const KNetwork::KResolverEntry &e,res)
			{
				net::Address addr = net::Address(e.address().asInet());
				if (addr.ipVersion() == 4)
					socks_server_addr_v4 = addr;
				else if (addr.ipVersion() == 6)
					socks_server_addr_v6 = addr;
			}
			socks_server_addr_resolved = true;
			//Out(SYS_CON|LOG_DEBUG) << "SocksResolver::hostResolved addr = " << socks_server_addr_v4.toString() << endl;
		}
		deleteLater(); // commit suicide
	}
	
	void Socks::setSocksServerAddress(const QString & host,bt::Uint16 port)
	{
		socks_server_addr_resolved = false;
		socks_server_host = host;
		socks_server_port = port;
		// resolve the address
		new SocksResolver(host,port);
		
	}

	Socks::State Socks::setup()
	{
		state = CONNECTING_TO_SERVER;
		if (!socks_server_addr_resolved)
		{
			state = FAILED;
			return state;
		}
		else if (sock->connectTo(dest.ipVersion() == 4 ? socks_server_addr_v4 : socks_server_addr_v6))
		{
			state = CONNECTING_TO_HOST;
			sock->setRemoteAddress(dest);
			return sendAuthRequest();
		}
		else if (sock->connecting())
		{
			return state;
		}
		else
		{
			state = FAILED;
			return state;
		}
	}
	
	Socks::State Socks::onReadyToWrite()
	{
		if (sock->connectSuccesFull())
		{
			state = CONNECTING_TO_HOST;
			sock->setRemoteAddress(dest);
			return sendAuthRequest();
		}
		else
		{
			state = FAILED;
		}
		return state;
	}
		
	Socks::State Socks::onReadyToRead()
	{
		if (state == CONNECTED)
			return state;
		
		if (sock->bytesAvailable() == 0)
		{
			state = FAILED;
			return state;
		}
		
		
		switch (internal_state)
		{
			case AUTH_REQUEST_SENT:
				return handleAuthReply();
				break;
			case USERNAME_AND_PASSWORD_SENT:
				return handleUsernamePasswordReply();
				break;
			case CONNECT_REQUEST_SENT:
				return handleConnectReply();
				break;
			default:
				return state;
		}
	}
	
	const Uint8 SOCKS_VERSION_4 = 4;
	const Uint8 SOCKS_VERSION_5 = 5;
	const Uint8 SOCKS_CMD_CONNECT = 1;
	const Uint8 SOCKS_CMD_BIND = 2;
	const Uint8 SOCKS_CMD_UDP_ASSOCIATE = 3;
	const Uint8 SOCKS_ADDR_TYPE_IPV4 = 1;
	const Uint8 SOCKS_ADDR_TYPE_DOMAIN = 3;
	const Uint8 SOCKS_ADDR_TYPE_IPV6 = 4;
	
	const Uint8 SOCKS_REPLY_OK = 0; //' succeeded
	const Uint8 SOCKS_REPLY_SERVER_FAILURE = 1; // general SOCKS server failure
	const Uint8 SOCKS_REPLY_NOT_ALLOWED = 2;  // connection not allowed by ruleset
	const Uint8 SOCKS_REPLY_NETWORK_UNREACHABLE = 3;
	const Uint8 SOCKS_REPLY_HOST_UNREACHABLE = 4;
	const Uint8 SOCKS_REPLY_CONNECTION_REFUSED = 5;
	const Uint8 SOCKS_REPLY_TTL_EXPIRED = 6;
	const Uint8 SOCKS_REPLY_CMD_NOT_SUPPORTED = 7;
	const Uint8 SOCKS_REPLY_ADDR_TYPE_NOT_SUPPORTED = 8;
	
	const Uint8 SOCKS_V4_REPLY_OK = 0x5a;
	const Uint8 SOCKS_V4_REPLY_FAILED = 0x5b;
	const Uint8 SOCKS_V4_REPLY_FAILED_2 = 0x5c;
	const Uint8 SOCKS_V4_REPLY_FAILED_3 = 0x5d;
	
	const Uint8 SOCKS_AUTH_METHOD_NONE = 0x00;
	const Uint8 SOCKS_AUTH_METHOD_GSSAPI = 0x01;
	const Uint8 SOCKS_AUTH_METHOD_USERNAME_PASSWORD = 0x02;
			
	
	struct SocksAuthRequest
	{
		Uint8 version;
		Uint8 nmethods;
		Uint8 methods[5];
		
		int size() const {return 2 + nmethods;}
	};
	
	struct SocksAuthReply
	{
		Uint8 version;
		Uint8 method;
	};
	
	struct SocksV4ConnectRequest
	{
		Uint8 version;
		Uint8 cmd;
		Uint16 port;
		Uint8 ip[4];
		char user_id[100];
		
		int size() const {return 8 + strlen(user_id) + 1;}
	};
	
	struct SocksV4ConnectReply
	{
		Uint8 null_byte;
		Uint8 reply;
		Uint8 dummy[6];
	};
	
	struct SocksConnectRequest
	{
		Uint8 version;
		Uint8 cmd;
		Uint8 reserved;
		Uint8 address_type;
		union 
		{
			struct 
			{
				Uint8 ip[4];
				Uint16 port;
			}ipv4;
			struct 
			{
				Uint8 ip[16];
				Uint16 port;
			}ipv6;
			/*struct 
			{
				Uint8 len;
				char domain_name[200];
			}domain;*/
		};
		
	};
	
	struct SocksConnectReply
	{
		Uint8 version;
		Uint8 reply;
		Uint8 reserved;
		Uint8 address_type;
		/*
		union 
		{
			Uint8 ip_v4[4];
			Uint8 ip_v6[16];
		};
		Uint16 port;
		*/
	};
	
	Socks::State Socks::sendAuthRequest()
	{
		if (version == 5)
		{
			SocksAuthRequest req;
			memset(&req,0,sizeof(SocksAuthRequest));
			req.version = SOCKS_VERSION_5;
			if (socks_username.length() > 0 && socks_password.length() > 0)
				req.nmethods = 2;
			else
				req.nmethods = 1;
			req.methods[0] = SOCKS_AUTH_METHOD_NONE; // No authentication
			req.methods[1] = SOCKS_AUTH_METHOD_USERNAME_PASSWORD; // Username and password
			req.methods[2] = SOCKS_AUTH_METHOD_GSSAPI; // GSSAPI
			sock->sendData((const Uint8*)&req,req.size());
			internal_state = AUTH_REQUEST_SENT;
		}
		else
		{
			if (dest.ipVersion() != 4)
			{
				Out(SYS_CON|LOG_IMPORTANT) << "SOCKSV4 does not suport IPv6" << endl;
				state = FAILED;
				return state;
			}
				
			// version 4 has no auth request
			SocksV4ConnectRequest req;
			memset(&req,0,sizeof(SocksV4ConnectRequest));
			req.version = SOCKS_VERSION_4;
			req.cmd = SOCKS_CMD_CONNECT;
			req.port = htons(dest.port());
			struct sockaddr_in* a = (struct sockaddr_in*)dest.address();
			memcpy(req.ip,&a->sin_addr,4);
			strcpy(req.user_id,"KTorrent");
			sock->sendData((const Uint8*)&req,req.size());
			internal_state = CONNECT_REQUEST_SENT;
			//Out(SYS_CON|LOG_DEBUG) << "SOCKSV4 send connect" << endl;
		}
		return state;
	}
	

	Socks::State Socks::handleAuthReply()
	{
		SocksAuthReply reply;
		if (sock->readData((Uint8*)&reply,sizeof(SocksAuthReply)) != sizeof(SocksAuthReply))
		{
			//Out(SYS_CON|LOG_DEBUG) << "sock->readData SocksAuthReply size not ok" << endl;
			state = FAILED;
			return state;
		}
		
		if (reply.version != SOCKS_VERSION_5 || reply.method == 0xFF)
		{
			//Out(SYS_CON|LOG_DEBUG) << "SocksAuthReply = " << reply.version << " " << reply.method << endl;
			state = FAILED;
			return state;
		}
		
		switch (reply.method)
		{
			case SOCKS_AUTH_METHOD_NONE:
				sendConnectRequest();
				break;
			case SOCKS_AUTH_METHOD_USERNAME_PASSWORD:
				sendUsernamePassword();
				break;
			case SOCKS_AUTH_METHOD_GSSAPI:
				break;
		}
		return state;
	}
	
	void Socks::sendUsernamePassword()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Socks: sending username and password " << endl;
		QByteArray user = socks_username.toLocal8Bit();
		QByteArray pwd = socks_password.toLocal8Bit();
		Uint32 off = 0;
		Uint8 buffer[3 + 2*256];
		buffer[off++] = 0x01; // version
		buffer[off++] = user.size();
		memcpy(buffer + off,user.constData(),user.size());
		off += user.size();
		buffer[off++] = pwd.size();
		memcpy(buffer + off,pwd.constData(),pwd.size());
		off += pwd.size();
		sock->sendData(buffer,off);
		internal_state = USERNAME_AND_PASSWORD_SENT;
	}
	
	Socks::State Socks::handleUsernamePasswordReply()
	{
	//	Out(SYS_CON|LOG_DEBUG) << "Socks: handleUsernamePasswordReply " << endl;
		Uint8 reply[2];
		if (sock->readData(reply,2) != 2)
		{
			//Out(SYS_CON|LOG_DEBUG) << "sock->readData UPWReply size not ok" << endl;
			state = FAILED;
			return state;
		}
		
		if (reply[0] != 1 || reply[1] != 0)
		{
			Out(SYS_CON|LOG_IMPORTANT) << "Socks: Wrong username or password !" << endl;
			state = FAILED;
			return state;
		}
		
		sendConnectRequest();
		return state;
	}
	
	void Socks::sendConnectRequest()
	{
		int len = 6;
		SocksConnectRequest req;
		memset(&req,0,sizeof(SocksConnectRequest));
		req.version = SOCKS_VERSION_5;
		req.cmd = SOCKS_CMD_CONNECT;
		req.address_type = dest.ipVersion() == 4 ? SOCKS_ADDR_TYPE_IPV4 : SOCKS_ADDR_TYPE_IPV6;
		if (dest.ipVersion() == 4)
		{
			struct sockaddr_in* a = (struct sockaddr_in*)dest.address();
			memcpy(req.ipv4.ip,&a->sin_addr,4);
			req.ipv4.port = a->sin_port;
			len += 4;
		}
		else
		{
			struct sockaddr_in6* a = (struct sockaddr_in6*)dest.address();
			memcpy(req.ipv6.ip,&a->sin6_addr,16);
			req.ipv6.port = a->sin6_port;
			len += 16;
		}
		sock->sendData((const Uint8*)&req,len);
		internal_state = CONNECT_REQUEST_SENT;
	}
	
	Socks::State Socks::handleConnectReply()
	{
		if (version == 4)
		{
			//Out(SYS_CON|LOG_DEBUG) << "SOCKSV4 handleConnectReply" << endl;
			SocksV4ConnectReply reply;
			if (sock->readData((Uint8*)&reply,sizeof(SocksV4ConnectReply)) != sizeof(SocksV4ConnectReply))
			{
			//	Out(SYS_CON|LOG_DEBUG) << "sock->readData SocksV4ConnectReply size not ok" << endl;
				state = FAILED;
				return state;
				
			}
			
			if (reply.reply != SOCKS_V4_REPLY_OK)
			{
			//	Out(SYS_CON|LOG_DEBUG) << "reply.reply != SOCKS_V4_REPLY_OK" << endl;
				state = FAILED;
				return state;
			}
			
			// Out(SYS_CON|LOG_DEBUG) << "SocksV4: connect OK ! " << endl;
			state = CONNECTED;
			return state;
		}
		
		SocksConnectReply reply;
		if (sock->readData((Uint8*)&reply,sizeof(SocksConnectReply)) != sizeof(SocksConnectReply))
		{
			//Out(SYS_CON|LOG_DEBUG) << "sock->readData SocksConnectReply size not ok" << endl;
			state = FAILED;
			return state;
		}
		
		if (reply.version != SOCKS_VERSION_5 || reply.reply != SOCKS_REPLY_OK)
		{
			//Out(SYS_CON|LOG_DEBUG) << "SocksConnectReply : " << reply.version << " " << reply.reply << " " << reply.address_type << endl;
			state = FAILED;
			return state;
		}
		
		Uint32 ba = sock->bytesAvailable();
		if (reply.address_type == SOCKS_ADDR_TYPE_IPV4)
		{
			Uint8 addr[6]; // IP and port
			if (ba < 6 || sock->readData(addr,6) != 6)
			{
				//Out(SYS_CON|LOG_DEBUG) << "Failed to read IPv4 address : " << endl;
				state = FAILED;
				return state;
			}
			else
			{
				//Out(SYS_CON|LOG_DEBUG) << "Socks: connect OK ! " << endl;
				state = CONNECTED;
				return state;
			}
		}
		else if (reply.address_type == SOCKS_ADDR_TYPE_IPV6)
		{
			Uint8 addr[18]; // IP and port
			if (ba < 18 || sock->readData(addr,6) != 6)
			{
				//Out(SYS_CON|LOG_DEBUG) << "Failed to read IPv4 address : " << endl;
				state = FAILED;
				return state;
			}
			else
			{
				//Out(SYS_CON|LOG_DEBUG) << "Socks: connect OK ! " << endl;
				state = CONNECTED;
				return state;
			}
		}
		else if (reply.address_type == SOCKS_ADDR_TYPE_DOMAIN)
		{
			Uint8 len = 0;
			if (ba == 0 || sock->readData(&len,1) != 1)
			{
				//Out(SYS_CON|LOG_DEBUG) << "Failed to read domain name length " << endl;
				state = FAILED;
				return state;
			}
			ba = sock->bytesAvailable();
			Uint8 tmp[256];
			if (ba < len || sock->readData(tmp,len) != len)
			{
				//Out(SYS_CON|LOG_DEBUG) << "Failed to read domain name" << endl;
				state = FAILED;
				return state;
			}
			else
			{
				//Out(SYS_CON|LOG_DEBUG) << "Socks: connect OK ! " << endl;
				state = CONNECTED;
				return state;
			}
		}
		else
		{
			//Out(SYS_CON|LOG_DEBUG) << "Invalid address type : " << reply.address_type << endl;
			state = FAILED;
			return state;
		}
	}
}

#include "socks.moc"
