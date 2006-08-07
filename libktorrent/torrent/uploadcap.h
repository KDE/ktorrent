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
#ifndef BTUPLOADCAP_H
#define BTUPLOADCAP_H

// DEPRECATED
#if 0
#include "cap.h"

namespace bt
{
	class PacketWriter;
	
	

	/**
	 * @author Joris Guisson
	 * @brief Keeps the upload rate under control
	 * 
	 * Before a PeerUploader can send a piece, it must first ask
	 * permission to a UploadCap object. This object will make sure
	 * that the upload rate remains under a specified threshold. When the
	 * threshold is set to 0, no upload capping will be done.
	*/
	class UploadCap : public Cap
	{
		static UploadCap self;

		UploadCap();
	public:
		virtual ~UploadCap();
		

		static UploadCap & instance() {return self;}
	};

}
#endif
#endif
