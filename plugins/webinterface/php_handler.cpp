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
	QMap<QString,QByteArray> PhpHandler::scripts;

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
		QByteArray php_s;
		if (!scripts.contains(path))
		{
			QFile fptr(path);
			if (!fptr.open(IO_ReadOnly))
			{
				Out(SYS_WEB|LOG_DEBUG) << "Failed to open " << path << endl;
				return false;
			}
			php_s = fptr.readAll();
			scripts.insert(path,php_s);
		}
		else
		{
			php_s = scripts[path];
		}
		
		output.resize(0);
	
		int firstphptag = QCString(php_s).find("<?php");
		if (firstphptag == -1)
			return false;
		
		int off = firstphptag + 6;
		QByteArray data;
		QTextStream ts(data,IO_WriteOnly);
		ts.setEncoding( QTextStream::UnicodeUTF8 );
		ts.writeRawBytes(php_s.data(),off); // first write the opening tag from the script
		php_i->globalInfo(ts);
		php_i->downloadStatus(ts);
		
		QMap<QString,QString>::const_iterator it;
			
		for ( it = args.begin(); it != args.end(); ++it )
		{
			ts << QString("$_REQUEST['%1']=\"%2\";\n").arg(it.key()).arg(it.data());
		}
		ts.writeRawBytes(php_s.data() + off,php_s.size() - off); // the rest of the script
		ts << flush;
		
#if 0
		QFile dinges("output.php");
		if (dinges.open(IO_WriteOnly))
		{
			QTextStream out(&dinges);
			out.writeRawBytes(data.data(),data.size());
			dinges.close();
		}
#endif
		return launch(data);
	}
	
	void PhpHandler::onExited()
	{
		// read remaining data
		onReadyReadStdout();
		finished();
	}
	
	void PhpHandler::onReadyReadStdout()
	{
		QTextStream out(output,IO_WriteOnly|IO_Append);
		while (canReadLineStdout())
		{
			QByteArray d = readStdout();
			out.writeRawBytes(d.data(),d.size());
		}
	}

}

#include "php_handler.moc"
