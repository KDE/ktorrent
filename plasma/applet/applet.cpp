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
#include <Plasma/Icon>
#include <Plasma/Meter>
#include <Plasma/Label>
#include <taskmanager/taskmanager.h>
#include <taskmanager/task.h>
#include <util/functions.h>
#include "applet.h"
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
		max_us = max_ds = 0;
		root_layout = 0;
		connected_to_app = config_dlg_created = false;

		// drop data!
		if (!args.isEmpty()) {
			QFile f(args[0].toString());
			if (f.open(QIODevice::ReadOnly)) {
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
		
		icon = new Plasma::Icon(KIcon("ktorrent"),QString(),this);
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
		
		QGraphicsGridLayout* grid = new QGraphicsGridLayout(0);
		upload_speed_meter = new Plasma::Meter(this);
		upload_speed_meter->setMeterType(Plasma::Meter::BarMeterHorizontal);
		upload_speed_meter->setMinimum(0);
		upload_speed_meter->setMaximum(100);
		upload_speed_meter->setValue(50);
		upload_speed = new Plasma::Label(this);
		grid->addItem(upload_speed_meter,0,0,Qt::AlignVCenter);
		grid->addItem(upload_speed,0,1,Qt::AlignVCenter);

		download_speed_meter = new Plasma::Meter(this);
		download_speed_meter->setMeterType(Plasma::Meter::BarMeterHorizontal);
		download_speed_meter->setMinimum(0);
		download_speed_meter->setMaximum(100);
		download_speed_meter->setValue(50);
		download_speed = new Plasma::Label(this);
		grid->addItem(download_speed_meter,1,0,Qt::AlignVCenter);
		grid->addItem(download_speed,1,1,Qt::AlignVCenter);
		root_layout->addItem(grid);
		
		label = new Plasma::Label(this);
		
		root_layout->addItem(label);
		root_layout->setAlignment(label,Qt::AlignHCenter|Qt::AlignVCenter);
		
		KLocale* loc = KGlobal::locale();
		upload_speed->setText(i18n("Up: %1 KB/s",loc->formatNumber(0,2)));
		download_speed->setText(i18n("Down: %1 KB/s",loc->formatNumber(0,2)));
		
		clearData();
		
		resize(icon_size * 8,root_layout->preferredHeight());
		
		if (current_source.isNull()) {
			current_source = selectTorrent();
		} else {
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

			if (!found) {
			    current_source = selectTorrent();
			}
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
		config_dlg_created = true;
		updateTorrentCombo();
	}
	
	void Applet::updateTorrentCombo()
	{
		if (!config_dlg_created)
			return;
		
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
				updateTorrentCombo();
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
		if (ds > max_ds)
			max_ds = ds;
		if (us > max_us)
			max_us = us;
		
		KLocale* loc = KGlobal::locale();
		upload_speed->setText(i18n("Up: %1 KB/s",loc->formatNumber(us / 1024.0,2)));
		download_speed->setText(i18n("Down: %1 KB/s",loc->formatNumber(ds / 1024.0,2)));
		
		if (max_us > 0)
		{
			int v = (int)ceil((us / max_us) * 100);
			if (v > 100)
				v = 100;
			upload_speed_meter->setValue(v);
		}
		else
			upload_speed_meter->setValue(0);
		
		if (max_ds > 0)
		{
			int v = (int)ceil((ds / max_ds) * 100);
			if (v > 100)
				v = 100;
			download_speed_meter->setValue(v);
		}
		else
			download_speed_meter->setValue(0);
		
		QString t = QString("<b>%1</b><br/>%2").arg(data.value("name").toString()).arg(data.value("status").toString());
		title->setText(t);
		
		int uploaded = data.value("bytes_uploaded").toInt();
		int downloaded = data.value("bytes_downloaded").toInt();
		int size = data.value("total_bytes_to_download").toInt();
		label->setText(i18n("Downloaded: %1 / %2 Uploaded: %3",
					   BytesToString(downloaded,2),BytesToString(size,2),BytesToString(uploaded,2)));
		
		chunk_bar->updateBitSets(
			data.value("total_chunks").toInt(),
			data.value("downloaded_chunks").toByteArray(),
			data.value("excluded_chunks").toByteArray());
	}
	
	void Applet::sourceAdded(const QString & s)
	{
		Q_UNUSED(s);
		updateTorrentCombo();
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
		updateTorrentCombo();
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
		upload_speed->setText(i18n("Up: %1 KB/s",loc->formatNumber(0,2)));
		upload_speed_meter->setValue(0);
		download_speed->setText(i18n("Down: %1 KB/s",loc->formatNumber(0,2)));
		download_speed_meter->setValue(0);
		label->setText(QString());
	}
}

K_EXPORT_PLASMA_APPLET(ktorrent, ktplasma::Applet)
