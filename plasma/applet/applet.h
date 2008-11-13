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

#include <plasma/version.h>
#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include "ui_appletconfig.h"

class QGraphicsLinearLayout;

namespace Plasma
{
#if (PLASMA_VERSION_MAJOR < 3)
	class Icon;
#else 
	class IconWidget;
#endif
	class Meter;
	class Label;
}


namespace ktplasma
{
	class ChunkBar;

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

	private slots:
		void dataUpdated(const QString &name,const Plasma::DataEngine::Data &data);
		void configUpdated();
		void sourceAdded(const QString & s);
		void sourceRemoved(const QString & s);
		void iconClicked();
		
	private:
		void updateTorrentCombo();
		void updateCurrent(const Plasma::DataEngine::Data &data);
		QString selectTorrent();
		void clearData();

	private:
#if (PLASMA_VERSION_MAJOR < 3)
		Plasma::Icon* icon;
#else 
		Plasma::IconWidget* icon;
#endif
		Plasma::Label* title;
		Plasma::Label* misc;
		Ui_AppletConfig ui;
		Plasma::DataEngine* engine;
		QString current_source;
		QGraphicsLinearLayout* root_layout;
		bool connected_to_app;
		bool config_dlg_created;
		ChunkBar* chunk_bar;
	};

}
 


#endif
