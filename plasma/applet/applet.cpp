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
#include <QGraphicsLinearLayout>
#include <kicon.h>
#include <kiconloader.h>
#include <plasma/widgets/icon.h>
#include <plasma/widgets/meter.h>
#include "applet.h"



namespace ktplasma
{

	Applet::Applet(QObject *parent, const QVariantList &args) : Plasma::Applet(parent, args),icon(0)
	{
		setAspectRatioMode(Plasma::ConstrainedSquare);
		int iconSize = IconSize(KIconLoader::Desktop);
		resize(iconSize * 4, iconSize * 2);
	}


	Applet::~Applet()
	{
	}
	
	void Applet::init()
	{
		setHasConfigurationInterface(true);
		
		QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
		
		//connect(icon, SIGNAL(clicked()), this, SLOT(pressed()));
		icon = new Plasma::Icon(KIcon("ktorrent"),QString(),this);
		
		
		upload_speed = new Plasma::Meter(this);
		upload_speed->setMeterType(Plasma::Meter::BarMeterVertical);
		upload_speed->setMinimum(0);
		upload_speed->setMaximum(100);
		upload_speed->setValue(50);
		
		download_speed = new Plasma::Meter(this);
		download_speed->setMeterType(Plasma::Meter::BarMeterVertical);
		download_speed->setMinimum(0);
		download_speed->setMaximum(100);
		download_speed->setValue(50);
		
		//meter_layout = new QGraphicsLinearLayout(this);
		layout->addItem(download_speed);
		layout->addItem(icon);
		layout->addItem(upload_speed);
		/*layout->addItem(meter_layout);
		// If we are vertically constrained, draw the meters horizontally
		// otherwise make them vertically
		if (formFactor() == Plasma::Vertical)
		{
			meter_layout->setOrientation(Qt::Vertical);
		}
		else
		{
			meter_layout->setOrientation(Qt::Horizontal);
		}
		*/
	}
	
	void Applet::constraintsEvent(Plasma::Constraints constraints)
	{
	}
	
	void Applet::createConfigurationInterface(KConfigDialog *parent)
	{
	}

	void Applet::dataUpdated(const QString &name,const Plasma::DataEngine::Data &data)
	{
	}

}

K_EXPORT_PLASMA_APPLET(ktorrent, ktplasma::Applet);