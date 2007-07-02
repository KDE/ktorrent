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
#ifndef CONVERTDIALOG_H
#define CONVERTDIALOG_H

#include "convert_dlg.h"
#include "ipfilterplugin.h"

#include <qevent.h>

namespace kt
{
	class ConvertDialog: public ConvertingDlg
	{
			Q_OBJECT

		public:
			ConvertDialog( IPFilterPlugin* p, QWidget *parent = 0, const char *name = 0 );

		public slots:
			virtual void btnClose_clicked();

		private:
			void convert();
			IPFilterPlugin* m_plugin;
			bool to_convert;
			bool converting;
			bool canceled;
			
		private slots:
			void closeEvent(QCloseEvent* e);
    		virtual void btnCancel_clicked();
	};
}
#endif
