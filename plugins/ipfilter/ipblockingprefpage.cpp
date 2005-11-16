/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "ipblockingprefpage.h"
#include "ipblockingpref.h"
#include "ipfilterpluginsettings.h"
#include "ipfilterplugin.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kurlrequester.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kprogress.h>

#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/coreinterface.h>

#include <qthread.h>
#include <qlabel.h>

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

	LoadingThread::LoadingThread(CoreInterface* core) : QThread()
	{
		m_core = core;
		this->start(QThread::LowPriority);
	}
	
	LoadingThread::~ LoadingThread()
	{}
	
	void LoadingThread::run()
	{
		QString filter = IPBlockingPluginSettings::filterFile();
		if(!filter.isEmpty())
		{
			//load list
			QString listURL = IPBlockingPluginSettings::filterFile();
			QFile dat(listURL);
			dat.open(IO_ReadOnly);

			QString trt;
			QTextStream stream( &dat );
			QString line;
			int i=0;
			while ( !stream.atEnd() )
			{
				line = stream.readLine();
				m_core->addBlockedIP(line);
				++i;
			}
			Out() << "Loaded " << i << " blocked IP ranges." << endl;
			dat.close();
		}
		delete this;
	}
	
	ConvertThread::ConvertThread(KProgress* kp, QLabel* lbl) : QThread()
	{
		progress = kp;
		lblProgress = lbl;
	}
	
	void ConvertThread::run()
	{
		QFile source(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.txt");
		QFile target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat");
		
		/**    READ INPUT FILE  **/
		QStringList list;
		lblProgress->setText("Loading txt file...");
		progress->show();
		if ( source.open( IO_ReadOnly ) ) 
		{	
			QTextStream stream( &source );
		
			int i = 0;
			while ( !stream.atEnd() ) {
				list += stream.readLine().section( ':' , -1 );
				if(i%908==0)//hardcoded value for level1.txt since I cannot know how many lines are there...
					progress->setProgress(i/908);
				++i;
			}
			source.close();
		} 
		else
			Out() << "Cannot find level1.txt" << endl;
		
		lblProgress->setText("Converting...");
		
		ulong blocks = list.count();
	
		/** WRITE TO OUTPUT **/
		if (!target.open( IO_WriteOnly ))
		{
			Out() << "Unable to open file for writing" << endl;
			return ;
		}
		
		Out() << "Loading finished. Starting conversion..." << endl;
		
		for(ulong i=0; i<blocks; ++i)
		{
			ipblock block = toBlock(list[i]);
			target.writeBlock( (char*) &block, sizeof(ipblock) );
			if(i%1000 == 0)
			{
				progress->setProgress((int)100*i/blocks);
				if(i%10000==0)
					Out() << "Block " << i << " written." << endl;
			}
			if(i==30000)
				continue;
		}
		Out() << "Finished converting." << endl;
		lblProgress->setText("File converted.");
	
		target.close();
		progress->hide();
		delete this;
	}
	
	IPBlockingPrefPageWidget::IPBlockingPrefPageWidget(QWidget* parent) : IPBlockingPref(parent)
	{
		m_filter->setURL(IPBlockingPluginSettings::filterFile());
		m_url->setURL(IPBlockingPluginSettings::filterURL());
		if (m_url->url() == "")
			m_url->setURL(QString("http://www.bluetack.co.uk/config/antip2p.txt"));
		kProgress1->hide();
	}

	void IPBlockingPrefPageWidget::apply()
	{
		KURLRequester* filter = m_filter;
		IPBlockingPluginSettings::setFilterFile(filter->url());
		IPBlockingPluginSettings::setFilterURL(m_url->url());
		IPBlockingPluginSettings::writeConfig();
	}

	void IPBlockingPrefPageWidget::btnDownload_clicked()
	{
		QString target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.txt");
		QFile target_file(target);
		KURL url(m_url->url());
		
		bool download = true;
		
		if(target_file.exists())
			if((KMessageBox::questionYesNo(this, i18n("Selected file already exists, do you want to download it again?"),i18n("File exists")) == 4))
				download = false;
		
		if(download)
		{
			if (KIO::NetAccess::download(url,target,NULL))
			{
				//Level1 list successfully downloaded, remove temporary file
				KIO::NetAccess::removeTempFile(target);
			}
			else
			{
				KMessageBox::error(0,KIO::NetAccess::lastErrorString(),i18n("Error"));
			}
		}
		convert();
	}

	void IPBlockingPrefPageWidget::convert()
	{	
		QFile target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat");
		if(target.exists())
		{
			if((KMessageBox::questionYesNo(this,i18n("Filter file (level1.dat) already exists, do you want to convert it again?"),i18n("File exists")) == 4))
				return;
		}
		ConvertThread* ct = new ConvertThread(kProgress1, lbl_progress);
		ct->start(QThread::LowPriority);
	}

	IPBlockingPrefPage::IPBlockingPrefPage(CoreInterface* core)
	: PrefPageInterface(i18n("IPBlocking filter"), i18n("IPBlocking filter Options"), KGlobal::iconLoader()->loadIcon("filter",KIcon::NoGroup)), m_core(core)
	{
		widget = 0;
	}

	IPBlockingPrefPage::~IPBlockingPrefPage()
	{}

	void IPBlockingPrefPage::apply()
	{
		widget->apply();
		LoadingThread* filters = new LoadingThread(this->m_core);
	}
	
	void IPBlockingPrefPage::loadFilters()
	{
		LoadingThread* filters = new LoadingThread(this->m_core);
	}

	void IPBlockingPrefPage::createWidget(QWidget* parent)
	{
		widget = new IPBlockingPrefPageWidget(parent);
	}

	void IPBlockingPrefPage::deleteWidget()
	{
		delete widget;
		widget = 0;
	}

	void IPBlockingPrefPage::updateData()
	{}
}



