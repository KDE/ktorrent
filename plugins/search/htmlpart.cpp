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
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/jobclasses.h>
//#include <qfile.h>
#include <qclipboard.h>
#include <qapplication.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kparts/browserextension.h>
#include <util/constants.h>
#include <khtmlview.h>
#include "htmlpart.h"

using namespace bt;

namespace kt
{
	
	HTMLPart::HTMLPart(QWidget *parent)
			: KHTMLPart(parent)
	{
		setJScriptEnabled(true);
		setJavaEnabled(true);
		setMetaRefreshEnabled(true);
		setPluginsEnabled(false);
		setStatusMessagesEnabled(false);
		KParts::BrowserExtension* ext = this->browserExtension();
		connect(ext,SIGNAL(openURLRequest(const KURL&,const KParts::URLArgs&)),
				this,SLOT(openURLRequest(const KURL&, const KParts::URLArgs& )));
	
		ext->enableAction("copy",true);
		ext->enableAction("paste",true);
		active_job = 0;
	}
	
	
	HTMLPart::~HTMLPart()
	{}
	
	void HTMLPart::copy()
	{
		QString txt = selectedText();
		QClipboard *cb = QApplication::clipboard();
		// Copy text into the clipboard
		if (cb)
			cb->setText(txt,QClipboard::Clipboard);
	}
	
	void HTMLPart::openURLRequest(const KURL &u,const KParts::URLArgs &)
	{
		if (active_job)
		{
			active_job->kill(true);
			active_job = 0;
		}
		
		KIO::TransferJob* j = KIO::get(u,false,false);
		connect(j,SIGNAL(data(KIO::Job*,const QByteArray &)),
				this,SLOT(dataRecieved(KIO::Job*, const QByteArray& )));
		connect(j,SIGNAL(result(KIO::Job*)),this,SLOT(jobDone(KIO::Job* )));
		connect(j,SIGNAL(mimetype(KIO::Job*, const QString &)),
				this,SLOT(mimetype(KIO::Job*, const QString& )));
	
		active_job = j;
		curr_data.resize(0);
		mime_type = QString::null;
		curr_url = u;
	}
	
	void HTMLPart::back()
	{
		if (history.count() <= 1)
		{
			backAvailable(false);
		}
		else
		{
			history.pop_back();
			KURL u = history.back();
			openURL(u);
			backAvailable(history.count() > 1 ? true : false);
			
		}
	}
	
	void HTMLPart::addToHistory(const KURL & url)
	{
		history.append(url);
		if (history.count() > 1)
			backAvailable(true);
	}
	
	void HTMLPart::reload()
	{
		openURL(url());
	}
	
	void HTMLPart::dataRecieved(KIO::Job* job,const QByteArray & data)
	{
		if (job != active_job)
		{
			job->kill(true);
			return;
		}
		
		if (data.size() == 0)
			return;
		
		Uint32 off = curr_data.size();
		curr_data.resize(curr_data.size() + data.size());
		for (Uint32 i = 0;i < data.size();i++)
		{
			curr_data[i + off] = data[i];
		}
	}
	
	void HTMLPart::mimetype(KIO::Job* job,const QString & mt)
	{
		if (job != active_job)
		{
			job->kill(true);
			return;
		}
	
		mime_type = mt;
	}
	
	void HTMLPart::jobDone(KIO::Job* job)
	{
		if (job != active_job)
		{
			job->kill(true);
			return;
		}
		
		if (job->error() == 0)
		{
			bool is_bencoded_data = curr_data.size() > 0 &&
					curr_data[0] == 'd' &&
					curr_data[curr_data.size()-1] == 'e';
			
			if (is_bencoded_data || mime_type == "application/x-bittorrent")
			{
				int ret = KMessageBox::questionYesNoCancel(0,
						i18n("Do you want to download or save the torrent?"),
						i18n("Download Torrent"),
						KGuiItem(i18n("to download", "Download"),"down"),
						KStdGuiItem::save());
			
				if (ret == KMessageBox::Yes)
					openTorrent(curr_url);
				else if (ret == KMessageBox::No)
					saveTorrent(curr_url);
			}
			else
			{
				addToHistory(curr_url);
				begin(curr_url);
				write(curr_data.data(),curr_data.size());
				end();
				view()->ensureVisible(0,0);
				searchFinished();
			}
		}
		else
		{
			begin(curr_url);
			write(KIO::buildErrorString(job->error(),job->errorText()));/*,&curr_url));**/
			end();
		}
		active_job = 0;
		curr_data.resize(0);
		curr_url = KURL();
		mime_type = QString::null;
	}
}

#include "htmlpart.moc"
