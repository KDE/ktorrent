/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#ifndef KTCAPTURECHECKERDETAILS_H
#define KTCAPTURECHECKERDETAILS_H

#include <kaction.h>

#include "ui_capturecheckerdetails.h"
#include "capturechecker.h"

namespace kt
	{
	
	class CaptureCheckerDetails : public QWidget, private Ui::CaptureCheckerDetailsWidget
		{
			Q_OBJECT
		
		public:
			CaptureCheckerDetails(QWidget * parent = 0);
			virtual ~CaptureCheckerDetails() { }
			
		public slots:
			void setCaptureChecker(CaptureChecker * value);
			void addNewCapture();
			
		private:
			CaptureChecker* captureChecker;
			
			KAction* captureAdd;
			KAction* captureRemove;
			
			KAction* variableAdd;
			KAction* variableRemove;
			KAction* variableUp;
			KAction* variableDown;
		
		};
	
	}

#endif