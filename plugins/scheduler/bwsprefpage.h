/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivan@ktorrent.org                                                     *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
 
#ifndef KTBWSPREFPAGE_H
#define KTBWSPREFPAGE_H

#include <interfaces/prefpageinterface.h>

#include "bwsprefpagewidget.h"

namespace kt
{
	/**
	 * @brief Bandwidth Scheduler Preferences Page.
	 * @author Ivan Vasic <ivan@ktorrent.org>
	*/
	class BWSPrefPage : public PrefPageInterface
	{
		public:
			BWSPrefPage();
			virtual ~BWSPrefPage();
			
			virtual bool apply();
			virtual void createWidget(QWidget* parent);
			virtual void updateData();
			virtual void deleteWidget();

		private:
			BWSPrefPageWidget* widget;
	};
}

#endif
