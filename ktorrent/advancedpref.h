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
#ifndef KT_ADVNACEDPREF_HH
#define KT_ADVNACEDPREF_HH

#include <interfaces/prefpageinterface.h>
#include "ui_advancedpref.h"
#include "ui_btpref.h"

namespace kt
{
	class AdvancedPref : public PrefPageInterface,public Ui_AdvancedPref
	{
		Q_OBJECT
	public:
		AdvancedPref(QWidget* parent);
		/*virtual */~AdvancedPref();

		void loadSettings();
		void loadDefaults();

	public slots:
		void onDiskPreallocToggled(bool on);
	};
        
	class BTPref: public PrefPageInterface,public Ui_BTPref
	{
		Q_OBJECT
	public:
		BTPref(QWidget* parent);
		/*virtual */~BTPref(){}
		void loadSettings();
	public Q_SLOTS:
		void onUploadDataCheckToggled(bool on);
	};


}

#endif

// kate: space-indent on; indent-width 8; replace-tabs off; mixed-indent off;
