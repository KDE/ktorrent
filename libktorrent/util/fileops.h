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
#ifndef BTFILEOPS_H
#define BTFILEOPS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <util/constants.h>
class QString;

namespace bt
{

	/**
	 * Creates a directory. Convenience function around
	 * KIO::NetAccess::mkdir .
	 * @param dir The url of the dir
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 * @throw Error upon error
	 */
	void MakeDir(const QString & dir,bool nothrow = false);

	/**
	 * Create a symbolic link @a link_url which links to @a link_to 
	 * @param link_to The file to link to
	 * @param link_url The link url
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void SymLink(const QString & link_to,const QString & link_url,bool nothrow = false);

	/**
	 * Move a file/dir from one location to another
	 * @param src The source file
	 * @param dst The destination file / directory
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void Move(const QString & src,const QString & dst,bool nothrow = false);

	/**
	 * Copy a file.
	 * @param src The source file
	 * @param dst The destination dir/file
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void CopyFile(const QString & src,const QString & dst,bool nothrow = false);
	
	/**
	 * Copy a file or directory
	 * @param src The source file
	 * @param dst The destination dir/file
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void CopyDir(const QString & src,const QString & dst,bool nothrow = false);
	
	/**
	 * Check wether a file/dir exists
	 * @param url The file/dir
	 * @return true if it exits
	 */
	bool Exists(const QString & url);

	/**
	 * Delete a file or directory.
	 * @param url The url of the file/dir
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void Delete(const QString & url,bool nothrow = false);

	/**
	 * Try to create a file. Doesn't do anything if the file
	 * already exists.
	 * @param url The url of the file
	 * @param nothrow wether or not we shouldn't throw an Error upon failure
	 */
	void Touch(const QString & url,bool nothrow = false);
	
	/**
	 * Calculates the size of a file
	 * @param url Name of the file
	 * @return The size of the file
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	Uint64 FileSize(const QString & url);
	
	/**
	 * Get the size of a file.
	 * @param fd The file descriptor of the file
	 * @return The size
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	Uint64 FileSize(int fd);
	
	/**
	 * Truncate a file (wrapper around ftruncate)
	 * @param fd The file descriptor of the file
	 * @param size The size to truncate to
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	void TruncateFile(int fd,Uint64 size,bool quick);
	
	/**
	 * Truncate a file (wrapper around ftruncate)
	 * @param fd Path of the file
	 * @param size The size to truncate to
	 * @param quick Use the quick way (doesn't prevent fragmentationt)
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	void TruncateFile(const QString & path,Uint64 size);
	
	/**
	 * Special truncate for FAT file systems.
	*/
	bool FatPreallocate(int fd,Uint64 size);
	
	/**
	 * Special truncate for FAT file systems.
	 */
	bool FatPreallocate(const QString & path,Uint64 size);

#ifdef HAVE_XFS_XFS_H
	/**
	 * Special truncate for XFS file systems.
	*/
	bool XfsPreallocate(int fd,Uint64 size);
	
	/**
	 * Special truncate for XFS file systems.
	 */
	bool XfsPreallocate(const QString & path,Uint64 size);

#endif

	/**
	 * Seek in a file, wrapper around lseek
	 * @param fd The file descriptor
	 * @param off Offset
	 * @param whence Position to seek from
	 * @throw Error if something else goes wrong
	 */
	void SeekFile(int fd,Int64 off,int whence);

	/// Calculate the number of bytes free on the filesystem path is located
	bool FreeDiskSpace(const QString & path,Uint64 & bytes_free);
}

#endif
