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
#include "fileops.h"
#include <config-btcore.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <klocale.h>
#include <kio/job.h> 
#include <kio/netaccess.h>
#include <kio/copyjob.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <qdir.h>
#include <qfile.h>
#include <qstringlist.h>
#include "error.h"
#include "log.h"
#include "file.h"
#include "array.h"
#include "functions.h"
#ifdef Q_WS_WIN
#include "win32.h"
#endif

#include "limits.h"

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

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

#ifndef Q_WS_WIN
#include <sys/statvfs.h>
#endif
#ifdef CopyFile
#undef CopyFile
#endif

namespace bt
{
	void MakeDir(const QString & dir,bool nothrow)
	{
		QDir d(dir);
		if (d.exists())
			return;
		
		
		QString n = d.dirName();
		if (!d.cdUp() || !d.mkdir(n))
		{
			QString error = i18n("Cannot create directory %1",dir);
			Out(SYS_DIO|LOG_NOTICE) << error << endl;
			if (!nothrow)
				throw Error(error);
		}
	}
	
	void MakePath(const QString & dir,bool nothrow)
	{
		QStringList sl = dir.split(bt::DirSeparator(),QString::SkipEmptyParts);
		QString ctmp;
#ifndef Q_WS_WIN
		ctmp += bt::DirSeparator();
#endif

		for (int i = 0;i < sl.count();i++)
		{
			ctmp += sl[i];
			if (!bt::Exists(ctmp))
			{
				try
				{
					MakeDir(ctmp,false);
				}
				catch (...)
				{
					if (!nothrow)
						throw;
					return;
				}
			}

			ctmp += bt::DirSeparator();
		}
	}

	void SymLink(const QString & link_to,const QString & link_url,bool nothrow)
	{
		if (symlink(QFile::encodeName(link_to),QFile::encodeName(link_url)) != 0)
		{
			if (!nothrow)
				throw Error(i18n("Cannot symlink %1 to %2: %3"
							,link_url,link_to
							,strerror(errno)));
			else
				Out(SYS_DIO|LOG_NOTICE) << QString("Error : Cannot symlink %1 to %2: %3")
					.arg(link_url).arg(link_to)
					.arg(strerror(errno)) << endl;
		}
	}

	void Move(const QString & src,const QString & dst,bool nothrow,bool silent)
	{
		//	Out() << "Moving " << src << " -> " << dst << endl;
		KIO::CopyJob *mv = KIO::move(KUrl(src),KUrl(dst),silent ? KIO::HideProgressInfo|KIO::Overwrite : KIO::DefaultFlags); 
		if (!KIO::NetAccess::synchronousRun(mv , 0)) 
		{
			if (!nothrow)
				throw Error(i18n("Cannot move %1 to %2: %3",
							src,dst,
							KIO::NetAccess::lastErrorString()));
			else
				Out(SYS_DIO|LOG_NOTICE) << QString("Error : Cannot move %1 to %2: %3")
					.arg(src).arg(dst)
					.arg(KIO::NetAccess::lastErrorString()) << endl;

		}
	}

	void CopyFile(const QString & src,const QString & dst,bool nothrow)
	{
		if (!KIO::NetAccess::file_copy(KUrl(src),KUrl(dst)))
		{
			if (!nothrow)
				throw Error(i18n("Cannot copy %1 to %2: %3",
							src,dst,
							KIO::NetAccess::lastErrorString()));
			else
				Out(SYS_DIO|LOG_NOTICE) << QString("Error : Cannot copy %1 to %2: %3")
					.arg(src).arg(dst)
					.arg(KIO::NetAccess::lastErrorString()) << endl;

		}
	}

	void CopyDir(const QString & src,const QString & dst,bool nothrow)
	{
		if (!KIO::NetAccess::dircopy(KUrl(src),KUrl(dst),0))
		{
			if (!nothrow)
				throw Error(i18n("Cannot copy %1 to %2: %3",
							src,dst,
							KIO::NetAccess::lastErrorString()));
			else
				Out(SYS_DIO|LOG_NOTICE) << QString("Error : Cannot copy %1 to %2: %3")
					.arg(src).arg(dst)
					.arg(KIO::NetAccess::lastErrorString()) << endl;

		}
	}

	bool Exists(const QString & url)
	{
		return QFile::exists(url);
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

			if (!DelDir(d.absoluteFilePath(entry)))
				return false;	
		}

		QStringList files = d.entryList(QDir::Files | QDir::System | QDir::Hidden);
		for (QStringList::iterator i = files.begin(); i != files.end();i++)
		{
			QString file = d.absoluteFilePath(*i);
			QFile fp(file);
			if (!QFileInfo(file).isWritable() && !fp.setPermissions(QFile::ReadUser | QFile::WriteUser))
				return false;

			if (!fp.remove())
				return false;	
		}

