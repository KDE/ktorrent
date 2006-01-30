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
*   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
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

using namespace bt;

namespace kt
{
	typedef struct
	{
		Uint32 ip1;
		Uint32 ip2;
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
	}

	void ConvertDialog::convert()
	{
		QFile source( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.txt" );
		QFile target( KGlobal::dirs() ->saveLocation( "data", "ktorrent" ) + "level1.dat" );

		/**    READ INPUT FILE  **/
		QStringList list;
		lbl_progress->setText( i18n( "Loading txt file..." ) );
		label1->setText( i18n("This could take a couple of minutes. Please wait...") );
		ulong source_size = source.size();
		btnClose->setEnabled( false );
		converting = true;

		int counter = 0;

		if ( source.open( IO_ReadOnly ) )
		{
			QTextStream stream( &source );

			int i = 0;
			QRegExp rx( "[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}-[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}" );
			QRegExpValidator v( rx, 0 );
			int poz = 0;

			while ( !stream.atEnd() )
			{
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

				list += ip_part;
			}
			source.close();
		}
		else
		{
			Out() << "Cannot find level1.txt" << endl;
			btnClose->setEnabled( true );
			btnClose->setText(i18n("&Close"));
			label1->setText("");
			to_convert = false;
			converting = false;
			return ;
		}

		if ( counter != 0 )
		{
			lbl_progress->setText( i18n( "Converting..." ) );
			if ( m_plugin )
				m_plugin->unloadAntiP2P();

			ulong blocks = list.count();

			/** WRITE TO OUTPUT **/
			if ( !target.open( IO_WriteOnly ) )
			{
				Out() << "Unable to open file for writing" << endl;
				btnClose->setEnabled( true );
				btnClose->setText(i18n("&Close"));
				label1->setText("");
				to_convert = false;
				converting = false;
				return ;
			}

			Out() << "Loading finished. Starting conversion..." << endl;

			for ( ulong i = 0; i < blocks; ++i )
			{
				ipblock block = toBlock( list[ i ] );
				target.writeBlock( ( char* ) & block, sizeof( ipblock ) );
				if ( i % 1000 == 0 )
				{
					kProgress1->setProgress( ( int ) 100 * i / blocks );
					if ( i % 10000 == 0 )
						Out() << "Block " << i << " written." << endl;
				}
				KApplication::kApplication()->processEvents();
			}
			kProgress1->setProgress(100);
			Out() << "Finished converting." << endl;
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


}
#include "convertdialog.moc"
