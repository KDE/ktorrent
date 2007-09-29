/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KT_PREFDIALOG_HH
#define KT_PREFDIALOG_HH

#include <QMap>
#include <kconfigdialog.h>

namespace kt
{
	class Core;
	class PrefPageInterface;

	/**
	 * KTorrent's preferences dialog, this uses the new KConfigDialog class which should make our live much easier.
	 * In order for this to work properly the widgets have to be named kcfg_FOO where FOO is a somehting out of our settings class.
	 *
	 * The use of KConfigDialog should deprecate PrefPageInterface.
	 * */
	class PrefDialog : public KConfigDialog
	{
		Q_OBJECT
	public:
		PrefDialog(QWidget* parent,Core* core);
		virtual ~PrefDialog();

		/**
		 * Add a pref page to the dialog.
		 * @param page The page
		 * */
		void addPrefPage(PrefPageInterface* page);

		/**
		 * Remove a pref page.
		 * @param page The page
		 * */
		void removePrefPage(PrefPageInterface* page);

	protected:
		virtual void updateWidgets();
		virtual void updateWidgetsDefault();
		virtual void updateSettings();

	private:
		QMap<PrefPageInterface*,KPageWidgetItem*> pages;
	};
}


#endif
