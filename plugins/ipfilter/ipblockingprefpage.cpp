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
#include <qcheckbox.h>

using namespace bt;

#define MAX_RANGES 500

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

	ConvertThread::ConvertThread(KProgress* kp, QLabel* lbl, IPFilterPlugin* p) : QThread()
	{
		progress = kp;
		lblProgress = lbl;
		m_plugin = p;
		
		//Unload level1 filter to prevent eventual crashes
		if(m_plugin)
			m_plugin->unloadAntiP2P();
	}

	void ConvertThread::run()
	{
		QFile source(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.txt");
		QFile target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat");

		/**    READ INPUT FILE  **/
		QStringList list;
		lblProgress->setText("Loading txt file...");
		progress->show();
		ulong source_size = source.size();
		if ( source.open( IO_ReadOnly ) )
		{
			QTextStream stream( &source );

			int i = 0;
			while ( !stream.atEnd() ) {
				QString line = stream.readLine();
				list += line.section( ':' , -1 );
				i += line.length() * sizeof(char); //rough estimation of string size
				progress->setProgress(i*100/source_size);
				++i;
			}
			source.close();
		}
		else
		{
			Out() << "Cannot find level1.txt" << endl;
			return;
		}

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
		//reload level1 filter
		if(m_plugin)
			m_plugin->loadAntiP2P();
		delete this;
	}

	IPBlockingPrefPageWidget::IPBlockingPrefPageWidget(QWidget* parent) : IPBlockingPref(parent)
	{
		m_filter->setURL(IPBlockingPluginSettings::filterFile());
		m_url->setURL(IPBlockingPluginSettings::filterURL());
		if (m_url->url() == "")
			m_url->setURL(QString("http://www.bluetack.co.uk/config/antip2p.txt"));
		
		bool use_level1 = IPBlockingPluginSettings::useLevel1();
		bool use_filter = IPBlockingPluginSettings::useFilter();
		checkUseLevel1->setChecked(use_level1);
		checkUseKTfilter->setChecked(use_filter);
		
		if(use_filter)
		{
			lbl_status1->setText(i18n("Status: Loaded and running."));
			m_url->setEnabled(true);
			btnDownload->setEnabled(true);
		}
		else
		{
			lbl_status1->setText(i18n("Status: Not loaded."));
			m_url->setEnabled(false);
			btnDownload->setEnabled(false);
		}
		
		if(use_level1)
		{
			lbl_status2->setText(i18n("Status: Loaded and running."));
			m_filter->setEnabled(true);
		}
		else
		{
			lbl_status2->setText(i18n("Status: Not loaded."));
			m_filter->setEnabled(false);
		}
		
		kProgress1->hide();
		m_plugin = 0;
	}

	void IPBlockingPrefPageWidget::apply()
	{
		KURLRequester* filter = m_filter;
		IPBlockingPluginSettings::setFilterFile(filter->url());
		IPBlockingPluginSettings::setFilterURL(m_url->url());
		IPBlockingPluginSettings::setUseLevel1(checkUseLevel1->isChecked());
		IPBlockingPluginSettings::setUseFilter(checkUseKTfilter->isChecked());
		IPBlockingPluginSettings::writeConfig();
		
		if(checkUseLevel1->isChecked())
		{
			QFile target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat");
			if(target.exists())
				lbl_status1->setText(i18n("Status: Loaded and running."));
			else
				lbl_status1->setText(i18n("Status: <font color=\"#ff0000\">Filter file not found.</font> Download and convert filter file."));
		}
		else
			lbl_status1->setText(i18n("Status: Not loaded."));
		
		if(checkUseKTfilter->isChecked())
		{
			QString filter = IPBlockingPluginSettings::filterFile();
			if(!filter.isEmpty())
			{
				lbl_status2->setText("Status: Loaded and running.");
			}
			else
				lbl_status2->setText("Status: <font color=\"#ff0000\">Filter file not found.</font> Choose one.");
		}
		else
			lbl_status2->setText(i18n("Status: Not loaded."));
	}

	void IPBlockingPrefPageWidget::btnDownload_clicked()
	{
		QString target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.txt");
		QFile target_file(target);
		KURL url(m_url->url());

		bool download = true;

		if(target_file.exists())
			if((KMessageBox::questionYesNo(this, i18n("Selected file already exists, do you want to download it again?"),i18n("File Exists")) == 4))
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
	
	void IPBlockingPrefPageWidget::checkUseLevel1_toggled(bool check)
	{
		if(check)
		{
			m_url->setEnabled(true);
			btnDownload->setEnabled(true);
		}
		else
		{
			lbl_status1->setText("");
			m_url->setEnabled(false);
			btnDownload->setEnabled(false);
		}
	}
	
	void IPBlockingPrefPageWidget::checkUseKTfilter_toggled(bool check)
	{
		if(check)
		{
			m_filter->setEnabled(true);
		}
		else
		{
			lbl_status2->setText("");
			m_filter->setEnabled(false);
		}
	}


	void IPBlockingPrefPageWidget::convert()
	{
		QFile target(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat");
		if(target.exists())
		{
			if((KMessageBox::questionYesNo(this,i18n("Filter file (level1.dat) already exists, do you want to convert it again?"),i18n("File Exists")) == 4))
				return;
		}
		ConvertThread* ct = new ConvertThread(kProgress1, lbl_progress, m_plugin);
		ct->start(QThread::LowPriority);
	}
	
	void IPBlockingPrefPageWidget::setPlugin(IPFilterPlugin* p)
	{
		m_plugin = p;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////////

	IPBlockingPrefPage::IPBlockingPrefPage(CoreInterface* core, IPFilterPlugin* p)
	: PrefPageInterface(i18n("IPBlocking Filter"), i18n("IPBlocking Filter Options"), KGlobal::iconLoader()->loadIcon("filter",KIcon::NoGroup)), m_core(core), m_plugin(p)
	{
		widget = 0;
	}

	IPBlockingPrefPage::~IPBlockingPrefPage()
	{}

	bool IPBlockingPrefPage::apply()
	{
		widget->apply();
		
		if(IPBlockingPluginSettings::useFilter())
			loadFilters();
		else
			unloadFilters();
		
		if(IPBlockingPluginSettings::useLevel1())
			m_plugin->loadAntiP2P();
		else
			m_plugin->unloadAntiP2P();
		
		return true;
	}

	void IPBlockingPrefPage::loadFilters()
	{
		/** LOAD KT FILTER LIST **/
		QString filter = IPBlockingPluginSettings::filterFile();
		if(!filter.isEmpty())
		{
			//load list
			QString listURL = IPBlockingPluginSettings::filterFile();
			QFile dat(listURL);
			dat.open(IO_ReadOnly);

			QTextStream stream( &dat );
			QString line;
			int i=0;
			while ( !stream.atEnd() && i < MAX_RANGES )
			{
				line = stream.readLine();
				m_core->addBlockedIP(line);
				++i;
			}
			Out() << "Loaded " << i << " blocked IP ranges." << endl;
			dat.close();
		}
		
	}

	void IPBlockingPrefPage::unloadFilters()
	{
		/** UNLOAD KT FILTER LIST **/
		QString filter = IPBlockingPluginSettings::filterFile();
		if(!filter.isEmpty())
		{
			//unload list
			QString listURL = IPBlockingPluginSettings::filterFile();
			QFile dat(listURL);
			dat.open(IO_ReadOnly);

			QTextStream stream( &dat );
			QString line;
			int i=0;
			while ( !stream.atEnd() && i < MAX_RANGES )
			{
				line = stream.readLine();
				m_core->removeBlockedIP(line);
				++i;
			}
			Out() << "Unloaded " << i << " blocked IP ranges." << endl;
			dat.close();
		}
		
	}
	
	void IPBlockingPrefPage::createWidget(QWidget* parent)
	{
		widget = new IPBlockingPrefPageWidget(parent);
		widget->setPlugin(m_plugin);
	}

	void IPBlockingPrefPage::deleteWidget()
	{
		delete widget;
		widget = 0;
	}

	void IPBlockingPrefPage::updateData()
	{}
}



