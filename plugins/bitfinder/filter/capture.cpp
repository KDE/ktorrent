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

#include <QReadLocker>
#include <QWriteLocker>

#include <util/log.h>

#include "capture.h"

using namespace bt;

namespace kt
	{
	
	Capture::Capture(QObject * parent) : QObject(parent)
		{
		
		}
	
	Capture::Capture(const Capture& other)
		{
		QWriteLocker writeLock(&lock);
		
		variables = other.getVariables();
		}
	
	Capture& Capture::operator=(const Capture& other)
		{
		QWriteLocker writeLock(&lock);
		
		variables = other.getVariables();
		
		return *this;
		}
	
	bool Capture::meetsMin(const Capture& min) const
		{
		QReadLocker readLock(&lock);
		
		if (variables.count() != min.varCount())
			return false;
		
		QPair<QString,QString> curMinVar;
		
		for (int i=0; i<variables.count(); i++)
			{
			curMinVar = min.getVariable(i);
			
			//the names or order don't match so let's skip it
			if (curMinVar.first != variables.at(i).first)
				return false;
			
			//if the min is empty consider it to mean there is no minimum
			if (curMinVar.second.isEmpty())
				return true;
			
			//if we don't have a value for this variable but there is one 
			//for the min then we should consider it not to match
			if (variables.at(i).second.isEmpty())
				return false;
			
			//check if both sides only use numbers - if so we can do a
			//numerical comparison - otherwise it'll need to be string
			bool minNumeric = false;
			bool varNumeric = false;
			
			float minValue = min.getVariable(i).second.toFloat(&minNumeric);
			float curValue = variables.at(i).second.toFloat(&varNumeric);
			
			if (minNumeric && varNumeric)
				{
				//the values are both numbers so let's do a numeric comparison
				if (curValue > minValue)
					{
					//we're definitely above the minimum so return true
					return true;
					}
				else if (curValue == minValue)
					{
					//the values at the level are equal - so we need to go
					//to the next to check (i.e. i++)
					continue;
					}
				else
					{
					//we're less than the minimum value - so it's definitely not a match
					return false;
					}
				}
			else
				{
				//do a string based comparison
				int stringCompare = QString::localeAwareCompare(min.getVariable(i).second, variables.at(i).second);
				
				if (stringCompare < 0)
					{
					//current value is above the minimum
					return true;
					}
				else if (stringCompare == 0)
					{
					//they're equal - need to check next variable
					continue;
					}
				else
					{
					//we're less than the minimum
					return false;
					}
					
				}
			
			}
		
		//if it's made it this far it means they're equal
		return true;
		}
		
	bool Capture::meetsMax(const Capture& max) const
		{
		QReadLocker readLock(&lock);
		
		if (variables.count() != max.varCount())
			return false;
		
		QPair<QString,QString> curMaxVar;
		
		for (int i=0; i<variables.count(); i++)
			{
			curMaxVar = max.getVariable(i);
			
			//the names or order don't match so let's skip it
			if (curMaxVar.first != variables.at(i).first)
				return false;
			
			//if the max is empty consider it to mean there is no maximum
			if (curMaxVar.second.isEmpty())
				return true;
			
			//if we don't have a value for this variable but there is one 
			//for the max then we should consider it not to match
			if (variables.at(i).second.isEmpty())
				return false;
			
			//check if both sides only use numbers - if so we can do a
			//numerical comparison - otherwise it'll need to be string
			bool maxNumeric = false;
			bool varNumeric = false;
			
			float maxValue = max.getVariable(i).second.toFloat(&maxNumeric);
			float curValue = variables.at(i).second.toFloat(&varNumeric);
			
			if (maxNumeric && varNumeric)
				{
				//the values are both numbers so let's do a numeric comparison
				if (curValue < maxValue)
					{
					//we're definitely below the maximum so return true
					return true;
					}
				else if (curValue == maxValue)
					{
					//the values at the level are equal - so we need to go
					//to the next to check (i.e. i++)
					continue;
					}
				else
					{
					//we're more than the maximum value - so it's definitely not a match
					return false;
					}
				}
			else
				{
				//do a string based comparison
				int stringCompare = QString::localeAwareCompare(max.getVariable(i).second, variables.at(i).second);
				
				if (stringCompare > 0)
					{
					//current value is below the maximum
					return true;
					}
				else if (stringCompare == 0)
					{
					//they're equal - need to check next variable
					continue;
					}
				else
					{
					//we're more than the maximum
					return false;
					}
					
				}
			
			}
		
		//if it's made it this far it means they're equal
		return true;
		}
	
	bool Capture::isEqual(const Capture& other) const
		{
		QReadLocker readLock(&lock);
		
		if (variables.count() != other.varCount())
			return false;
		
		QPair<QString,QString> curOtherVar;
		
		for (int i=0; i<variables.count(); i++)
			{
			curOtherVar = other.getVariable(i);
			
			//the names or order don't match so let's skip it
			if (curOtherVar.first != variables.at(i).first)
				return false;
			
			//check if both sides only use numbers - if so we can do a
			//numerical comparison - otherwise it'll need to be string
			bool otherNumeric = false;
			bool varNumeric = false;
			
			float otherValue = other.getVariable(i).second.toFloat(&otherNumeric);
			float curValue = variables.at(i).second.toFloat(&varNumeric);
			
			if (otherNumeric && varNumeric)
				{
				if (curValue != otherValue)
					{
					return false;
					}
				}
			else
				{
				//do a string based comparison
				int stringCompare = QString::localeAwareCompare(other.getVariable(i).second, variables.at(i).second);
				
				if (stringCompare != 0)
					{
					return false;
					}
					
				}
			
			}
		
		//if it's made it this far it means they're equal
		return true;
		}
	
	bool Capture::isEmpty() const
		{
		QReadLocker readLock(&lock);
		
		return variables.isEmpty();
		}
	
	QPair<QString,QString> Capture::getVariable(int index) const
		{
		QReadLocker readLock(&lock);
		
		if (index>=variables.count() || index < 0)
			return QPair<QString,QString>();
		
		return variables.at(index);
		}
	
	QString Capture::getValue(QString varName) const
		{
		QReadLocker readLock(&lock);
		
		for (int i=0; i<variables.count(); i++)
			{
			if (variables.at(i).first == varName)
				return variables.at(i).second;
			}
		
		return QString();
		}
	
	QList< QPair<QString,QString> > Capture::getVariables() const
		{
		QReadLocker readLock(&lock);
		
		return variables;
		}
	
	int Capture::varCount() const
		{
		QReadLocker readLock(&lock);
		
		return variables.count();
		}
	
	void Capture::addVariable(QString name, QString value)
		{
		QWriteLocker writeLock(&lock);
		
		for (int i=0; i<variables.count(); i++)
			{
			if (variables.at(i).first == name)
				return;
			}
		
		QPair<QString, QString> newVar;
		newVar.first = name;
		newVar.second = value;
		
		variables.append(newVar);
		}
	
	}
