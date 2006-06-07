/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef PASTEDIALOG_H
#define PASTEDIALOG_H

#include "pastedlgbase.h"
class KTorrentCore;

/**
 * @author Ivan Vasic
 * @brief Torrent URL paste dialog
 **/
class PasteDialog: public PasteDlgBase
{
		Q_OBJECT
public slots:
    virtual void btnOK_clicked();
	
	public:
		PasteDialog(KTorrentCore* core, QWidget *parent = 0, const char *name = 0);
		
	private:
		KTorrentCore* m_core;
};

#endif
