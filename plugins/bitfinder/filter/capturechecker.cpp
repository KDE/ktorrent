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

#include <QRegExp>
#include <QReadLocker>
#include <QWriteLocker>
#include <QStringList>

#include <util/log.h>
using namespace bt;

namespace kt
	{
	
	CaptureChecker::CaptureChecker(QObject * parent) : QObject(parent)
		{
		connect(this, SIGNAL(variablesChanged(QList< Variable >)), this, SLOT(updateMappings()));
		connect(this, SIGNAL(capturesChanged(QMap< QString, QString >)), this, SLOT(updateMappings()));
		
		connect(this, SIGNAL(capturesChanged(QMap< QString, QString >)), this, SIGNAL(changed()));
		connect(this, SIGNAL(variablesChanged(QList< Variable >)), this, SIGNAL(changed()));
		connect(this, SIGNAL(mappingsChanged(QMap< QPair < QString , QString >, int >)), this, SIGNAL(changed()));
		}
	
	QMap<QString, QString> CaptureChecker::getCaptures() const
		{
		QReadLocker readLock(&lock);
		
		return captures;
		}
	
	QList<Variable> CaptureChecker::getVariables() const
		{
		QReadLocker readLock(&lock);
		
		return variables;
		}
	
	QMap<QPair<QString, QString>, int> CaptureChecker::getMappings() const
		{
		QReadLocker readLock(&lock);
		
		return mappings;
		}
	
	Capture CaptureChecker::getMinCapture() const
		{
		QReadLocker readLock(&lock);
		
		Capture value;
		
		for (int i=0; i<variables.count(); i++)
			{
			value.addVariable(variables.at(i).name, variables.at(i).min);
			}
		
		return value;
		}
		
	Capture CaptureChecker::getMaxCapture() const
		{
		QReadLocker readLock(&lock);
		
		Capture value;
		
		for (int i=0; i<variables.count(); i++)
			{
			value.addVariable(variables.at(i).name, variables.at(i).max);
			}
		
		return value;
		}

	Capture CaptureChecker::findCapture(QString sourceString, QString capture) const
		{
		QReadLocker readLock(&lock);
		
		QStringList captureList;
		
		if (capture.isEmpty())
			{
			captureList = captures.keys();
			}
		else
			{
			captureList.append(capture);
			}
		
		//run through the list to find a capture
		QMap<QString, QString>::const_iterator i = captures.constBegin();
		QRegExp curCap;
		curCap.setCaseSensitivity(Qt::CaseInsensitive);
		QPair<QString, QString> curPair;
		
		Capture bestMatch;
		
		while ( i != captures.constEnd() )
			{
			if (!captureList.contains(i.key()))
				{
				//this one isn't in the list - so ignore it.
				i++;
				continue;
				}
			
			curCap.setPattern(i.value());
			
			if (curCap.indexIn(sourceString) == -1)
				{
				//no match on this capture so move onto the next capture
				i++;
				continue;
				}
			
			Capture value;
			
			for (int j=0; j<variables.count(); j++)
				{
				curPair.first = i.key();
				curPair.second = variables.at(j).name;
				
				value.addVariable(curPair.second, curCap.cap(mappings.value(curPair)));
				}
			
			bestMatch = value;
			
			if (value.isInRange(getMinCapture(), getMaxCapture()))
				{
				//we've got the values and met the limits
				return value;
				}
			
			i++;
			}
		
		//we didn't get any matches that are inRange so here's the closest
		return bestMatch;
		}
	
	QDomElement CaptureChecker::getXmlElement() const
		{
		QReadLocker readLock(&lock);
		
		QDomDocument doc;
		QDomElement captureChecker = doc.createElement("CaptureChecker");
		
// 		QMap<QString, QString> captures;
		QMap<QString, QString>::const_iterator cit = captures.constBegin();
		while ( cit != captures.constEnd() )
			{
			QDomElement capture = doc.createElement("Capture");
			capture.setAttribute("Name", cit.key());
			capture.setAttribute("Value", cit.value());
			captureChecker.appendChild(capture);
			cit++;
			}
// 		QList<Variable> variables;
		for (int i=0; i<variables.count(); i++)
			{
			QDomElement variable = doc.createElement("Variable");
			variable.setAttribute("Name", variables.at(i).name);
			variable.setAttribute("Min", variables.at(i).min);
			variable.setAttribute("Max", variables.at(i).max);
			captureChecker.appendChild(variable);
			}
// 		QMap<QPair<QString, QString>, int> mappings;
		QMap<QPair<QString, QString>, int>::const_iterator mit = mappings.constBegin();
		
		while ( mit != mappings.constEnd() )
			{
			QDomElement mapping = doc.createElement("Mapping");
			mapping.setAttribute("Capture", mit.key().first);
			mapping.setAttribute("Variable", mit.key().second);
			mapping.setAttribute("Index", QString::number(mit.value()));
			captureChecker.appendChild(mapping);
			mit++;
			}
		
		return captureChecker;
		}
	
	void CaptureChecker::loadXmlElement(const QDomElement& captureChecker)
		{
// 		QMap<QString, QString> captures;
		QDomNodeList captureNodes = captureChecker.elementsByTagName("Capture");
		QDomElement curCap;
		captures.clear();
		for (int i=0; i<captureNodes.count(); i++)
			{
			curCap = captureNodes.at(i).toElement();
			if (curCap.attribute("Name").isEmpty())
				continue;
			
			captures.insert(curCap.attribute("Name"), curCap.attribute("Value"));
			}
// 		QList<Variable> variables;
		QDomNodeList variableNodes = captureChecker.elementsByTagName("Variable");
		QDomElement curVarNode;
		Variable curVar;
		variables.clear();
		for (int i=0; i<variableNodes.count(); i++)
			{
			curVarNode = variableNodes.at(i).toElement();
			if (curVarNode.attribute("Name").isEmpty())
				continue;
			
			curVar.name = curVarNode.attribute("Name");
			curVar.min = curVarNode.attribute("Min");
			curVar.max = curVarNode.attribute("Max");
			variables.append(curVar);
			}
// 		QMap<QPair<QString, QString>, int> mappings;
		QDomNodeList mappingNodes = captureChecker.elementsByTagName("Mapping");
		QDomElement curMap;
		mappings.clear();
		for (int i=0; i<mappingNodes.count(); i++)
			{
			curMap = mappingNodes.at(i).toElement();
			if (curMap.attribute("Capture").isEmpty())
				continue;
			
			if (curMap.attribute("Variable").isEmpty())
				continue;
			
			mappings.insert(QPair<QString, QString>(curMap.attribute("Capture"), curMap.attribute("Variable")), 
				curMap.attribute("Index").toInt());
			}
		//ok all the ones in XML are loaded up, let's just make sure it's complete
		updateMappings();
		}
	
	bool CaptureChecker::addNewCapture(const QString& name)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);

		if (captures.contains(name))
			return false;
		
		captures.insert(name, QString(""));
		} //writeLock is out of scope - huzzah
		emit capturesChanged(captures);
		return true;
		}
	
	bool CaptureChecker::setCaptureValue(const QString& name, const QString& value)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);

		if (!captures.contains(name))
			return false;
		
		captures.insert(name, value);
		} //writeLock is out of scope - huzzah
		emit capturesChanged(captures);
		return true;
		}
	
	void CaptureChecker::removeCapture(const QString& name)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		captures.remove(name);
		} //writeLock is out of scope - huzzah
		emit capturesChanged(captures);
		}
	
	bool CaptureChecker::addNewVariable(const QString& name)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);

		for (int i=0; i<variables.count(); i++)
			{
			if (variables.at(i).name == name)
				return false;
			}
		
		Variable newVar;
		newVar.name = name;
		
		variables.append(newVar);
		
		} //writeLock is out of scope - huzzah
		emit variablesChanged(variables);
		return true;
		}
	
	void CaptureChecker::removeVariable(const QString& name)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		for (int i=variables.count()-1; i>=0; i--)
			{
			if (variables.at(i).name == name)
				variables.removeAt(i);
			}
		}//writeLock is out of scope - huzzah
		emit variablesChanged(variables);
		}
	
	void CaptureChecker::moveVariableUp(int pos)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		if (pos == 0)
			return;
		
		Variable shiftMe = variables.takeAt(pos);
		variables.insert(pos - 1, shiftMe);
		
		}//writeLock is out of scope - huzzah
		emit variablesChanged(variables);
		}
		
	void CaptureChecker::moveVariableDown(int pos)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		if (pos == variables.count()-1)
			return;
		
		Variable shiftMe = variables.takeAt(pos);
		variables.insert(pos + 1, shiftMe);
		
		}//writeLock is out of scope - huzzah
		emit variablesChanged(variables);
		}
	
	void CaptureChecker::setMappingValue(const QString& captureName, const QString& variableName, int index)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		QPair<QString, QString> curPair;
		curPair.first = captureName;
		curPair.second = variableName;
		
		if (!mappings.contains(curPair))
			return;
		
		if (mappings.value(curPair) == index)
			return;
		
		mappings.insert(curPair, index);
		
		}//writeLock is out of scope - huzzah
		emit mappingsChanged(mappings);
		}
	
	void CaptureChecker::updateMappings()
		{
		{//limit the scope of the writeLock
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
		}//writeLock is out of scope - huzzah
		emit mappingsChanged(mappings);
		}
	
	void CaptureChecker::setCaptures(QMap<QString, QString> value)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		if (captures == value)
			return;
		
		captures=value;
		
		}//writeLock is out of scope - huzzah
		emit capturesChanged(captures);
		}
	
	void CaptureChecker::setVariables(QList<Variable> value)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		if (variables == value)
			return;
		
		variables=value;
		
		}//writeLock is out of scope - huzzah
		emit variablesChanged(variables);
		}
	
	}