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

#include <QString>
#include <QMap>

#include "ui_capturecheckerdetails.h"
#include "capturechecker.h"

namespace kt
	{
	
	enum captureColumns
		{
		CAPTURE_NAME,
		CAPTURE_VALUE
		};
	
	enum variableColumns
		{
		VARIABLE_NAME,
		VARIABLE_MIN,
		VARIABLE_MAX
		};
	
	enum mappingColumns
		{
		MAP_CAPTURE,
		MAP_VARIABLE,
		MAP_INDEX,
		MAP_TEST
		};
	
	class CaptureCheckerDetails : public QWidget, private Ui::CaptureCheckerDetailsWidget
		{
			Q_OBJECT
		
		public:
			CaptureCheckerDetails(QWidget * parent = 0);
			virtual ~CaptureCheckerDetails() { }
			
		public slots:
			void resizeColumns();
			
			void connectCaptureChecker(CaptureChecker * value);
			void setCaptureChecker(CaptureChecker * value);
			
			void addNewCapture();
			void removeCapture();
			void addNewVariable();
			void removeVariable();
			void moveVariableUp();
			void moveVariableDown();
			
			//use these slots to update the availability of various actions
			void captureSelectionChanged();
			void variableSelectionChanged();
			
			//slot to keep a local copy of the test String
			void setTestString(const QString& value);
			
			//slots to update the various capture settings in the gui
			void verifyMappingInput(int row, int cell);
			void setCaptures(QMap<QString, QString> value);
			void setVariables(QList<Variable> value);
			void setMappings(QMap<QPair<QString,QString>, int> value);
			void updateMappingTest();
			
			//slot to emit changes back to the CaptureChecker
			void emitCaptures();
			void emitVariables();
		
		signals:
			void capturesChanged(QMap<QString, QString> captures);
			void variablesChanged(QList<Variable> variables);
			void mappingsChanged();
		
		private:
			CaptureChecker* captureChecker;
			QString testString;
			
			KAction* captureAdd;
			KAction* captureRemove;
			
			KAction* variableAdd;
			KAction* variableRemove;
			KAction* variableUp;
			KAction* variableDown;
		
		};
	
	}

#endif
