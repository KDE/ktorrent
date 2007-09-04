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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QFile>
#include <QFileInfo>
#include <util/log.h>
#include "phpcodegenerator.h"
#include "phphandler.h"


using namespace kt;
using namespace bt;

namespace kt
{
	QMap<QString,QByteArray> PhpHandler::scripts;

	PhpHandler::PhpHandler(const QString & php_exe,PhpCodeGenerator* gen) : QProcess(0),php_exe(php_exe),gen(gen)
	{
		if (php_exe.length() == 0)
			this->php_exe = "/usr/bin/php";
		connect(this,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdout()));
		connect(this,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onFinished(int,QProcess::ExitStatus)));

	}
	
	PhpHandler::~PhpHandler()
	{
	}
	
	bool PhpHandler::executeScript(const QString & path,const QMap<QString,QString> & args)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "executeScript" << php_exe << " " << path << endl;

		QFileInfo fi(php_exe);
		if (! (fi.isExecutable() && (fi.isFile() || fi.isSymLink())))
		{
			Out(SYS_WEB|LOG_IMPORTANT) << "Cannot launch php executable " << php_exe << endl;
			return false;
		}

		QByteArray php_s;
		if (!scripts.contains(path))
		{
			QFile fptr(path);
			if (!fptr.open(QIODevice::ReadOnly))
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
		
		output="";
	
		int firstphptag = php_s.indexOf("<?php");
		if ( firstphptag == -1)
			return false;
		
		QByteArray extra_data = (gen->globalInfo() + gen->downloadStatus()).toLocal8Bit();
		
		QMap<QString,QString>::const_iterator it;
			
		for ( it = args.begin(); it != args.end(); ++it )
		{
			QString s = QString("$_REQUEST[%1]=\"%2\";\n").arg(it.key()).arg(it.value());
			extra_data.append(s.toLocal8Bit());
		}
			
		php_s.insert(firstphptag + 6, extra_data);
		start(php_exe);
		if (write(php_s) != php_s.length())
		{
			Out(SYS_WEB|LOG_DEBUG) << "WebGUI: failed to write data to PHP process " << path << endl;
			return false;
		}
		closeWriteChannel();
		return true;
	}
	
	void PhpHandler::onFinished(int /*exitCode*/,QProcess::ExitStatus /*exitStatus*/)
	{
		// read remaining data
		onReadyReadStdout();
		finished();
	}
	
	void PhpHandler::onReadyReadStdout()
	{
		while (bytesAvailable())
		{
			output.append(readLine());
		}
	}

}

#include "phphandler.moc"
