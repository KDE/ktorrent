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
#include "convertdialog.h"

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kprogress.h>
#include <kmimetype.h>

#include <util/log.h>
#include <util/constants.h>
#include <torrent/globals.h>
#include <interfaces/coreinterface.h>

#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qevent.h>
#include <qurloperator.h>
#include "antip2p.h"

using namespace bt;

namespace kt
{


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

	IPBlock RangeToBlock(const QString& range)
	{
		IPBlock block;
		QStringList ls = QStringList::split('-', range);
		block.ip1 = toUint32(ls[0]);
		block.ip2 = toUint32(ls[1]);
		return block;
	}

	ConvertDialog::ConvertDialog( IPFilterPlugin* p, QWidget *parent, const char *name )
			: ConvertingDlg( parent, name )
	{
		m_plugin = p;
		btnClose->setText(i18n("Convert"));
		to_convert = true;
		converting = false;
		canceled = false;
		kProgress1->setEnabled(false);
	}

	void ConvertDialog::convert()
	{
		QFile source( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.txt" );
		QFile target( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat" );
		QFile temp( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat.tmp" );
		
		if(target.exists())
		{
			//make backup
			KIO::NetAccess::file_copy(KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat", KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat.tmp", -1, true);
		}

		/*    READ INPUT FILE  */
		QValueList<IPBlock> list;
		lbl_progress->setText( i18n( "Loading txt file..." ) );
		label1->setText( i18n("Please wait...") );
		ulong source_size = source.size();
		btnClose->setEnabled( false );
		converting = true;

		int counter = 0;

		if ( source.open( IO_ReadOnly ) )
		{
			QTextStream stream( &source );
			kProgress1->setEnabled(true);

			int i = 0;
			QRegExp rx( "[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}-[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}" );
			QRegExpValidator v( rx, 0 );
			int poz = 0;

			while ( !stream.atEnd() )
			{
				if(canceled)
					return;
				
				KApplication::kApplication() ->processEvents();
				QString line = stream.readLine();
				i += line.length() * sizeof( char ); //rough estimation of string size
				kProgress1->setProgress( i * 100 / source_size );
				++i;

				QString ip_part = line.section( ':' , -1 );
				if ( v.validate( ip_part, poz ) != QValidator::Acceptable )
					continue;
				else
					++counter;

				list += RangeToBlock(ip_part);
			}
			source.close();
		}
		else
		{
			Out(SYS_IPF|LOG_IMPORTANT) << "Cannot find level1 file" << endl;
			btnClose->setEnabled( true );
			btnClose->setText(i18n("&Close"));
			label1->setText("");
			to_convert = false;
			converting = false;
			return ;
		}

		if ( counter != 0 )
		{
			qHeapSort(list);
			lbl_progress->setText( i18n( "Converting..." ) );
			if ( m_plugin )
				m_plugin->unloadAntiP2P();

			ulong blocks = list.count();

			/** WRITE TO OUTPUT **/
			if ( !target.open( IO_WriteOnly ) )
			{
				Out(SYS_IPF|LOG_IMPORTANT) << "Unable to open file for writing" << endl;
				btnClose->setEnabled( true );
				btnClose->setText(i18n("&Close"));
				label1->setText("");
				to_convert = false;
				converting = false;
				return ;
			}

			Out(SYS_IPF|LOG_NOTICE) << "Loading finished. Starting conversion..." << endl;

			QValueList<IPBlock>::iterator iter;
			int i = 0;
			for (iter = list.begin(); iter != list.end(); ++iter, ++i)
			{
				IPBlock & block = *iter;
				target.writeBlock( ( char* ) & block, sizeof( IPBlock ) );
				if ( i % 1000 == 0 )
				{
					kProgress1->setProgress( ( int ) 100 * i / blocks );
					if ( i % 10000 == 0 )
						Out(SYS_IPF|LOG_DEBUG) << "Block " << i << " written." << endl;
				}
				KApplication::kApplication()->processEvents();
				
				if(canceled)
					return;
			}
			kProgress1->setProgress(100);
			Out(SYS_IPF|LOG_NOTICE) << "Finished converting." << endl;
			lbl_progress->setText( i18n( "File converted." ) );
			target.close();
		}
		else
		{
			lbl_progress->setText( "<font color=\"#ff0000\">" + i18n( "Could not load filter:" ) + "</font>" + i18n( "Bad filter file. It may be corrupted or has a bad format." ) );
			target.remove();
			source.remove();
			btnClose->setEnabled( true );
			btnClose->setText(i18n("&Close"));
			label1->setText("");
			to_convert = false;
			converting = false;
		}

		KApplication::kApplication()->processEvents();
		//reload level1 filter
		if ( m_plugin )
			m_plugin->loadAntiP2P();

		btnClose->setEnabled( true );
		to_convert = false;
		converting = false;
		btnClose->setText(i18n("&Close"));
		label1->setText("");
	}

	void ConvertDialog::btnClose_clicked()
	{
		if(to_convert)
			convert();
		else
			this->close();
	}
	
	void ConvertDialog::closeEvent(QCloseEvent* e)
	{
		if(!converting)
			e->accept();
		else
			e->ignore();
	}
	
	void ConvertDialog::btnCancel_clicked()
	{
		if(converting)
		{
			QFile target( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat" );
			if(target.exists())
				target.remove();
			
			QFile temp( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat.tmp");
			if(temp.exists())
			{
				KIO::NetAccess::file_copy(KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat.tmp", KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat", -1, true);
				temp.remove();
			}
			
			canceled = true;
			Out(SYS_IPF|LOG_NOTICE) << "Conversion canceled." << endl;
		}
		
		
		this->reject();
	}

}

#include "convertdialog.moc"
