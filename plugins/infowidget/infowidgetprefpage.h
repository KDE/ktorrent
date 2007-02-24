/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTINFOWIDGETPREFPAGE_H
#define KTINFOWIDGETPREFPAGE_H

#include <interfaces/prefpageinterface.h>

class IWPref;

namespace kt
{
	class InfoWidgetPlugin;
	

	/**
	@author Joris Guisson
	*/
	class InfoWidgetPrefPage : public PrefPageInterface
	{
		InfoWidgetPlugin* iw;
		IWPref* pref;
	public:
		InfoWidgetPrefPage(InfoWidgetPlugin* iw);
		virtual ~InfoWidgetPrefPage();

		virtual bool apply();
		virtual void createWidget(QWidget* parent);
		virtual void deleteWidget();
		virtual void updateData();

	};

}

#endif
