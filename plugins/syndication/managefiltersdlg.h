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
#ifndef KTMANAGEFILTERSDLG_H
#define KTMANAGEFILTERSDLG_H

#include <kdialog.h>
#include "ui_managefiltersdlg.h"

namespace kt
{

	/**
		Dialog to manage filters for a feed
	*/
	class ManageFiltersDlg : public KDialog,public Ui_ManageFiltersDlg
	{
		Q_OBJECT
	public:
		ManageFiltersDlg(QWidget* parent);
		virtual ~ManageFiltersDlg();

	private slots:
		void add();
		void remove();
		void removeAll();
		void newFilter();
	};

}

#endif
