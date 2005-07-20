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
#include <kmessagebox.h>
//#include <qfile.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kparts/browserextension.h>
#include "htmlpart.h"

HTMLPart::HTMLPart(QWidget *parent)
		: KHTMLPart(parent)
{
	setJScriptEnabled(true);
	setJavaEnabled(true);
	setMetaRefreshEnabled(true);
	setPluginsEnabled(false);
	KParts::BrowserExtension* ext = this->browserExtension();
	connect(ext,SIGNAL(openURLRequest(const KURL&,const KParts::URLArgs&)),
			this,SLOT(openURLRequest(const KURL&, const KParts::URLArgs& )));

	ext->enableAction("copy",true);
	ext->enableAction("paste",true);
}


HTMLPart::~HTMLPart()
{}

void HTMLPart::copy()
{
	QString txt = selectedText();
	QClipboard *cb = QApplication::clipboard();
    // Copy text into the clipboard
	cb->setText(txt,QClipboard::Clipboard);
}

void HTMLPart::openURLRequest(const KURL &u,const KParts::URLArgs &)
{
	
	if (/*KIO::NetAccess::mimetype(u,0) == "application/x-bittorrent" ||*/
		   u.prettyURL().endsWith(".torrent")  )
	{
		int ret = KMessageBox::questionYesNo(0,
					i18n("Do you want to download the torrent?"),
					i18n("Download Torrent"),
					KGuiItem(i18n("Download"),"down"),KStdGuiItem::cancel());
		
		if (ret == KMessageBox::Yes)
			openTorrent(u);
	}
	else
	{
		if (url().isValid())
			addToHistory(url());
		openURL(u);
	}
}
/*
void HTMLPart::download(const KURL & u)
{
	QString target;
	if (KIO::NetAccess::download(u,target,0))
	{
		// see if we didn't get back HTML code
		QFile fptr(target);
		if (fptr.open(IO_ReadOnly))
		{
			QString line;
			fptr.readLine(line,5);
			if (!line.startsWith("<"))
			{
				// this appears to be a torrent file so pose the big question

			}
			else
			{
				addToHistory(url());
				openURL(u);
			}
		}
		else
		{
			addToHistory(url());
			openURL(u);
		}
		KIO::NetAccess::removeTempFile(target);
	}
}
*/
void HTMLPart::back()
{
	if (history.count() == 0)
		return;

	KURL u = history.back();
	history.pop_back();
	openURL(u);
	if (history.count() == 0)
		backAvailable(false);
}

void HTMLPart::addToHistory(const KURL & url)
{
	history.append(url);
	if (history.count() == 1)
		backAvailable(true);
}

void HTMLPart::reload()
{
	openURL(url());
}

#include "htmlpart.moc"
