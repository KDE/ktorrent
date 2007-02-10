  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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
#include "php_handler.h"

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <util/log.h>
#include "php_interface.h"


using namespace kt;
using namespace bt;

namespace kt
{
	QMap<QString,QString> PhpHandler::scripts;

	PhpHandler::PhpHandler(const QString & php_exe,PhpInterface *php) : QProcess(php_exe),php_i(php)
	{
		connect(this,SIGNAL(readyReadStdout()),this,SLOT(onReadyReadStdout()));
		connect(this,SIGNAL(processExited()),this,SLOT(onExited()));
	}
	
	PhpHandler::~PhpHandler()
	{
	}
	
	bool PhpHandler::executeScript(const QString & path,const QMap<QString,QString> & args)
	{
		QString php_s;
		if (!scripts.contains(path))
		{
			QFile fptr(path);
			if (!fptr.open(IO_ReadOnly))
			{
				Out(SYS_WEB|LOG_DEBUG) << "Failed to open " << path << endl;
				return false;
			}
			php_s = QString(fptr.readAll());
			scripts.insert(path,php_s);
		}
		else
		{
			php_s = scripts[path];
		}
		
		output="";
	
		int firstphptag = php_s.find("<?php");
		if ( firstphptag == -1)
			return false;
		
		QString extra_data = php_i->globalInfo() + php_i->downloadStatus();
		
		QMap<QString,QString>::const_iterator it;
			
		for ( it = args.begin(); it != args.end(); ++it )
		{
			extra_data += QString("$_REQUEST[%1]=\"%2\";\n").arg(it.key()).arg(it.data());
		}
			
		php_s.insert(firstphptag + 6, extra_data);
		return launch(php_s);
	}
	
	void PhpHandler::onExited()
	{
		// read remaining data
		onReadyReadStdout();
		finished();
	}
	
	void PhpHandler::onReadyReadStdout()
	{
		while (canReadLineStdout())
		{
			QByteArray d = readStdout();
			output += QString(d);
		}
	}

}

#include "php_handler.moc"
				 