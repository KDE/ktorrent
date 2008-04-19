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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>
#include "fileops.h"
#include "error.h"
#include "log.h"
#include <torrent/globals.h>
#include "file.h"
#include "array.h"

#ifdef HAVE_XFS_XFS_H

#if !defined(HAVE___S64) || !defined(HAVE___U64)
#include <stdint.h>
#endif

#ifndef HAVE___U64
typedef	uint64_t	__u64;
#endif

#ifndef HAVE___S64
typedef	int64_t		__s64;
#endif

#include <xfs/xfs.h>
#endif

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#if HAVE_STATVFS
#include <sys/statvfs.h>
#else
#include <sys/param.h>
#include <sys/mount.h>
#endif

namespace bt
{
	void MakeDir(const QString & dir,bool nothrow)
	{
		if (mkdir(QFile::encodeName(dir),0777) < -1)
		{
			if (!nothrow)
				throw Error(i18n("Cannot create directory %1: %2")
					.arg(dir).arg(strerror(errno)));
			else
			{
				Out() << QString("Error : Cannot create directory %1 : %2").arg(dir).arg(strerror(errno))<< endl;
			}
		}
	}
	
	void SymLink(const QString & link_to,const QString & link_url,bool nothrow)
	{
		if (symlink(QFile::encodeName(link_to),QFile::encodeName(link_url)) != 0)
		{
			if (!nothrow)
				throw Error(i18n("Cannot symlink %1 to %2: %3")
					.arg(link_url.utf8()).arg(link_to.utf8())
					.arg(strerror(errno)));
			else
				Out() << QString("Error : Cannot symlink %1 to %2: %3")
						.arg(link_url.utf8()).arg(link_to.utf8())
				.arg(strerror(errno)) << endl;
		}
	}

	void Move(const QString & src,const QString & dst,bool nothrow)
	{
	//	Out() << "Moving " << src << " -> " << dst << endl;
		if (!KIO::NetAccess::move(KURL::fromPathOrURL(src),KURL::fromPathOrURL(dst),0))
		{
			if (!nothrow)
				throw Error(i18n("Cannot move %1 to %2: %3")
					.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << QString("Error : Cannot move %1 to %2: %3")
						.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()) << endl;
		
		}
	}

	void CopyFile(const QString & src,const QString & dst,bool nothrow)
	{
		if (!KIO::NetAccess::file_copy(KURL::fromPathOrURL(src),KURL::fromPathOrURL(dst)))
		{
			if (!nothrow)
				throw Error(i18n("Cannot copy %1 to %2: %3")
						.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << QString("Error : Cannot copy %1 to %2: %3")
						.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()) << endl;
	
		}
	}
	
	void CopyDir(const QString & src,const QString & dst,bool nothrow)
	{
		if (!KIO::NetAccess::dircopy(KURL::fromPathOrURL(src),KURL::fromPathOrURL(dst),0))
		{
			if (!nothrow)
				throw Error(i18n("Cannot copy %1 to %2: %3")
						.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << QString("Error : Cannot copy %1 to %2: %3")
						.arg(src).arg(dst)
						.arg(KIO::NetAccess::lastErrorString()) << endl;
	
		}
	}

	bool Exists(const QString & url)
	{
	//	Out() << "Testing if " << url << " exists " << endl;
		if (access(QFile::encodeName(url),F_OK) < 0)
		{
	//		Out() << "No " << endl;
			return false;
		}
		else
		{
	//		Out() << "Yes " << endl;
			return true;
		}
	}
	
	static bool DelDir(const QString & fn)
	{
		QDir d(fn);
		QStringList subdirs = d.entryList(QDir::Dirs);
		
		for (QStringList::iterator i = subdirs.begin(); i != subdirs.end();i++)
		{
			QString entry = *i;

			if (entry == ".." || entry == ".")
				continue;
			
			if (!DelDir(d.absFilePath(entry)))
			{
				Out(SYS_GEN|LOG_DEBUG) << "Delete of " << fn << "/" << entry << " failed !" << endl;
				return false;	
			}
		}
		
		QStringList files = d.entryList(QDir::Files | QDir::System | QDir::Hidden);
		for (QStringList::iterator i = files.begin(); i != files.end();i++)
		{
			QString entry = *i;

			if (remove(QFile::encodeName(d.absFilePath(entry))) < 0)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Delete of " << fn << "/" << entry << " failed !" << endl;
				return false;	
			}
		}
		
		if (!d.rmdir(d.absPath()))
		{
			Out(SYS_GEN|LOG_DEBUG) << "Failed to remove " << d.absPath() << endl;
			return false;
		}
		
		return true;
	}

