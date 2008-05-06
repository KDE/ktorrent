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

#ifndef KTCAPTURECHECKER_H
#define KTCAPTURECHECKER_H

#include <QReadWriteLock>
#include <QMap>
#include <QString>
#include <QPair>

namespace kt
	{
	
	struct Variable
		{
		QString name;
		QString min;
		QString max;
		
		bool operator==(const Variable& other)
			{
			return name==other.name && min==other.min && max==other.max;
			}
		};
		
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
	
	class CaptureChecker : public QObject
		{
		Q_OBJECT
		
		public:
			CaptureChecker(QObject * parent = 0);
			~CaptureChecker() {}
			
			QMap<QString, QString> getCaptures();
			QList<Variable> getVariables();
		
		public slots:
			bool addNewCapture(const QString& name);
			bool setCaptureValue(const QString& name, const QString& value);
			void removeCapture(const QString& name);
			
			bool addNewVariable(const QString& name);
			void removeVariable(const QString& name);
			
			void setMappingValue(const QString& captureName, const QString& variableName, int index);
			
			//mapping depends upon captures and variables
			void updateMappings();
			
			//write full set from scratch
			void setCaptures(QMap<QString, QString> value);
			void setVariables(QList<Variable> value);
		
		signals:
			void capturesChanged(QMap<QString, QString> captures);
			void variablesChanged(QList<Variable> variables);
			void mappingsChanged(QMap<QPair<QString, QString>, int>);
		
		private:
			QReadWriteLock lock;
			QMap<QString, QString> captures;
			QList<Variable> variables;
			QMap<QPair<QString, QString>, int> mappings;
			
		};
	
	}

#endif