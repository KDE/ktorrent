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
#include "torrentfileinterface.h"

namespace kt
{

	TorrentFileInterface::TorrentFileInterface(const QString & path,Uint64 size)
	: path(path),size(size),first_chunk(0),last_chunk(0),num_chunks_downloaded(0),
	priority(NORMAL_PRIORITY),m_emitDlStatusChanged(true),preview(false)
	{
		preexisting = false;
	}


	TorrentFileInterface::~TorrentFileInterface()
	{}

	float TorrentFileInterface::getDownloadPercentage() const
	{
		Uint32 num = last_chunk - first_chunk + 1;
		return 100.0f * (float)num_chunks_downloaded / num;
	}
}

#include "torrentfileinterface.moc"

