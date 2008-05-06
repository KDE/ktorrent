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

#include "capturechecker.h"

#include <QReadLocker>
#include <QWriteLocker>

namespace kt
	{
	
	CaptureChecker::CaptureChecker(QObject * parent) : QObject(parent)
		{
		connect(this, SIGNAL(variablesChanged(QList< Variable >)), this, SLOT(updateMappings()));
		connect(this, SIGNAL(capturesChanged(QMap< QString, QString >)), this, SLOT(updateMappings()));
		}
	
	bool CaptureChecker::addNewCapture(const QString& name)
		{
		QWriteLocker writeLock(&lock);

		if (captures.contains(name))
			return false;
		
		captures.insert(name, QString(""));
		emit capturesChanged(captures);
		return true;
		}
	
	bool CaptureChecker::setCaptureValue(const QString& name, const QString& value)
		{
		QWriteLocker writeLock(&lock);

		if (!captures.contains(name))
			return false;
		
		captures.insert(name, value);
		emit capturesChanged(captures);
		return true;
		}
	
	bool CaptureChecker::addNewVariable(const QString& name)
		{
		QWriteLocker writeLock(&lock);

		for (int i=0; i<variables.count(); i++)
			{
			if (variables.at(i).name == name)
				return false;
			}
		
		Variable newVar;
		newVar.name = name;
		
		variables.append(newVar);
		
		emit variablesChanged(variables);
		return true;
		}
	
	void CaptureChecker::setMappingValue(const QString& captureName, const QString& variableName, int index)
		{
		QWriteLocker writeLock(&lock);
		
		QPair<QString, QString> curPair;
		curPair.first = captureName;
		curPair.second = variableName;
		
		if (!mappings.contains(curPair))
			return;
			
		mappings.insert(curPair, index);
		
		emit mappingsChanged(mappings);
		}
	
	void CaptureChecker::updateMappings()
		{
		QWriteLocker writeLock(&lock);
		
		QMap<QPair<QString, QString>, int> value;
		
		QPair<QString, QString> curPair;
		
		QMap<QString, QString>::const_iterator i = captures.constBegin();
		
		while ( i != captures.constEnd() )
			{
			for (int j=0; j<variables.count(); j++)
				{
				int curIndex = 0;
				
				curPair.first = i.key();
				curPair.second = variables.at(j).name;
				
				if (mappings.contains(curPair))
					{
					curIndex = mappings.value(curPair);
					}
				
				value.insert(curPair, curIndex);
				}
			
			i++;
			}
		
		mappings = value;
		emit mappingsChanged(mappings);
		}
	
	void CaptureChecker::setCaptures(QMap<QString, QString> value)
		{
		QWriteLocker writeLock(&lock);
		
		if (captures == value)
			return;
		
		captures=value;
		
		emit capturesChanged(captures);
		}
	
	void CaptureChecker::setVariables(QList<Variable> value)
		{
		QWriteLocker writeLock(&lock);
		
		if (variables == value)
			return;
		
		variables=value;
		
		emit variablesChanged(variables);
		}
	
	}