	void Delete(const QString & url,bool nothrow)
	{
		QCString fn = QFile::encodeName(url);
#if HAVE_STAT64
		struct stat64 statbuf;
		if (lstat64(fn, &statbuf) < 0)
			return;
#else
		struct stat statbuf;
		if (lstat(fn, &statbuf) < 0)
			return;
#endif
		
		bool ok = true;
		// first see if it is a directory
		if (S_ISDIR(statbuf.st_mode)) 
		{
			ok = DelDir(url);
		}
		else
		{
			ok = remove(fn) >= 0;
		}
		
		if (!ok)
		{
			QString err = i18n("Cannot delete %1: %2")
					.arg(url)
					.arg(strerror(errno));
			if (!nothrow)
				throw Error(err);
			else
				Out() << "Error : " << err << endl;
		}
	}

	void Touch(const QString & url,bool nothrow)
	{
		if (Exists(url))
			return;
		
		File fptr;
		if (!fptr.open(url,"wb"))
		{
			if (!nothrow)
				throw Error(i18n("Cannot create %1: %2")
						.arg(url)
						.arg(fptr.errorString()));
			else
				Out() << "Error : Cannot create " << url << " : "
						<< fptr.errorString() << endl;
		
		}
	}
	
	Uint64 FileSize(const QString & url)
	{
		int ret = 0;
#if HAVE_STAT64
		struct stat64 sb;
		ret = stat64(QFile::encodeName(url),&sb);
#else
		struct stat sb;
		ret = stat(QFile::encodeName(url),&sb);
#endif
		if (ret < 0)
			throw Error(i18n("Cannot calculate the filesize of %1: %2")
					.arg(url).arg(strerror(errno)));
		
		return (Uint64)sb.st_size;
	}
	
	Uint64 FileSize(int fd)
	{
		int ret = 0;
#if HAVE_STAT64
		struct stat64 sb;
		ret = fstat64(fd,&sb);
#else
		struct stat sb;
		ret = fstat(fd,&sb);
#endif
		if (ret < 0)
			throw Error(i18n("Cannot calculate the filesize : %2").arg(strerror(errno)));
		
		return (Uint64)sb.st_size;
	}

	bool FatPreallocate(int fd,Uint64 size)
	{
		try
		{
			SeekFile(fd, size - 1, SEEK_SET);
			char zero = 0;		
			if (write(fd, &zero, 1) == -1)
				return false;
				
			TruncateFile(fd,size,true);
		}
		catch (bt::Error & e)
		{
			Out() << e.toString() << endl;
			return false;
		}
		return true;
	}
	
	bool FatPreallocate(const QString & path,Uint64 size)
	{
		int fd = ::open(QFile::encodeName(path),O_RDWR | O_LARGEFILE);
		if (fd < 0)
			throw Error(i18n("Cannot open %1 : %2").arg(path).arg(strerror(errno)));
		
		bool ret = FatPreallocate(fd,size);
		close(fd);
		return ret;
	}

#ifdef HAVE_XFS_XFS_H
	
