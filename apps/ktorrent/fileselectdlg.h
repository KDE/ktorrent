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

#ifndef FILESELECTDLG_H
#define FILESELECTDLG_H

#include "fileselectdlgbase.h"



namespace kt
{
	class TorrentInterface;
	class FileTreeDirItem;
}
/**
 * @author Joris Guisson
 *
 * Dialog to select which files to download from a multifile torrent.
 */
class FileSelectDlg : public FileSelectDlgBase
{
	Q_OBJECT

	kt::TorrentInterface* tc;
	kt::FileTreeDirItem* root;
public:
	FileSelectDlg(QWidget* parent = 0, const char* name = 0,
				  bool modal = true, WFlags fl = 0 );
	virtual ~FileSelectDlg();
	
	void execute(kt::TorrentInterface* tc);
	
protected slots:
	virtual void reject();
	virtual void accept();
	void selectAll();
	void selectNone();
	void invertSelection();

};

#endif

