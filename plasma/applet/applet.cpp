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
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <kicon.h>
#include <kiconloader.h>
#include <kconfigdialog.h>
#include <klocale.h>
#include <plasma/widgets/icon.h>
#include <plasma/widgets/meter.h>
#include <plasma/widgets/label.h>
#include "applet.h"



namespace ktplasma
{
	const double TO_KB = 1024.0;
	const double TO_MEG = (1024.0 * 1024.0);
	const double TO_GIG = (1024.0 * 1024.0 * 1024.0);
	
	QString BytesToString(unsigned long long bytes,int precision)
	{
		KLocale* loc = KGlobal::locale();
		if (bytes >= 1024 * 1024 * 1024)
			return i18n("%1 GB",loc->formatNumber(bytes / TO_GIG,precision < 0 ? 2 : precision));
		else if (bytes >= 1024*1024)
			return i18n("%1 MB",loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
		else if (bytes >= 1024)
			return i18n("%1 KB",loc->formatNumber(bytes / TO_KB,precision < 0 ? 1 : precision));
		else
			return i18n("%1 B",bytes);
	}
	

	Applet::Applet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),icon(0)
	{
		setAspectRatioMode(Plasma::ConstrainedSquare);
		int iconSize = IconSize(KIconLoader::Desktop);
		resize(iconSize * 4, iconSize * 2);
		engine = 0;
		max_us = max_ds = 0;
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
		
		QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		layout->setOrientation(Qt::Vertical);
		
		QGraphicsLinearLayout* line = new QGraphicsLinearLayout(0);
		
		//connect(icon, SIGNAL(clicked()), this, SLOT(pressed()));
		icon = new Plasma::Icon(KIcon("ktorrent"),QString(),this);
		int icon_size = IconSize(KIconLoader::Desktop);
		icon->setMaximumSize(icon_size,icon_size);
		icon->setMinimumSize(icon_size,icon_size);
		icon->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
		
		title = new Plasma::Label(this);
		title->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
		line->addItem(icon);
		line->addItem(title);
		layout->addItem(line);
		
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
		layout->addItem(grid);
		
		label = new Plasma::Label(this);

		layout->addItem(label);
		layout->setAlignment(label,Qt::AlignHCenter);
		
		KLocale* loc = KGlobal::locale();
		upload_speed->setText(i18n("Up: %1 KB/s",loc->formatNumber(10000,2)));
		download_speed->setText(i18n("Down: %1 KB/s",loc->formatNumber(10000,2)));
		resize(icon_size * 8,layout->preferredHeight());
		
		QStringList sources = engine->sources();
		if (sources.count() > 0)
		{
			current_source = sources[0];
			engine->connectSource(current_source,this,1000);
		}
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
			names << engine->query(s).value("name").toString();
		}
		ui.torrent_to_display->addItems(names);
		
		if (current_source.isNull())
		{
			current_source = sources[0];
			engine->connectSource(current_source,this,1000);
		}
	}
	
	void Applet::configUpdated()
	{
		QString name = ui.torrent_to_display->currentText();
		if (!current_source.isNull())
		{
			engine->disconnectSource(current_source,this);
			current_source = QString();
		}
		
		QStringList sources = engine->sources();
		foreach (const QString & s,sources)
		{
			if (engine->query(s).value("name").toString() == name)
			{
				current_source = s;
				engine->connectSource(current_source,this,1000);
				break;
			}
		}
	}

	void Applet::dataUpdated(const QString &name,const Plasma::DataEngine::Data &data)
	{
		if (name != current_source)
			return;
		
		int ds = data.value("download_rate").toInt();
		int us = data.value("upload_rate").toInt();
		if (ds > max_ds)
			max_ds = ds;
		if (us > max_us)
			max_us = us;
		
		KLocale* loc = KGlobal::locale();
		upload_speed->setText(i18n("Up: %1 KB/s",loc->formatNumber(us / 1024.0,2)));
		download_speed->setText(i18n("Down: %1 KB/s",loc->formatNumber(ds / 1024.0,2)));
		
		if (max_us > 0)
			upload_speed_meter->setValue((int)ceil((us / max_us) * 100));
		else
			upload_speed_meter->setValue(0);
		
		if (max_ds > 0)
			download_speed_meter->setValue((int)ceil((ds / max_ds) * 100));
		else
			download_speed_meter->setValue(0);
		
		title->setText(data.value("name").toString());
		
		int uploaded = data.value("bytes_uploaded").toInt();
		int downloaded = data.value("bytes_downloaded").toInt();
		int size = data.value("total_bytes_to_download").toInt();
		label->setText(i18n("Downloaded: %1 / %2 Uploaded: %3",
					   BytesToString(downloaded,2),BytesToString(size,2),BytesToString(uploaded,2)));
	}
	
	void Applet::sourceAdded(const QString & s)
	{
		updateTorrentCombo();
		if (current_source.isNull())
		{
			current_source = s;
			engine->connectSource(current_source,this,1000);
		}
	}
	
	void Applet::sourceRemoved(const QString & s)
	{
		updateTorrentCombo();
		if (current_source == s)
		{
			QStringList sources = engine->sources();
			if (sources.count() > 0)
			{
				current_source = sources[0];
				engine->connectSource(current_source,this,1000);
			}
		}
	}

}

K_EXPORT_PLASMA_APPLET(ktorrent, ktplasma::Applet);