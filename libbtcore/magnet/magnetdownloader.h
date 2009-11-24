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

#ifndef BT_MAGNETDOWNLOADER_H
#define BT_MAGNETDOWNLOADER_H

#include <QObject>
#include <btcore_export.h>
#include "magnetlink.h"


namespace bt
{
	/**
		Class which tries to download the metadata associated to a MagnetLink
	*/
	class BTCORE_EXPORT MagnetDownloader : public QObject
	{
		Q_OBJECT
	public:
		MagnetDownloader(const MagnetLink & mlink,QObject* parent);
		virtual ~MagnetDownloader();
		
	private:
		MagnetLink mlink;
	};

}

#endif // BT_MAGNETDOWNLOADER_H