		if (!d.rmdir(d.absolutePath()))
			return false;

		return true;
	}

	void Delete(const QString & url,bool nothrow)
	{
		bool ok = true;
		// first see if it is a directory
		if (QDir(url).exists())
		{
			ok = DelDir(url);
		}
		else
		{
			ok = QFile::remove(url);
		}

		if (!ok)
		{
			QString err = i18n("Cannot delete %1: %2",url,strerror(errno));
			if (!nothrow)
				throw Error(err);
			else
				Out(SYS_DIO|LOG_NOTICE) << "Error : " << err << endl;
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
				throw Error(i18n("Cannot create %1: %2",url,fptr.errorString()));
			else
				Out(SYS_DIO|LOG_NOTICE) << "Error : Cannot create " << url << " : "
					<< fptr.errorString() << endl;
		}
	}

	Uint64 FileSize(const QString & url)
	{
		int ret = 0;
#ifdef HAVE_STAT64
		struct stat64 sb;
		ret = stat64(QFile::encodeName(url),&sb);
#else
		struct stat sb;
		ret = stat(QFile::encodeName(url),&sb);
#endif
		if (ret < 0)
			throw Error(i18n("Cannot calculate the filesize of %1: %2",url,strerror(errno)));

		return (Uint64)sb.st_size;
	}

	Uint64 FileSize(int fd)
	{
		int ret = 0;
#ifdef HAVE_STAT64
		struct stat64 sb;
		ret = fstat64(fd,&sb);
#else
		struct stat sb;
		ret = fstat(fd,&sb);
#endif
		if (ret < 0)
			throw Error(i18n("Cannot calculate the filesize : %1",strerror(errno)));

		return (Uint64)sb.st_size;
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
			throw Error(i18n("Cannot open %1 : %2",path,strerror(errno)));

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
#ifdef HAVE_FTRUNCATE64
			if (ftruncate64(fd,size) == -1)
#else
			if (ftruncate(fd,size) == -1)
#endif
				throw Error(i18n("Cannot expand file: %1",strerror(errno)));
		}
		else
		{
#ifdef HAVE_POSIX_FALLOCATE64
			if (posix_fallocate64(fd,0,size) != 0)
				throw Error(i18n("Cannot expand file: %1",strerror(errno)));
#elif HAVE_POSIX_FALLOCATE
			if (posix_fallocate(fd,0,size) != 0)
				throw Error(i18n("Cannot expand file: %1",strerror(errno)));
#else
			if (ftruncate(fd,size) == -1)
				throw Error(i18n("Cannot expand file: %1",strerror(errno)));
#endif
		}
	}

	void TruncateFile(const QString & path,Uint64 size)
	{
		int fd = ::open(QFile::encodeName(path),O_RDWR | O_LARGEFILE);
		if (fd < 0)
			throw Error(i18n("Cannot open %1 : %2",path,strerror(errno)));

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
#ifdef HAVE_LSEEK64
		if (lseek64(fd,off,whence) == -1)
#else
		if (lseek(fd,off,whence) == -1)
#endif
			throw Error(i18n("Cannot seek in file : %1",strerror(errno)));
	}

	bool FreeDiskSpace(const QString & path,Uint64 & bytes_free)
	{
#ifdef HAVE_STATVFS
#ifdef HAVE_STATVFS64
		struct statvfs64 stfs;
		if (statvfs64(QFile::encodeName(path), &stfs) == 0)
#else
		struct statvfs stfs;
		if (statvfs(QFile::encodeName(path), &stfs) == 0)
#endif
		{
			if (stfs.f_blocks == 0) // if this is 0, then we are using gvfs
				return false;

			bytes_free = ((Uint64)stfs.f_bavail) * ((Uint64)stfs.f_frsize);
			return true;
		}
		else
		{
			Out(SYS_GEN|LOG_DEBUG) << "Error : statvfs for " << path << " failed :  "
				<< QString(strerror(errno)) << endl;

			return false;
		}
#elif defined(Q_WS_WIN)
#ifdef UNICODE
		LPCWSTR tpath = (LPCWSTR)path.utf16();
#else
		const char *tpath = path.toLocal8Bit();
#endif
		if(GetDiskFreeSpaceEx(tpath, (PULARGE_INTEGER)&bytes_free, NULL, NULL)) {
			return true;
		} else {
			return false;
		}
#else
		return false;
#endif
	}
	
	bool FileNameToLong(const QString & path)
	{
		int length = 0;
		QStringList names = path.split("/");
		foreach (const QString & s, names)
		{
			QByteArray encoded = QFile::encodeName(s);
			if (encoded.length() >= NAME_MAX)
				return true;
			length += encoded.length();
		}
		
		length += path.count("/");
		return length >= PATH_MAX;
	}
	
	static QString ShortenName(const QString & name,int extra_number)
	{
		QFileInfo fi(name);
		QString ext = fi.suffix();
		QString base = fi.completeBaseName();
		
		// calculate the fixed length, 1 is for the . between filename and extension
		int fixed_len = 0;
		if (ext.length() > 0)
			fixed_len += QFile::encodeName(ext).length() + 1;
		if (extra_number > 0)
			fixed_len += QFile::encodeName(QString::number(extra_number)).length();
		
		// if we can't shorten it, give up
		if (fixed_len > NAME_MAX - 4)
			return name;
		
		do
		{
			base = base.left(base.length() - 1);
		}while (fixed_len + QFile::encodeName(base).length() > NAME_MAX - 4 && base.length() != 0);
		
		base += "... "; // add ... so that the user knows the name is shortened
		
		QString ret = base;
		if (extra_number > 0)
			ret += QString::number(extra_number);
		if (ext.length() > 0)
			ret += "." + ext;
		return ret;
	}
	
	static QString ShortenPath(const QString & path,int extra_number)
	{
		int max_len = PATH_MAX;
		QByteArray encoded = QFile::encodeName(path);
		if (encoded.length() < max_len)
			return path;

		QFileInfo fi(path);
		QString ext = fi.suffix();
		QString name = fi.completeBaseName();
		QString fpath = fi.path() + "/";

                // calculate the fixed length, 1 is for the . between filename and extension
		int fixed_len = QFile::encodeName(fpath).length();
		if (ext.length() > 0)
			fixed_len += QFile::encodeName(ext).length() + 1;
		if (extra_number > 0)
			fixed_len += QFile::encodeName(QString::number(extra_number)).length();

                // if we can't shorten it, give up
		if (fixed_len > max_len - 4)
			return path;

		do
		{
			name = name.left(name.length() - 1);
		}while (fixed_len + QFile::encodeName(name).length() > max_len - 4 && name.length() != 0);

		name += "... "; // add ... so that the user knows the name is shortened

		QString ret = fpath + name;
		if (extra_number > 0)
			ret += QString::number(extra_number);
		if (ext.length() > 0)
			ret += "." + ext;

		return ret;
	}
	
	QString ShortenFileName(const QString & path,int extra_number)
	{
		QString assembled = "/";
		QStringList names = path.split("/",QString::SkipEmptyParts);
		int cnt = 0;
		for (QStringList::iterator i = names.begin();i != names.end();i++)
		{
			QByteArray encoded = QFile::encodeName(*i);
			if (encoded.length() >= NAME_MAX)
				*i = ShortenName(*i,extra_number);
			
			assembled += *i;
			if (cnt < names.count() - 1)
				assembled += '/';
			cnt++;
		}
		
		if (QFile::encodeName(assembled).length() >= PATH_MAX)
		{
			// still to long, then the Shorten the filename
			assembled = ShortenPath(assembled,extra_number);
		}
		
		return assembled;
	}
	

	Uint64 DiskUsage(const QString& filename)
	{
		Uint64 ret = 0;
#ifndef Q_WS_WIN
#ifdef HAVE_STAT64
		struct stat64 sb;
		if (stat64(QFile::encodeName(filename),&sb) == 0)
#else
		struct stat sb;
		if (stat(QFile::encodeName(filename),&sb) == 0)
#endif
		{
			ret = (Uint64)sb.st_blocks * 512;
		}
#else
		DWORD high = 0;
		DWORD low = GetCompressedFileSize((LPWSTR)filename.utf16(),&high);
		if (low != INVALID_FILE_SIZE)
			ret = (high * MAXDWORD) + low;
#endif
		return ret;
	}
	
	Uint64 DiskUsage(int fd)
	{
		Uint64 ret = 0;
#ifndef Q_WS_WIN
#ifdef HAVE_FSTAT64
		struct stat64 sb;
		if (fstat64(fd,&sb) == 0)
#else
		struct stat sb;
		if (fstat(fd,&sb) == 0)
#endif
		{
			ret = (Uint64)sb.st_blocks * 512;
		}
#else
		struct _BY_HANDLE_FILE_INFORMATION info;
		GetFileInformationByHandle((void *)&fd,&info);
		ret = (info.nFileSizeHigh * MAXDWORD) + info.nFileSizeLow;
#endif
		return ret;
	}

}
