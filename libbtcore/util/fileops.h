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

#include <util/constants.h>
#include <btcore_export.h>

class QString;

#ifdef CopyFile
#undef CopyFile
#endif

namespace bt
{

	/**
	 * Creates a directory.
	 * @param dir The url of the dir
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 * @throw Error upon error
	 */
	BTCORE_EXPORT void MakeDir(const QString & dir,bool nothrow = false);

	/**
	 * Creates a path. 
	 * @param dir The url of the dir
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 * @throw Error upon error
	 */
	BTCORE_EXPORT void MakePath(const QString & dir,bool nothrow = false);

	/**
	* Create a symbolic link @a link_url which links to @a link_to 
	 * @param link_to The file to link to
	 * @param link_url The link url
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 */
	BTCORE_EXPORT void SymLink(const QString & link_to,const QString & link_url,bool nothrow = false);

	/**
	 * Move a file/dir from one location to another
	 * @param src The source file
	 * @param dst The destination file / directory
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 * @param silent Wehter or not to hide progress info
	 */
	BTCORE_EXPORT void Move(const QString & src,const QString & dst,bool nothrow = false,bool silent = false);

	/**
	 * Copy a file.
	 * @param src The source file
	 * @param dst The destination dir/file
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 */
	BTCORE_EXPORT void CopyFile(const QString & src,const QString & dst,bool nothrow = false);
	
	/**
	 * Copy a file or directory
	 * @param src The source file
	 * @param dst The destination dir/file
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 */
	BTCORE_EXPORT void CopyDir(const QString & src,const QString & dst,bool nothrow = false);
	
	/**
	 * Check whether a file/dir exists
	 * @param url The file/dir
	 * @return true if it exits
	 */
	BTCORE_EXPORT bool Exists(const QString & url);

	/**
	 * Delete a file or directory.
	 * @param url The url of the file/dir
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 */
	BTCORE_EXPORT void Delete(const QString & url,bool nothrow = false);

	/**
	 * Try to create a file. Doesn't do anything if the file
	 * already exists.
	 * @param url The url of the file
	 * @param nothrow whether or not we shouldn't throw an Error upon failure
	 */
	BTCORE_EXPORT void Touch(const QString & url,bool nothrow = false);
	
	/**
	 * Calculates the size of a file
	 * @param url Name of the file
	 * @return The size of the file
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	BTCORE_EXPORT Uint64 FileSize(const QString & url);
	
	/**
	 * Get the size of a file.
	 * @param fd The file descriptor of the file
	 * @return The size
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	BTCORE_EXPORT Uint64 FileSize(int fd);
	
	/**
	 * Truncate a file (wrapper around ftruncate)
	 * @param fd The file descriptor of the file
	 * @param size The size to truncate to
	 * @param quick Use the quick way (doesn't prevent fragmentationt)
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	BTCORE_EXPORT void TruncateFile(int fd,Uint64 size,bool quick);
	
	/**
	 * Truncate a file (wrapper around ftruncate)
	 * @param fd Path of the file
	 * @param size The size to truncate to
	 * @throw Error if the file doesn't exist, or something else goes wrong
	 */
	BTCORE_EXPORT void TruncateFile(const QString & path,Uint64 size);

#ifdef HAVE_XFS_XFS_H
	/**
	 * Special truncate for XFS file systems.
	*/
	BTCORE_EXPORT bool XfsPreallocate(int fd,Uint64 size);
	
	/**
	 * Special truncate for XFS file systems.
	 */
	BTCORE_EXPORT bool XfsPreallocate(const QString & path,Uint64 size);

#endif

	/**
	 * Seek in a file, wrapper around lseek
	 * @param fd The file descriptor
	 * @param off Offset
	 * @param whence Position to seek from
	 * @throw Error if something else goes wrong
	 */
	BTCORE_EXPORT void SeekFile(int fd,Int64 off,int whence);

	/// Calculate the number of bytes free on the filesystem path is located
	BTCORE_EXPORT bool FreeDiskSpace(const QString & path,Uint64 & bytes_free);
	
	/// Check if a filename is to long
	BTCORE_EXPORT bool FileNameToLong(const QString & path);
	
	/**
	 * Shorten a filename
	 * @param path Path of the file
	 * @param extra_number Append this number before the extension (if negative or 0, nothing will be appended) This is to handle duplicate shortened names
	 * @return The shortened path
	 */
	BTCORE_EXPORT QString ShortenFileName(const QString & path,int extra_number = -1);
	
	/// Calculate the amount of space a file is taking up (this is not the filesize!)
	BTCORE_EXPORT Uint64 DiskUsage(const QString & filename);
	
	/// Calculate the amount of space a file is taking up (this is not the filesize!)
	BTCORE_EXPORT Uint64 DiskUsage(int fd);
}

#endif
