/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include "applet.h"
#include <math.h>
#include <QFile>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QGraphicsProxyWidget>
#include <KIcon>
#include <KIconLoader>
#include <KConfigDialog>
#include <KLocale>
#include <KRun>
#include <KWindowSystem>
#if (PLASMA_VERSION_MAJOR < 3)
#include <Plasma/Icon>
#else
#include <Plasma/IconWidget>
#endif
#include <Plasma/Meter>
#include <Plasma/Label>
#include <taskmanager/taskmanager.h>
#include <taskmanager/task.h>
#include <util/functions.h>
#include "chunkbar.h"


using namespace bt;

namespace ktplasma
{

	

	Applet::Applet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),icon(0)
	{
		KLocale::setMainCatalog("ktorrent");
		setAspectRatioMode(Plasma::ConstrainedSquare);
		int iconSize = IconSize(KIconLoader::Desktop);
		resize(iconSize * 4, iconSize * 2);
		engine = 0;
		root_layout = 0;
		connected_to_app = false;

		// drop data!
		if (!args.isEmpty()) 
		{
			QFile f(args[0].toString());
			if (f.open(QIODevice::ReadOnly)) 
			{
				QDataStream s(&f);
				s >> current_source;
			}
		}
	}


	Applet::~Applet()
	{
	}
	
	void Applet::init()
	{
		engine = dataEngine("ktorrent");
		
		connect(engine,SIGNAL(sourceAdded(const QString &)),this,SLOT(sourceAdded(const QString&)));
		connect(engine,SIGNAL(sourceRemoved(const QString &)),this,SLOT(sourceRemoved(const QString&)));

		setHasConfigurationInterface(true);
		
		root_layout = new QGraphicsLinearLayout(this);
		root_layout->setContentsMargins(0, 0, 0, 0);
		root_layout->setSpacing(0);
		root_layout->setOrientation(Qt::Vertical);
		
		QGraphicsLinearLayout* line = new QGraphicsLinearLayout(0);
		
#if (PLASMA_VERSION_MAJOR < 3)
		icon = new Plasma::Icon(KIcon("ktorrent"),QString(),this);
#else
		icon = new Plasma::IconWidget(KIcon("ktorrent"),QString(),this);
#endif
		int icon_size = IconSize(KIconLoader::Desktop);
		icon->setMaximumSize(icon_size,icon_size);
		icon->setMinimumSize(icon_size,icon_size);
		icon->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		connect(icon, SIGNAL(clicked()), this, SLOT(iconClicked()));
		
		title = new Plasma::Label(this);
		title->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		line->addItem(icon);
		line->addItem(title);
		root_layout->addItem(line);
		
		chunk_bar = new ChunkBar(this);
		root_layout->addItem(chunk_bar);

		misc = new Plasma::Label(this);
		root_layout->addItem(misc);
		
		clearData();
		resize(icon_size * 8,root_layout->preferredHeight());
		
		if (current_source.isNull()) 
		{
			current_source = selectTorrent();
		} 
		else 
		{
			QStringList sources = engine->sources();
			bool found = false;
			foreach (const QString & s,sources)
			{
				QString name = engine->query(s).value("name").toString();
				if (name == current_source) {
					current_source = s;
					found = true;
					break;
				}
			}

			if (!found)
			    current_source = selectTorrent();
		}
		
		if (!current_source.isNull())
		{
			connected_to_app = true;
			engine->connectSource(current_source,this,1000);
		}
		else
		{
			connected_to_app = engine->query("core").value("connected").toBool();
			if (!connected_to_app)
				title->setText(i18n("KTorrent is not running !"));
			else
				title->setText(i18n("No torrents loaded !"));
		}

		engine->connectSource("core",this);
	}
	
	QString Applet::selectTorrent()
	{
		QStringList sources = engine->sources();
		foreach (const QString & s,sources)
			if (s != "core")
				return s;
		
		return QString();
	}
	
	void Applet::constraintsEvent(Plasma::Constraints constraints)
	{
		if (constraints & Plasma::FormFactorConstraint) 
		{
			if (formFactor() == Plasma::Vertical) 
			{
				
			} 
			else if (formFactor() == Plasma::Horizontal) 
			{

			}
		}

		if (constraints & (Plasma::SizeConstraint | Plasma::FormFactorConstraint)) 
		{
		}
	}
	
	void Applet::createConfigurationInterface(KConfigDialog *parent)
	{
		QWidget *widget = new QWidget();
		ui.setupUi(widget);
		parent->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
		parent->addPage(widget, parent->windowTitle(), "ktorrent");
		connect(parent, SIGNAL(applyClicked()), this, SLOT(configUpdated()));
		connect(parent, SIGNAL(okClicked()), this, SLOT(configUpdated()));
		updateTorrentCombo();
	}
	
	void Applet::updateTorrentCombo()
	{	
		QStringList sources = engine->sources();
		ui.torrent_to_display->clear();
		ui.torrent_to_display->setEnabled(sources.count() > 0);
		if (sources.count() == 0)
			return;
		
		QStringList names;
		foreach (const QString & s,sources)
		{
			if (s != "core")
				names << engine->query(s).value("name").toString();
		}
		ui.torrent_to_display->addItems(names);
		
		if (current_source.isNull())
		{
			current_source = selectTorrent();
			if (!current_source.isNull())
				engine->connectSource(current_source,this,1000);
			else
				clearData();
		}
	}
	
	void Applet::configUpdated()
	{
		QString name = ui.torrent_to_display->currentText();
		if (!current_source.isNull())
		{
			engine->disconnectSource(current_source,this);
			current_source = QString();
			clearData();
		}
		
		QStringList sources = engine->sources();
		foreach (const QString & s,sources)
		{
			if (s != "core" && engine->query(s).value("name").toString() == name)
			{
				current_source = s;
				engine->connectSource(current_source,this,1000);
				break;
			}
		}
	}

	void Applet::dataUpdated(const QString &name,const Plasma::DataEngine::Data &data)
	{
		if (name == "core")
		{
			if (!connected_to_app && data.value("connected").toBool())
			{
				connected_to_app = true;
				current_source = selectTorrent();
				if (!current_source.isEmpty())
				{
					engine->connectSource(current_source,this,1000);
				}
				else
				{
					title->setText(i18n("No torrents loaded !"));
					clearData();
				}
			}
			else if (connected_to_app && !data.value("connected").toBool())
			{
				connected_to_app = false;
				current_source = QString();
				title->setText(i18n("KTorrent is not running !"));
				clearData();
			}
		}
		else if (name == current_source)
		{
			updateCurrent(data);
		}
	}
	
	void Applet::updateCurrent(const Plasma::DataEngine::Data &data)
	{
		double ds = data.value("download_rate").toDouble();
		double us = data.value("upload_rate").toDouble();
		int uploaded = data.value("bytes_uploaded").toInt();
		int downloaded = data.value("bytes_downloaded").toInt();
		int size = data.value("total_bytes_to_download").toInt();		
		int st = data.value("seeders_total").toInt();
		int sc = data.value("seeders_connected_to").toInt();
		int ct = data.value("leechers_total").toInt();
		int cc = data.value("leechers_connected_to").toInt();
		KLocale* loc = KGlobal::locale();
		float share_ratio = (downloaded == 0) ? 0 : (float)uploaded/downloaded;
		misc->setText(
			i18n(
				 "<table>\
				<tr><td>Download Speed:</td><td>%5 KB/s </td><td>Seeders: </td><td>%1 (%2)</td></tr>\
				<tr><td>Upload Speed:</td><td>%6 KB/s </td><td>Leechers: </td><td>%3 (%4)</td></tr>\
				<tr><td>Downloaded:</td><td>%7 / %8 </td><td>Uploaded: </td><td>%9</td></tr>\
				</table>",
	 			sc,st,cc,ct,loc->formatNumber(ds / 1024.0,2),loc->formatNumber(us / 1024.0,2),
				BytesToString(downloaded,2),BytesToString(size,2),BytesToString(uploaded,2)));
		
		
		
		QString t = i18n("<b>%1</b><br/>%2 (Share Ratio: <font color=\"%4\">%3</font>)",
						data.value("name").toString(),
						data.value("status").toString(),
						loc->formatNumber(share_ratio,2),
						share_ratio <= 0.8 ? "#ff0000" : "#1c9a1c");
		
		title->setText(t);
		
		chunk_bar->updateBitSets(
			data.value("total_chunks").toInt(),
			data.value("downloaded_chunks").toByteArray(),
			data.value("excluded_chunks").toByteArray());
	}
	
	void Applet::sourceAdded(const QString & s)
	{
		Q_UNUSED(s);
		if (current_source.isNull())
		{
			current_source = selectTorrent();
			if (!current_source.isNull())
				engine->connectSource(current_source,this,1000);
			else
				clearData();
		}
	}
	
	void Applet::sourceRemoved(const QString & s)
	{
		if (current_source == s)
		{
			current_source = selectTorrent();
			if (!current_source.isNull())
				engine->connectSource(current_source,this,1000);
			else
				clearData();
		}
	}

	void Applet::iconClicked()
	{
		TaskManager::TaskDict tasks = TaskManager::TaskManager::self()->tasks();
		for (TaskManager::TaskDict::iterator i = tasks.begin();i != tasks.end();i ++)
		{
			if (i.value()->className() == "ktorrent")
			{
				KWindowSystem::activateWindow(i.key());
				return;
			}
		}

		// can't find the window, try launching it
		KUrl::List empty;
		KRun::run("ktorrent", empty, 0);
	}

	void Applet::clearData()
	{
		KLocale* loc = KGlobal::locale();		
		misc->setText(
			i18n(
				"<table>\
				<tr><td>Download Speed:</td><td>%5 KB/s </td><td>Seeders: </td><td>%1 (%2)</td></tr>\
				<tr><td>Upload Speed:</td><td>%6 KB/s </td><td>Leechers: </td><td>%3 (%4)</td></tr>\
				<tr><td>Downloaded:</td><td>%7 / %8 </td><td>Uploaded: </td><td>%9</td></tr>\
				</table>",
				0,0,0,0,loc->formatNumber(0,2),loc->formatNumber(0,2),
				BytesToString(0,2),BytesToString(0,2),BytesToString(0,2)));
	}
}

K_EXPORT_PLASMA_APPLET(ktorrent, ktplasma::Applet)
