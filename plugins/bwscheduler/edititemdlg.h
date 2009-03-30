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
#ifndef KTEDITITEMDLG_H
#define KTEDITITEMDLG_H

#include <KDialog>
#include "ui_edititemdlg.h"

namespace kt
{
	struct ScheduleItem;

	/**
		@author
	*/
	class EditItemDlg : public KDialog, public Ui_EditItemDlg
	{
		Q_OBJECT
	public:		
		EditItemDlg(QWidget* parent);
		virtual ~EditItemDlg();
		
		/**
		 * Execute the dialog
		 * @param item The item to fill in if the user presses OK
		 * @return true if OK was pressed
		 */
		bool execute(ScheduleItem* item);

	private slots:
		void fromChanged(const QTime & time);
		void toChanged(const QTime & time);
		void pausedChanged(bool on);
		void screensaverLimitsToggled(bool on);
	};

}

#endif
