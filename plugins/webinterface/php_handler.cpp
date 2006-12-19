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

using namespace kt;
PhpHandler::PhpHandler(PhpInterface *php):QObject()
{
	php_i=php;
	locked=false;
	proc=new QProcess();
	connect(proc, SIGNAL(readyReadStdout()), this, SLOT(readStdout()));
	connect(proc, SIGNAL(readyReadStderr()), this, SLOT(readStderr()));
	connect(proc, SIGNAL(processExited()), this, SLOT(processExitedSlot()));

}

PhpHandler::~PhpHandler()
{
	delete proc;
}

bool PhpHandler::executeScript(QString cmd, QString s, QMap<QString, QString> requestVars)
{
	int end;
	if(locked)
		return false;

	preParse(&s, requestVars);
	output="";
	error="";
	proc->setArguments(cmd);
	if(!proc->isRunning())
		if(!proc->start())
			return false;
	locked=true;
	proc->writeToStdin(s);
	proc->flushStdin();
	proc->closeStdin();
	while(proc->isRunning())
		sleep(10);
	
	end=output.find("</html>");
	output.truncate( end == -1 ? end : end + strlen("</html>"));

	return proc->normalExit();
}


void PhpHandler::preParse(QString *d, QMap<QString, QString> requestVars)
{
	int firstphptag;
	firstphptag=d->find("<?php");
	if(firstphptag==-1)
		return;
	d->insert(firstphptag+6,php_i->globalInfo());
	d->insert(firstphptag+6,php_i->downloadStatus());
	QValueList<QString> keys=requestVars.keys();
	QValueList<QString>::iterator it;
    	for ( it = keys.begin(); it != keys.end(); ++it )
		d->insert(firstphptag+6, QString("$_REQUEST[%1]=\"%2\";\n").arg(*it).arg(requestVars[*it]));
	

}

void PhpHandler::readStdout()
{
	output.append(proc->readStdout().data());
}

void PhpHandler::readStderr()
{
	error.append(proc->readStderr().data());
}

void PhpHandler::processExitedSlot()
{
	locked=false;
}


