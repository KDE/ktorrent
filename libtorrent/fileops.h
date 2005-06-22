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
#ifndef BTFILEOPS_H
#define BTFILEOPS_H

class QString;
class KURL;

namespace bt
{

	/**
	 * Creates a directory. Convenience function around
	 * KIO::NetAccess::mkdir .
	 * @param dir The url of the dir
	 * @throw Error upon error
	 */
	void MakeDir(const KURL & dir);

	/**
	 * Create a symbolic link @a link_url which links to @a link_to 
	 * @param link_to The file to link to
	 * @param link_url The link url
	 */
	void SymLink(const QString & link_to,const QString & link_url);

	/**
	 * Move a file from one location to another
	 * @param src The source file
	 * @param dst The destination file / directory
	 */
	void MoveFile(const KURL & src,const KURL & dst);
}

#endif
