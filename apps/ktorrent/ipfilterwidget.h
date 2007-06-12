/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef IPFILTERWIDGET_H
#define IPFILTERWIDGET_H

#include "ipfilterwidgetbase.h"

/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief Integrated IPFilter GUI class.
 * Used to show, add and remove banned peers from blacklist.
 */
class IPFilterWidget: public BlacklistWidgetBase
{
		Q_OBJECT
	public:
		IPFilterWidget(QWidget *parent = 0, const char *name = 0);
		
		virtual void btnApply_clicked();
		virtual void btnOk_clicked();
		virtual void btnSave_clicked();
		virtual void btnOpen_clicked();
		virtual void btnClear_clicked();
		virtual void btnRemove_clicked();
		virtual void btnAdd_clicked();
		
		void saveFilter(QString& fn);
		void loadFilter(QString& fn);
};

#endif
