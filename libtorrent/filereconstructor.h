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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTFILERECONSTRUCTOR_H
#define BTFILERECONSTRUCTOR_H

#include <qobject.h>
#include "globals.h"

namespace bt
{

	class ChunkManager;
	
	/**
	@author Joris Guisson
	*/
	class FileReconstructor : public QObject
	{
		Q_OBJECT
	public:
		FileReconstructor(Torrent & tor,ChunkManager & cman);
		virtual ~FileReconstructor();

		void reconstruct(const QString & output);
		
	signals:
		void completed(int chunks);
	private:
		void multiReconstruct(const QString & dir);
		void singleReconstruct(const QString & file);
		void reconstructFile(const QString & path,Uint32 file_size,Uint32 & cur_off);
		void createFileDir(const QString & file);
		
		Torrent & tor;
		ChunkManager & cman;
	};

}

#endif
