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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef DCOPINTERFACE_H
#define DCOPINTERFACE_H

#include <dcopobject.h>

/**
@author Joris Guisson
*/
class DCOPInterface : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual void startAll() = 0;
	virtual void stopAll() = 0;
	virtual void setMaxDownloads(int max) = 0;
	virtual void setKeepSeeding(bool ks) = 0;
	virtual void setMaxUploadSpeed(int kbytes_per_sec) = 0;
	virtual void setMaxConnectionsPerDownload(int max) = 0;
	virtual void setShowSysTrayIcon(bool yes) = 0;
	virtual bool changeDataDir(const QString & new_dir) = 0;
	virtual void openTorrent(const QString & file) = 0;

};


#endif
