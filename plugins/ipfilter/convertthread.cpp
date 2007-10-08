/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <errno.h>
#include <string.h>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QRegExp>
#include <QRegExpValidator>
#include <klocale.h>
#include <kio/job.h>
#include <interfaces/functions.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/constants.h>
#include "convertthread.h"
#include "convertdialog.h"

using namespace bt;

namespace kt
{
	typedef struct
	{
		bt::Uint32 ip1;
		bt::Uint32 ip2;
	} ipblock;


	Uint32 toUint32(QString& ip)
	{
		bool test;
		Uint32 ret = ip.section('.',0,0).toULongLong(&test);
		ret <<= 8;
		ret |= ip.section('.',1,1).toULong(&test);
		ret <<= 8;
		ret |= ip.section('.',2,2).toULong(&test);
		ret <<= 8;
		ret |= ip.section('.',3,3).toULong(&test);

		return ret;
	}

	ipblock toBlock(QString& range)
	{
		ipblock block;
		QStringList ls = range.split('-');
		block.ip1 = toUint32(ls[0]);
		block.ip2 = toUint32(ls[1]);
		return block;
	}
	

	ConvertThread::ConvertThread(ConvertDialog* dlg) : dlg(dlg),abort(false)
	{
		txt_file = kt::DataDir() + "level1.txt";
		dat_file = kt::DataDir() + "level1.dat";
		tmp_file = kt::DataDir() + "level1.dat.tmp";
		
	}
	
	ConvertThread::~ConvertThread()
	{
	}
		
	void ConvertThread::run()
	{
		readInput();
		writeOutput();
	}

	void ConvertThread::readInput()
	{
		/*    READ INPUT FILE  */
		int counter = 0;
		QFile source(txt_file);
		if (!source.open(QIODevice::ReadOnly))
		{
			Out(SYS_IPF|LOG_IMPORTANT) << "Cannot find level1.txt file" << endl;
			failure_reason = i18n("Cannot open %1 : %2",txt_file,strerror(errno));
			return;
		}

		Out(SYS_IPF|LOG_NOTICE) << "Loading " << txt_file << " ..." << endl;
		dlg->message(i18n("Loading txt file..."));
		
		ulong source_size = source.size();
		QTextStream stream( &source );
				
		int i = 0;
		QRegExp rx( "[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}-[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}" );
		QRegExpValidator v( rx, 0 );
		int poz = 0;

		while (!stream.atEnd() && !abort)
		{
			QString line = stream.readLine();
			i += line.length() * sizeof( char ); //rough estimation of string size
			dlg->progress(i,source_size);
			++i;
			
			QString ip_part = line.section( ':' , -1 );
			if ( v.validate( ip_part, poz ) != QValidator::Acceptable )
				continue;
			else
				input += ip_part;
		}
		source.close();
		Out(SYS_IPF|LOG_NOTICE) << "Loaded " << input.count() << " lines"  << endl;
		dlg->progress(100,100);
	}
	
	void ConvertThread::writeOutput()
	{
		QFile target(dat_file);
		if (!target.open(QIODevice::WriteOnly))
		{
			Out(SYS_IPF|LOG_IMPORTANT) << "Unable to open file for writing" << endl;
			failure_reason = i18n("Cannot open %1 : %2",dat_file,strerror(errno));
			return;
		}
		
		Out(SYS_IPF|LOG_NOTICE) << "Loading finished, starting conversion..." << endl;
		dlg->message(i18n( "Converting..." ));
		
		QStringList::iterator iter;
		int i = 0;
		int tot = input.count();
		foreach (QString line,input)
		{
			dlg->progress(i,tot);
			ipblock block = toBlock(line);
			target.write( ( char* ) & block, sizeof( ipblock ) );

			if (abort)
			{
				return;
			}
			i++;
		}
	}
}

#include "convertthread.moc"
			 