	bool XfsPreallocate(int fd, Uint64 size)
	{
		if( ! platform_test_xfs_fd(fd) )
		{
			return false;
		}
		
		xfs_flock64_t allocopt;
		allocopt.l_whence = 0;
		allocopt.l_start = 0;
		allocopt.l_len  = size;
		
		return (! static_cast<bool>(xfsctl(0, fd, XFS_IOC_RESVSP64, &allocopt)) );
		
	}
	
	bool XfsPreallocate(const QString & path, Uint64 size)
	{
		int fd = ::open(QFile::encodeName(path), O_RDWR | O_LARGEFILE);
		if (fd < 0)
			throw Error(i18n("Cannot open %1 : %2").arg(path).arg(strerror(errno)));
		
		bool ret = XfsPreallocate(fd,size);
		close(fd);
		return ret;
	}

#endif

	void TruncateFile(int fd,Uint64 size,bool quick)
	{
		if (FileSize(fd) == size)
			return;
		
		if (quick)
		{
#if HAVE_FTRUNCATE64
			if (ftruncate64(fd,size) == -1)
#else
			if (ftruncate(fd,size) == -1)
#endif	
				throw Error(i18n("Cannot expand file : %1").arg(strerror(errno)));
		}
		else
		{
#if HAVE_POSIX_FALLOCATE64
			if (posix_fallocate64(fd,0,size) != 0)
				throw Error(i18n("Cannot expand file : %1").arg(strerror(errno)));
#elif HAVE_POSIX_FALLOCATE
			if (posix_fallocate(fd,0,size) != 0)
				throw Error(i18n("Cannot expand file : %1").arg(strerror(errno)));
#else
			SeekFile(fd,0,SEEK_SET);
			bt::Array<Uint8> buf(4096);
			buf.fill(0);
	
			Uint64 written = 0;
			while (written < size)
			{
				int to_write = size - written;
				if (to_write > 4096)
					to_write = 4096;
				
				int ret = write(fd,buf,to_write);
				if (ret < 0)
					throw Error(i18n("Cannot expand file : %1").arg(strerror(errno)));
				else if (ret == 0 || ret != (int)to_write)
					throw Error(i18n("Cannot expand file").arg(strerror(errno)));
				else
					written += to_write;
			}
#endif
		}
	}
	
	void TruncateFile(const QString & path,Uint64 size)
	{
		int fd = ::open(QFile::encodeName(path),O_RDWR | O_LARGEFILE);
		if (fd < 0)
			throw Error(i18n("Cannot open %1 : %2").arg(path).arg(strerror(errno)));
		
		try
		{
			TruncateFile(fd,size,true);
			close(fd);
		}
		catch (...)
		{
			close(fd);
			throw;
		}
	}
	
	void SeekFile(int fd,Int64 off,int whence)
	{
#if HAVE_LSEEK64
		if (lseek64(fd,off,whence) == -1)
#else
		if (lseek(fd,off,whence) == -1)
#endif
			throw Error(i18n("Cannot seek in file : %1").arg(strerror(errno)));
	}
	
	bool FreeDiskSpace(const QString & path,Uint64 & bytes_free)
	{
#if HAVE_STATVFS
#if HAVE_STATVFS64
		struct statvfs64 stfs;
		if (statvfs64(path.local8Bit(), &stfs) == 0)
#else
		struct statvfs stfs;
		if (statvfs(path.local8Bit(), &stfs) == 0)
#endif
		{
			bytes_free = ((Uint64)stfs.f_bavail) * ((Uint64)stfs.f_frsize);
			return true;
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Error : statvfs for " << path << " failed :  "
						<< QString(strerror(errno)) << endl;

			return false;
		}
#else
		struct statfs stfs;
		if (statfs(path.local8Bit(), &stfs) == 0)
		{
			bytes_free = ((Uint64)stfs.f_bavail) * ((Uint64)stfs.f_bsize);
			return true;
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Error : statfs for " << path << " failed :  "
						<< QString(strerror(errno)) << endl;

			return false;
		}
#endif
	}
}
