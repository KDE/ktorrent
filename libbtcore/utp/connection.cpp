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

#include "connection.h"
#include <time.h>
#include <QFile>
#include <QEvent>
#include <QTextStream>
#include <QThread>
#include <util/sha1hash.h>
#include <util/log.h>
#include <util/functions.h>
#include "localwindow.h"
#include "remotewindow.h"
#include "delaywindow.h"



using namespace bt;

namespace utp
{

	Connection::TransmissionError::TransmissionError(const char* file, int line)
	{
		location = QString("TransmissionError in %1 at line %2\n").arg(file).arg(line);
	}
	
	Connection::Connection(bt::Uint16 recv_connection_id, Type type, const net::Address& remote, Transmitter* transmitter) 
		: transmitter(transmitter)
	{
		stats.type = type;
		stats.remote = remote;
		stats.recv_connection_id = recv_connection_id;
		stats.reply_micro = 0;
		stats.eof_seq_nr = -1;
		local_wnd = new LocalWindow(128*1024);
		remote_wnd = new RemoteWindow();
		delay_window = new DelayWindow();
		fin_sent = false;
		stats.rtt = 100;
		stats.rtt_var = 0;
		stats.timeout = 1000;
		stats.packet_size = 1500 - IP_AND_UDP_OVERHEAD - sizeof(utp::Header);
		stats.last_window_size_transmitted = 128*1024;
		if (type == OUTGOING)
		{
			stats.send_connection_id = recv_connection_id + 1;
		}
		else
		{
			stats.send_connection_id = recv_connection_id - 1;
			stats.state = CS_IDLE;
			stats.seq_nr = 5;
		}
		
		stats.bytes_received = 0;
		stats.bytes_sent = 0;
		stats.packets_received = 0;
		stats.packets_sent = 0;
		stats.bytes_lost = 0;
		stats.packets_lost = 0;
		
		connect(this,SIGNAL(doDelayedStartTimer()),this,SLOT(delayedStartTimer()),Qt::QueuedConnection);
		startTimer();
		Out(SYS_UTP|LOG_NOTICE) << "UTP: Connection " << recv_connection_id << "|" << stats.send_connection_id << endl;
	}

	Connection::~Connection()
	{
		delete local_wnd;
		delete remote_wnd;
		delete delay_window;
	}
	
	void Connection::startConnecting()
	{
		if (stats.type == OUTGOING)
			sendSYN();
	}

#if 0
	static void DumpPacket(const Header & hdr,const SelectiveAck* sack)
	{
		Out(SYS_UTP|LOG_NOTICE) << "==============================================" << endl;
		Out(SYS_UTP|LOG_NOTICE) << "UTP: Packet Header: " << endl;
		Out(SYS_UTP|LOG_NOTICE) << "type:                              " << TypeToString(hdr.type) << endl;
		Out(SYS_UTP|LOG_NOTICE) << "version:                           " << hdr.version << endl;
		Out(SYS_UTP|LOG_NOTICE) << "extension:                         " << hdr.extension << endl;
		Out(SYS_UTP|LOG_NOTICE) << "connection_id:                     " << hdr.connection_id << endl;
		Out(SYS_UTP|LOG_NOTICE) << "timestamp_microseconds:            " << hdr.timestamp_microseconds << endl;
		Out(SYS_UTP|LOG_NOTICE) << "timestamp_difference_microseconds: " << hdr.timestamp_difference_microseconds << endl;
		Out(SYS_UTP|LOG_NOTICE) << "wnd_size:                          " << hdr.wnd_size << endl;
		Out(SYS_UTP|LOG_NOTICE) << "seq_nr:                            " << hdr.seq_nr << endl;
		Out(SYS_UTP|LOG_NOTICE) << "ack_nr:                            " << hdr.ack_nr << endl;
		Out(SYS_UTP|LOG_NOTICE) << "==============================================" << endl;
		if (!sack)
			return;
		
		Out(SYS_UTP|LOG_NOTICE) << "SelectiveAck:                      " << endl;
		Out(SYS_UTP|LOG_NOTICE) << "extension:                         " << sack->extension << endl;
		Out(SYS_UTP|LOG_NOTICE) << "length:                            " << sack->length << endl;
		Out(SYS_UTP|LOG_NOTICE) << "bitmask:                           " << hex(sack->bitmask[0]) << endl;
		Out(SYS_UTP|LOG_NOTICE) << "bitmask:                           " << hex(sack->bitmask[1]) << endl;
		Out(SYS_UTP|LOG_NOTICE) << "bitmask:                           " << hex(sack->bitmask[2]) << endl;
		Out(SYS_UTP|LOG_NOTICE) << "bitmask:                           " << hex(sack->bitmask[3]) << endl;
		Out(SYS_UTP|LOG_NOTICE) << "==============================================" << endl;
	}
#endif

