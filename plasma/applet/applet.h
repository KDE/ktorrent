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
#ifndef KTPLASMAAPPLET_H
#define KTPLASMAAPPLET_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>

class QGraphicsLinearLayout;

namespace Plasma
{
	class Icon;
	class Meter;
}


namespace ktplasma
{

	/**
		Plasma applet for ktorrent
	*/	
	class Applet : public Plasma::Applet
	{
		Q_OBJECT

	public:
		Applet(QObject *parent, const QVariantList &args);
		virtual ~Applet();
		
		virtual void init();
		virtual void constraintsEvent(Plasma::Constraints constraints);
		virtual void createConfigurationInterface(KConfigDialog *parent);

	public slots:
		void dataUpdated(const QString &name,
						const Plasma::DataEngine::Data &data);

	private:
		Plasma::Icon* icon;
		Plasma::Meter* upload_speed;
		Plasma::Meter* download_speed;
		QGraphicsLinearLayout* meter_layout;
	};

}
 


#endif
