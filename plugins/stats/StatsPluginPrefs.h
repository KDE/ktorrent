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

#ifndef STATSPLUGINPREFS_H_
#define STATSPLUGINPREFS_H_

#include <qspinbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <interfaces/prefpageinterface.h>

#include "StatsPluginPrefsPage.h"
#include "statspluginsettings.h"

namespace kt {

/**
\brief Prefs page
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class StatsPluginPrefs : public QObject, public PrefPageInterface
{
	Q_OBJECT
	
	private:
		///Widget
		StatsPluginPrefsPage *pmUi;
	public:
		///Constructor
		StatsPluginPrefs();
		///Destructor
		virtual ~StatsPluginPrefs();
		
		virtual bool 	apply ();
		virtual void 	createWidget (QWidget *parent);
		virtual void 	updateData ();
		virtual void 	deleteWidget ();
	signals:
		void Applied();
};

}

#endif
