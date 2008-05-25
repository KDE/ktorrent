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
#ifndef KTADDITEMDLG_H
#define KTADDITEMDLG_H

#include <KDialog>
#include "ui_additemdlg.h"

namespace kt
{
	struct ScheduleItem;

	/**
		@author
	*/
	class AddItemDlg : public KDialog, public Ui_AddItemDlg
	{
		Q_OBJECT
	public:
		enum Mode
		{
			NEW_ITEM,
			EDIT_ITEM
		};
		
		AddItemDlg(Mode mode,QWidget* parent);
		virtual ~AddItemDlg();
		
		/**
		 * Execute the dialog
		 * @param item The item to fill in if the user presses OK
		 * @return true if OK was pressed
		 */
		bool execute(ScheduleItem* item);

	private slots:
		void fromChanged(const QTime & time);
		void toChanged(const QTime & time);
	};

}

#endif
