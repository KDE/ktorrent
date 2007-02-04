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


using namespace kt;
using namespace bt;

PhpHandler::PhpHandler(PhpInterface *php):php_i(php)
{
}

PhpHandler::~PhpHandler()
{
}

bool PhpHandler::executeScript(QString cmd, QString s, QMap<QString, QString> requestVars)
{
	int fdsStdin[2], fdsStdout[2];
	char buf[4096];
	pid_t pid;

	if(fi.filePath()!=cmd)
		fi.setFile(cmd);
	if(!fi.isExecutable())
		return false;

	preParse(&s, requestVars);
	output="";

	if (pipe (fdsStdin) == -1 || pipe (fdsStdout) == -1)
	{
		Out(SYS_WEB|LOG_DEBUG) << QString("pipe failed : %1").arg(strerror(errno)) << endl;
		return false;
	}

	pid = fork ();
	if (pid < 0)
	{
		Out(SYS_WEB|LOG_DEBUG) << QString("failed to fork PHP process : %1").arg(strerror(errno)) << endl;
		return false;		
	}
	
	if (pid == (pid_t) 0) {
		close (fdsStdin[1]);
		close (fdsStdout[0]);
		
		dup2 (fdsStdin[0], STDIN_FILENO);
		dup2 (fdsStdout[1], STDOUT_FILENO);
		execlp (cmd.latin1(), cmd.latin1(), 0);
		exit(-1); // we are in the child so just exit the process
	}
	else {

		FILE* streamStdin;
		FILE* streamStdout;

		close (fdsStdin[0]);
		close (fdsStdout[1]);

		streamStdin  = fdopen (fdsStdin[1], "w");
		streamStdout = fdopen (fdsStdout[0], "r");

		fprintf (streamStdin, "%s",(const char * ) s.utf8());
		fflush (streamStdin);
		close (fdsStdin[1]);


		while(fgets(buf, 4096, streamStdout)){
			output.append(QString::fromUtf8(buf, strlen(buf)));
		}
		close (fdsStdout[0]);
		waitpid (pid, NULL, 0);
	}


	return true;
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
