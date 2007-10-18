/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef StatsPlugin_H_
#define StatsPlugin_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <kgenericfactory.h>
#include <klocale.h>

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

#include <memory>

#include <SpdTabPage.h>
#include <ConnsTabPage.h>
#include <SettingsPage.h>
#include <statspluginsettings.h>

namespace kt {

/** \brief Statistics plugin
\author Krzysztof Kundzicz <athantor@gmail.com>
\version 1.1
*/

class StatsPlugin : public Plugin
{
	Q_OBJECT
	
	private:
		///Speeds tab
		std::auto_ptr<SpdTabPage> pmUiSpd;
		///Connections tab
		std::auto_ptr<ConnsTabPage> pmUiConns;
		///Settings Page
		SettingsPage* pmUiSett;
		///Timer
		std::auto_ptr<QTimer> pmTmr;
		
		///Updates counter
		uint32_t mUpdCtr;
		
	public:
		/** \brief Constructor
		\param p Parent
		*/
		StatsPlugin(QObject * p, const QStringList&);
		///Destructor
		~StatsPlugin();
		
		void load();
  		void unload();
  		bool versionCheck(const QString& version) const;
  		void guiUpdate();
  	
  	public slots:
  		///Gather data
  		void DispatchDataGathering();
  		///Settings has been changed
  		void SettingsChanged();

};

} //ns end

#endif
