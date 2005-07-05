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
#include <unistd.h>
#include <errno.h>
#include <klocale.h>
#include <kio/netaccess.h>
#include "fileops.h"
#include "error.h"
#include "log.h"

namespace bt
{
	extern Log& Out();

	void MakeDir(const KURL & dir,bool nothrow)
	{
		if (!KIO::NetAccess::mkdir(dir,0,0755))
		{
			if (!nothrow)
				throw Error(i18n("Cannot create directory %1: %2")
					.arg(dir.prettyURL()).arg(KIO::NetAccess::lastErrorString()));
			else
			{
				Out() << "Error : Cannot create directory " << dir << " : "
						<< KIO::NetAccess::lastErrorString() << endl;
			}
		}
	}
	
	void SymLink(const QString & link_to,const QString & link_url,bool nothrow)
	{
		if (symlink(link_to.utf8(),link_url.utf8()) != 0)
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

	void Move(const KURL & src,const KURL & dst,bool nothrow)
	{
		if (!KIO::NetAccess::move(src,dst,0))
		{
			if (!nothrow)
				throw Error(i18n("Cannot move %1 to %2: %3")
					.arg(src.prettyURL()).arg(dst.prettyURL())
					.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << QString("Error : Cannot move %1 to %2: %3")
						.arg(src.prettyURL()).arg(dst.prettyURL())
						.arg(KIO::NetAccess::lastErrorString()) << endl;
		}
	}

	void CopyFile(const KURL & src,const KURL & dst,bool nothrow)
	{
		if (!KIO::NetAccess::file_copy(src,dst))
		{
			if (!nothrow)
				throw Error(i18n("Cannot copy %1 to %2: %3")
						.arg(src.prettyURL()).arg(dst.prettyURL())
						.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << QString("Error : Cannot copy %1 to %2: %3")
						.arg(src.prettyURL()).arg(dst.prettyURL())
						.arg(KIO::NetAccess::lastErrorString()) << endl;
		}
	}

	bool Exists(const KURL & url)
	{
		return KIO::NetAccess::exists(url,false,0);
	}

	void Delete(const KURL & url,bool nothrow)
	{
		if (!KIO::NetAccess::del(url,0))
		{
			if (!nothrow)
				throw Error(i18n("Cannot delete %1: %2")
						.arg(url.prettyURL())
						.arg(KIO::NetAccess::lastErrorString()));
			else
				Out() << "Error : Cannot delete " << url << " : " << KIO::NetAccess::lastErrorString() << endl;
		}
	}

	/*
	switch (errno)
	{
	case EPERM:
	Out() << "EPERM" << endl;
	break;
	case EFAULT:
	Out() << "EFAULT" << endl;
	break;
	case EACCES:
	Out() << "EACCESS" << endl;
	break;
	
	case ENAMETOOLONG:
	Out() << "ENAMETOOLONG" << endl;
	break;
	
	case ENOENT:
	Out() << "ENOENT" << endl;
	break;
	
	case ENOTDIR:
	Out() << "ENOTDIR" << endl;
	break;
	
	case ENOMEM:
	Out() << "ENOMEM" << endl;
	break;
	case EROFS:
	Out() << "EROFS" << endl;
	break;
	
	case EEXIST:
	Out() << "EEXIST" << endl;
	break;
	
	case ELOOP:
	Out() << "ELOOP" << endl;
	break;
	case ENOSPC:
	Out() << "ENOSPC" << endl;
	break;
	
	case EIO:
	Out() << "EIO" << endl;
	break;
	
}*/
}