	ConnectionState Connection::handlePacket(const PacketParser & parser,const QByteArray& packet)
	{
		QMutexLocker lock(&mutex);
		stats.packets_received++;
		
		const Header * hdr = parser.header();
		const SelectiveAck* sack = parser.selectiveAck();
		int data_off = parser.dataOffset();
		
		//DumpPacket(*hdr,sack);
		
		updateDelayMeasurement(hdr);
		remote_wnd->packetReceived(hdr,sack,this);
		switch (stats.state)
		{
			case CS_SYN_SENT:
				// now we should have a state packet
				if (hdr->type == ST_STATE)
				{
					// connection estabished
					stats.state = CS_CONNECTED;
					local_wnd->setLastSeqNr(hdr->seq_nr - 1);
					connected.wakeAll();
					stats.timeout = 1000;
					Out(SYS_UTP|LOG_NOTICE) << "UTP: established connection with " << stats.remote.toString() << endl;
				}
				else
				{
					sendReset();
					stats.state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_IDLE:
				if (hdr->type == ST_SYN)
				{
					// Send back a state packet
					local_wnd->setLastSeqNr(hdr->seq_nr);
					sendState();
					stats.state = CS_CONNECTED;
					stats.timeout = 1000;
					Out(SYS_UTP|LOG_NOTICE) << "UTP: established connection with " << stats.remote.toString() << endl;
				}
				else
				{
					sendReset();
					stats.state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_CONNECTED:
				if (hdr->type == ST_DATA)
				{
					// push data into local window
					int s = packet.size() - data_off;
					local_wnd->packetReceived(hdr,(const bt::Uint8*)packet.data() + data_off,s);
					if (local_wnd->size() > 0)
						data_ready.wakeAll();
					
					// send back an ACK 
					sendStateOrData();
				}
				else if (hdr->type == ST_STATE)
				{
					// try to send more data packets
					sendPackets();
				}
				else if (hdr->type == ST_FIN)
				{
					stats.eof_seq_nr = hdr->seq_nr;
					// other side now has closed the connection
					stats.state = CS_FINISHED; // state becomes finished
					sendPackets();
					checkIfClosed();
				}
				else
				{
					sendReset();
					stats.state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_FINISHED:
				if (hdr->type == ST_DATA)
				{
					if (hdr->seq_nr <= stats.eof_seq_nr)
					{
						// push data into local window
						int s = packet.size() - data_off;
						local_wnd->packetReceived(hdr,(const bt::Uint8*)packet.data() + data_off,s);
						if (local_wnd->size() > 0)
							data_ready.wakeAll();
					}
					
					// send back an ACK 
					sendStateOrData();
					if (stats.state == CS_FINISHED && !fin_sent && output_buffer.size() == 0)
					{
						sendFIN();
						fin_sent = true;
					}
					checkIfClosed();
				}
				else if (hdr->type == ST_STATE)
				{
					// try to send more data packets
					sendPackets();
					checkIfClosed();
				}
				else if (hdr->type == ST_FIN)
				{
					stats.eof_seq_nr = hdr->seq_nr;
					sendPackets();
					checkIfClosed();
				}
				else
				{
					sendReset();
					stats.state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_CLOSED:
				break;
		}
		
		startTimer();
		return stats.state;
	}
	
	void Connection::checkIfClosed()
	{
		// Check if we need to go to the closed state
		// We can do this if all our packets have been acked and the local window
		// has been fully read
		if (remote_wnd->allPacketsAcked() && local_wnd->isEmpty())
		{
			stats.state = CS_CLOSED;
			Out(SYS_UTP|LOG_NOTICE) << "UTP: Connection " << stats.recv_connection_id << "|" << stats.send_connection_id << " closed " << endl;
			data_ready.wakeAll();
		}
	}
	
	void Connection::updateRTT(const utp::Header* hdr,bt::Uint32 packet_rtt,bt::Uint32 packet_size)
	{
		Q_UNUSED(hdr);
		int delta = stats.rtt - (int)packet_rtt;
		stats.rtt_var += (qAbs(delta) - stats.rtt_var) / 4;
		stats.rtt += ((int)packet_rtt - stats.rtt) / 8;
		stats.timeout = qMax(stats.rtt + stats.rtt_var * 4, 500);
		stats.bytes_sent += packet_size;
		startTimer();
	}

	
	void Connection::sendPacket(Uint32 type,Uint16 p_ack_nr)
	{
		bt::Uint32 extension_length = 0;
		bt::Uint32 sack_bits = local_wnd->selectiveAckBits();
		if (sack_bits > 0)
			extension_length += 2 + qMax(sack_bits / 8,(bt::Uint32)4);
	
		TimeValue tv;
		QByteArray ba(Header::size() + extension_length,0);
		Header hdr;
		hdr.version = 1;
		hdr.type = type;
		hdr.extension = extension_length == 0 ? 0 : SELECTIVE_ACK_ID;
		hdr.connection_id = type == ST_SYN ? stats.recv_connection_id : stats.send_connection_id;
		hdr.timestamp_microseconds = tv.timestampMicroSeconds();
		hdr.timestamp_difference_microseconds = stats.reply_micro;
		hdr.wnd_size = stats.last_window_size_transmitted = local_wnd->availableSpace();
		hdr.seq_nr = stats.seq_nr;
		hdr.ack_nr = p_ack_nr;
		hdr.write((bt::Uint8*)ba.data());
		
		if (extension_length > 0)
		{
			bt::Uint8* ptr = (bt::Uint8*)(ba.data() + Header::size());
			SelectiveAck sack;
			sack.extension = ptr[0] = 0;
			sack.length = ptr[1] = extension_length - 2;
			sack.bitmask = ptr + 2;
			local_wnd->fillSelectiveAck(&sack);
		}
		
		
		if (!transmitter->sendTo(ba,stats.remote,receiveConnectionID()))
			throw TransmissionError(__FILE__,__LINE__);
		
		last_packet_sent = tv;
		stats.packets_sent++;
		startTimer();
	}


	void Connection::sendSYN()
	{
		stats.seq_nr = 1;
		stats.state = CS_SYN_SENT;
		stats.timeout = CONNECT_TIMEOUT;
		sendPacket(ST_SYN,0);
		stats.seq_nr++;
		startTimer();
	}
	
	void Connection::sendState()
	{
		sendPacket(ST_STATE,local_wnd->lastSeqNr());
	}

	void Connection::sendFIN()
	{
		sendPacket(ST_FIN,local_wnd->lastSeqNr());
	}

	void Connection::sendReset()
	{
		sendPacket(ST_RESET,local_wnd->lastSeqNr());
	}

	void Connection::updateDelayMeasurement(const utp::Header* hdr)
	{
		TimeValue now;
		bt::Uint32 tms = now.timestampMicroSeconds();
		if (tms > hdr->timestamp_microseconds)
			stats.reply_micro = tms - hdr->timestamp_microseconds;
		else
			stats.reply_micro = hdr->timestamp_difference_microseconds - tms;
		
		bt::Uint32 base_delay = delay_window->update(hdr,now.toTimeStamp());
		
		int our_delay = hdr->timestamp_difference_microseconds / 1000 - base_delay;
		int off_target = CCONTROL_TARGET - our_delay;
		double delay_factor = (double)off_target / (double)CCONTROL_TARGET;
		double window_factor = remote_wnd->windowUsageFactor();
		double scaled_gain = MAX_CWND_INCREASE_PACKETS_PER_RTT * delay_factor * window_factor;
		
		remote_wnd->updateWindowSize(scaled_gain);
		if (remote_wnd->maxWindow() <= MIN_PACKET_SIZE)
			stats.packet_size = MIN_PACKET_SIZE;
		else if (remote_wnd->maxWindow() <= 1000)
			stats.packet_size = 500;
		else if (remote_wnd->maxWindow() <= 5000)
			stats.packet_size = 1000;
		else
			stats.packet_size = 1500 - IP_AND_UDP_OVERHEAD - sizeof(utp::Header);
	
		/*
		Out(SYS_UTP|LOG_DEBUG) << "base_delay " << base_delay << endl;
		Out(SYS_UTP|LOG_DEBUG) << "our_delay " << our_delay << endl;
		Out(SYS_UTP|LOG_DEBUG) << "off_target " << off_target << endl;
		Out(SYS_UTP|LOG_DEBUG) << "delay_factor " << delay_factor << endl;
		Out(SYS_UTP|LOG_DEBUG) << "window_factor " << window_factor << endl;
		Out(SYS_UTP|LOG_DEBUG) << "scaled_gain " << scaled_gain << endl; 
		Out(SYS_UTP|LOG_DEBUG) << "packet_size " << stats.packet_size << endl;
		*/
	}
	
	
	

	int Connection::send(const bt::Uint8* data, Uint32 len)
	{
		QMutexLocker lock(&mutex);
		if (stats.state != CS_CONNECTED)
			return -1;
		
		// first put data in the output buffer then send packets
		bt::Uint32 ret = output_buffer.write(data,len);
		sendPackets();
		return ret;
	}
	
	void Connection::sendPackets()
	{
		// chop output_buffer data in packets and keep sending
		// until we are no longer allowed or the buffer is empty
		while (output_buffer.size() > 0 && remote_wnd->availableSpace() > 0)
		{
			bt::Uint32 to_read = qMin((bt::Uint32)output_buffer.size(),remote_wnd->availableSpace());
			to_read = qMin(to_read,stats.packet_size);
			if (to_read == 0)
				break;
			
			QByteArray packet(to_read,0);
			if (output_buffer.read((bt::Uint8*)packet.data(),to_read) != to_read)
				Out(SYS_UTP|LOG_DEBUG) << "Output buffer read failed " << endl;
		
			sendDataPacket(packet);
		}
		
		if (stats.state == CS_FINISHED && !fin_sent && output_buffer.size() == 0)
		{
			sendFIN();
			fin_sent = true;
		}
	}

	void Connection::sendStateOrData()
	{
		if (output_buffer.size() > 0 && remote_wnd->availableSpace() > 0)
			sendPackets();
		else
			sendState();
	}

	int Connection::sendDataPacket(const QByteArray& packet)
	{
		bt::Uint32 to_send = packet.size();
		TimeValue now;
		
		bt::Uint32 extension_length = 0;
		bt::Uint32 sack_bits = local_wnd->selectiveAckBits();
		if (sack_bits > 0)
			extension_length += 2 + qMin(sack_bits / 8,(bt::Uint32)4);
		
		QByteArray ba(Header::size() + extension_length + packet.size(),0);
		Header hdr;
		hdr.version = 1;
		hdr.type = ST_DATA;
		hdr.extension = extension_length == 0 ? 0 : SELECTIVE_ACK_ID;
		hdr.connection_id = stats.send_connection_id;
		hdr.timestamp_microseconds = now.timestampMicroSeconds();
		hdr.timestamp_difference_microseconds = stats.reply_micro;
		hdr.wnd_size = stats.last_window_size_transmitted = local_wnd->availableSpace();
		hdr.seq_nr = stats.seq_nr;
		hdr.ack_nr = local_wnd->lastSeqNr();
		hdr.write((bt::Uint8*)ba.data());
		
		if (extension_length > 0)
		{
			bt::Uint8* ptr = (bt::Uint8*)(ba.data() + Header::size());
			SelectiveAck sack;
			sack.extension = ptr[0] = 0;
			sack.length = ptr[1] = extension_length - 2;
			sack.bitmask = ptr + 2;
			local_wnd->fillSelectiveAck(&sack);
		}
		
		memcpy(ba.data() + Header::size() + extension_length,packet.data(),to_send);
		if (!transmitter->sendTo(ba,stats.remote,receiveConnectionID()))
			throw TransmissionError(__FILE__,__LINE__);
		
		last_packet_sent = now;
		stats.packets_sent++;
		remote_wnd->addPacket(packet,stats.seq_nr,bt::Now());
		stats.seq_nr++;
		startTimer();
		return to_send;
	}

	void Connection::retransmit(const QByteArray& packet, Uint16 p_seq_nr)
	{
		TimeValue now;
		
		bt::Uint32 extension_length = 0;
		bt::Uint32 sack_bits = local_wnd->selectiveAckBits();
		if (sack_bits > 0)
			extension_length += 2 + qMin(sack_bits / 8,(bt::Uint32)4);
		
		QByteArray ba(Header::size() + extension_length + packet.size(),0);
		Header hdr;
		hdr.version = 1;
		hdr.type = ST_DATA;
		hdr.extension = extension_length == 0 ? 0 : SELECTIVE_ACK_ID;
		hdr.connection_id = stats.send_connection_id;
		hdr.timestamp_microseconds = now.timestampMicroSeconds();
		hdr.timestamp_difference_microseconds = stats.reply_micro;
		hdr.wnd_size = stats.last_window_size_transmitted = local_wnd->availableSpace();
		hdr.seq_nr = p_seq_nr;
		hdr.ack_nr = local_wnd->lastSeqNr();
		hdr.write((bt::Uint8*)ba.data());
		
		if (extension_length > 0)
		{
			bt::Uint8* ptr = (bt::Uint8*)(ba.data() + Header::size());
			SelectiveAck sack;
			sack.extension = ptr[0] = 0;
			sack.length = ptr[1] = extension_length - 2;
			sack.bitmask = ptr + 2;
			local_wnd->fillSelectiveAck(&sack);
		}
		
		memcpy(ba.data() + Header::size() + extension_length,packet.data(),packet.size());
		startTimer();
		if (!transmitter->sendTo(ba,stats.remote,receiveConnectionID()))
			throw TransmissionError(__FILE__,__LINE__);
		
		last_packet_sent = now;
		stats.packets_sent++;
	}

	bt::Uint32 Connection::bytesAvailable() const
	{
		QMutexLocker lock(&mutex);
		return local_wnd->size();
	}

	bool Connection::isWriteable() const
	{
		QMutexLocker lock(&mutex);
		return remote_wnd->availableSpace() > 0 && stats.state == CS_CONNECTED;
	}

	int Connection::recv(Uint8* buf, Uint32 max_len)
	{
		QMutexLocker lock(&mutex);
		if (stats.state == CS_FINISHED)
			checkIfClosed();
		
		if (local_wnd->size() == 0 && stats.state == CS_CLOSED)
			return -1;
		
		bt::Uint32 ret = local_wnd->read(buf,max_len);
		// Update the window if there is room again
		if (stats.last_window_size_transmitted < 2000 && local_wnd->availableSpace() > 2000)
			sendState();
		
		stats.bytes_received += ret;
		return ret;
	}

	
	bool Connection::waitUntilConnected()
	{
		mutex.lock();
		connected.wait(&mutex);
		bool ret = stats.state == CS_CONNECTED;
		mutex.unlock();
		return ret;
	}


	bool Connection::waitForData()
	{
		mutex.lock();
		data_ready.wait(&mutex);
		bool ret = local_wnd->size() > 0;
		mutex.unlock();
		return ret;
	}


	void Connection::close()
	{
		QMutexLocker lock(&mutex);
		if (stats.state == CS_CONNECTED)
		{
			stats.state = CS_FINISHED;
			startTimer();
			sendPackets();
		}
	}
	
	
	void Connection::reset()
	{
		QMutexLocker lock(&mutex);
		if (stats.state != CS_CLOSED)
		{
			sendReset();
			stats.state = CS_CLOSED;
			remote_wnd->clear();
			data_ready.wakeAll();
		}
	}

	void Connection::handleTimeout()
	{
		Out(SYS_UTP|LOG_DEBUG) << "Connection " << stats.recv_connection_id << "|" << stats.send_connection_id << " timeout " << endl;
		QMutexLocker lock(&mutex);
		switch (stats.state)
		{
			case CS_SYN_SENT:
				// No answer to SYN, so just close the connection
				stats.state = CS_CLOSED;
				connected.wakeAll();
				break;
			case CS_FINISHED:
				stats.state = CS_CLOSED;
				data_ready.wakeAll();
				break;
			case CS_CONNECTED:
				remote_wnd->timeout(this);
				stats.packet_size = MIN_PACKET_SIZE;
				stats.timeout *= 2;
				
				if (stats.timeout >= MAX_TIMEOUT)
				{
					// If we have reached the max timeout, kill the connection
					Out(SYS_UTP|LOG_DEBUG) << "Connection " << stats.recv_connection_id << "|" << stats.send_connection_id << " max timeout reached, closing" << endl;
					stats.state = CS_FINISHED;
				}
				sendPackets();
				startTimer();
				if (TimeValue() - last_packet_sent > KEEP_ALIVE_TIMEOUT)
				{
					// Keep the connection alive
					sendState();
				}
				break;
			case CS_CLOSED:
			case CS_IDLE:
				startTimer();
				break;
		}
	}

	void Connection::dumpStats()
	{
		Out(SYS_UTP|LOG_DEBUG) << "Connection " << stats.recv_connection_id << "|" << stats.send_connection_id << " stats:" << endl;
		Out(SYS_UTP|LOG_DEBUG) << "bytes_received   = " << stats.bytes_received << endl;
		Out(SYS_UTP|LOG_DEBUG) << "bytes_sent       = " << stats.bytes_sent << endl;
		Out(SYS_UTP|LOG_DEBUG) << "packets_received = " << stats.packets_received << endl;
		Out(SYS_UTP|LOG_DEBUG) << "packets_sent     = " << stats.packets_sent << endl;
		Out(SYS_UTP|LOG_DEBUG) << "bytes_lost       = " << stats.bytes_lost << endl;
		Out(SYS_UTP|LOG_DEBUG) << "packets_lost     = " << stats.packets_lost << endl;
	}

	bool Connection::allDataSent() const
	{
		QMutexLocker lock(&mutex);
		return remote_wnd->allPacketsAcked() && output_buffer.size() == 0;
	}

	void Connection::startTimer()
	{
		// Timers can only be started from the same thread so if 
		// we are being called from another use a signal
		if (QThread::currentThread() != thread())
			emit doDelayedStartTimer();
		else
			timer.start(stats.timeout,this);
	}
	
	void Connection::delayedStartTimer()
	{
		timer.start(stats.timeout,this);
	}

	void Connection::timerEvent(QTimerEvent* event)
	{
		Q_UNUSED(event);
		handleTimeout();
	}

}

