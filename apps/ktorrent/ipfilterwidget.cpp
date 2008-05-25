/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
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

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qvalidator.h>

#include <klistview.h>
#include <klineedit.h>
#include <ksocketaddress.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#define MAX_RANGES 500

using namespace bt;

IPFilterWidget::IPFilterWidget(QWidget *parent, const char *name)
		:BlacklistWidgetBase(parent, name)
{
	IPBlocklist& ipfilter = IPBlocklist::instance();
	QStringList* blocklist = ipfilter.getBlocklist();
	
	for (QStringList::Iterator it = blocklist->begin(); it != blocklist->end(); ++it)
	{
		new KListViewItem(lstPeers, *it);
	}
	
	delete blocklist;
}

void IPFilterWidget::btnAdd_clicked()
{
	int var=0;
	
	QRegExp rx("([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3})");
	QRegExpValidator v( rx,0);
	
	QString ip = peerIP->text();

	if(v.validate( ip, var ) == QValidator::Acceptable)
	{
		if(lstPeers->findItem(ip, 0) == 0)
			new KListViewItem(lstPeers, ip);
	}
	else
		KMessageBox::sorry(0, i18n("You must enter IP in format 'XXX.XXX.XXX.XXX'. You can also use wildcards for ranges like '127.0.0.*'."));
}

void IPFilterWidget::btnRemove_clicked()
{
	if(lstPeers->currentItem())
		delete lstPeers->currentItem();
}

void IPFilterWidget::btnClear_clicked()
{
	lstPeers->clear();
}

void IPFilterWidget::btnOpen_clicked()
{
	QString lf = KFileDialog::getOpenFileName(QString::null, "*.txt|",this,i18n("Choose a file"));

	if(lf.isEmpty())
		return;
	
	btnClear_clicked();
	
	loadFilter(lf);
}

void IPFilterWidget::btnSave_clicked()
{
	QString sf = KFileDialog::getSaveFileName(QString::null,"*.txt|",this,i18n("Choose a filename to save under"));

	if(sf.isEmpty())
		return;
	
	saveFilter(sf);
}

void IPFilterWidget::btnOk_clicked()
{
	btnApply_clicked();
	this->accept();
}

void IPFilterWidget::btnApply_clicked()
{
	IPBlocklist& ipfilter = IPBlocklist::instance();
	
	int count = 0;
	
	QStringList* peers = new QStringList();
	 
	QListViewItemIterator it(lstPeers);
	while (it.current()) 
	{
		*peers << it.current()->text(0);
		++it;
		++count;
	}
	
	ipfilter.setBlocklist(peers);
	
	delete peers;
	
	Out(SYS_IPF|LOG_NOTICE) << "Loaded " << count << " blocked IP ranges." << endl;
}

void IPFilterWidget::saveFilter(QString& fn)
{
	QFile fptr(fn);
	
	if (!fptr.open(IO_WriteOnly))
	{
		Out(SYS_GEN|LOG_NOTICE) << QString("Could not open file %1 for writing.").arg(fn) << endl;
		return;
	}
	
	QTextStream out(&fptr);
	
	QListViewItemIterator it(lstPeers);
	while (it.current()) 
	{
		out << it.current()->text(0) << ::endl;
		++it;
	}
	
	fptr.close();
}

void IPFilterWidget::loadFilter(QString& fn)
{
	QFile dat(fn);
	dat.open(IO_ReadOnly);

	QTextStream stream( &dat );
	QString line;
	
	QRegExp rx("([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3})");
	QRegExpValidator v( rx,0);
	
	
	int i=0;
	int var=0;
	bool err = false;
	
	while ( !stream.atEnd() && i < MAX_RANGES )
	{
		line = stream.readLine();
		if ( v.validate( line, var ) != QValidator::Acceptable )
		{
			err = true;
			continue;
		}
				
		new KListViewItem(lstPeers, line);
		++i;
	}
	
	if(err)
		Out(SYS_IPF|LOG_NOTICE) << "Some lines could not be loaded. Check your filter file..." << endl;
	
	dat.close();
}

#include "ipfilterwidget.moc"
