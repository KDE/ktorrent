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

#ifndef SettingsPage_H_
#define SettingsPage_H_

#include <QWidget>

#include <klocale.h>
#include <kcolorbutton.h>

#include <interfaces/prefpageinterface.h>

#include <PluginPage.h>
#include <statspluginsettings.h>

#include <ui_Settings.h>

namespace kt {

/** \brief Settings page
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class SettingsPage : public PrefPageInterface, public Ui_StatsSettingsWgt
{
	Q_OBJECT
	
	public:
		/** \brief Constructor
		\param  p Parent
		*/
		SettingsPage(QWidget * p);
		///Destructor
		virtual ~SettingsPage();
	
	public slots:
		void updateSettings();
	
	signals:
		///Settings has been applied
		void Applied();
		
};

} //ns end

#endif
