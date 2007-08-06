/***************************************************************************
 *   Copyright (C) 2007 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "ipfilterwidget.h"

#include <torrent/ipblocklist.h>
#include <torrent/globals.h>
#include <util/log.h>
#include <util/constants.h>

#include <QtGui>
#include <QtCore>

#include <KMessageBox>
#include <KFileDialog>
#include <KUrl>

/*#include <ksocketaddress.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>*/

#define MAX_RANGES 500

using namespace bt;

IPFilterWidget::IPFilterWidget ( QWidget* parent, Qt::WFlags fl )
		:QDialog ( parent, fl )
{
	setupUi ( this );

	IPBlocklist& ipfilter = IPBlocklist::instance();
	QStringList* blocklist = ipfilter.getBlocklist();

	for ( QStringList::Iterator it = blocklist->begin(); it != blocklist->end(); ++it )
	{
		( new QListWidgetItem ( lstPeers ) )->setText ( *it );
	}

	delete blocklist;

	setupConnections();
}

void IPFilterWidget::setupConnections()
{
	connect(btnAdd, SIGNAL(clicked()), this, SLOT(btnAdd_clicked()));
	connect(btnClear, SIGNAL(clicked()), this, SLOT(btnClear_clicked()));
	connect(btnApply, SIGNAL(clicked()), this, SLOT(btnApply_clicked()));
	connect(btnOk, SIGNAL(clicked()), this, SLOT(btnOk_clicked()));
	connect(btnSave, SIGNAL(clicked()), this, SLOT(btnSave_clicked()));
	connect(btnOpen, SIGNAL(clicked()), this, SLOT(btnOpen_clicked()));
	connect(btnRemove, SIGNAL(clicked()), this, SLOT(btnRemove_clicked()));	
}

void IPFilterWidget::btnAdd_clicked()
{
	int var=0;

	QRegExp rx ( "([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3})" );
	QRegExpValidator v ( rx,0 );

	QString ip = peerIP->text();

	if ( v.validate ( ip, var ) == QValidator::Acceptable )
	{
		if ( lstPeers->findItems ( ip, 0 ).empty() )
			( new QListWidgetItem ( lstPeers ) )->setText ( ip );
	}
	else
		KMessageBox::sorry ( 0, i18n ( "You must enter IP in format 'XXX.XXX.XXX.XXX'. You can also use wildcards for ranges like '127.0.0.*'." ) );
}

void IPFilterWidget::btnRemove_clicked()
{
	if ( lstPeers->currentItem() )
		delete lstPeers->currentItem();
}

void IPFilterWidget::btnClear_clicked()
{
	lstPeers->clear();
}

void IPFilterWidget::btnOpen_clicked()
{
	QString lf = KFileDialog::getOpenFileName ( KUrl(), "*.txt|",this,i18n ( "Choose a file" ) );

	if ( lf.isEmpty() )
		return;

	btnClear_clicked();

	loadFilter ( lf );
}

void IPFilterWidget::btnSave_clicked()
{
	QString sf = KFileDialog::getSaveFileName ( KUrl(),"*.txt|",this,i18n ( "Choose a filename to save under" ) );

	if ( sf.isEmpty() )
		return;

	saveFilter ( sf );
}

void IPFilterWidget::btnOk_clicked()
{
	btnApply_clicked();
	this->accept();
}

void IPFilterWidget::btnApply_clicked()
{
	IPBlocklist& ipfilter = IPBlocklist::instance();

	QStringList* peers = new QStringList();

	for ( int i=0; i<lstPeers->count(); ++i )
	{
		*peers << lstPeers->item ( i )->text();
	}

	ipfilter.setBlocklist ( peers );

	delete peers;

	Out ( SYS_IPF|LOG_NOTICE ) << "Loaded " << lstPeers->count() << " blocked IP ranges." << endl;
}

void IPFilterWidget::saveFilter ( QString& fn )
{
	QFile fptr ( fn );

	if ( !fptr.open ( QIODevice::WriteOnly ) )
	{
		Out ( SYS_GEN|LOG_NOTICE ) << QString ( "Could not open file %1 for writing." ).arg ( fn ) << endl;
		return;
	}

	QTextStream out ( &fptr );

	for ( int i=0; i<lstPeers->count(); ++i )
	{
		out << lstPeers->item ( i )->text() << ::endl;
	}

	fptr.close();
}

void IPFilterWidget::loadFilter ( QString& fn )
{
	QFile dat ( fn );
	dat.open ( QIODevice::ReadOnly );

	QTextStream stream ( &dat );
	QString line;

	QRegExp rx ( "([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3})" );
	QRegExpValidator v ( rx,0 );


	int i=0;
	int var=0;
	bool err = false;

	while ( !stream.atEnd() && i < MAX_RANGES )
	{
		line = stream.readLine();
		if ( v.validate ( line, var ) != QValidator::Acceptable )
		{
			err = true;
			continue;
		}

		( new QListWidgetItem ( lstPeers ) )->setText ( line );
		++i;
	}

	if ( err )
		Out ( SYS_IPF|LOG_NOTICE ) << "Some lines could not be loaded. Check your filter file..." << endl;

	dat.close();
}

#include "ipfilterwidget.moc"
