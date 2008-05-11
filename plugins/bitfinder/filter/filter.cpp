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

#include <QRegExp>
#include <QDomDocument>
#include <QDomText>

#include <util/log.h>

#include "filter.h"
#include "capture.h"

using namespace bt;

namespace kt
{

	Filter::Filter(const QString& name) : name(name)
		{
		type = FT_ACCEPT;
		group = "Ungrouped";
		multiMatch = MM_ALWAYS_MATCH;
		rerelease = RR_DOWNLOAD_ALL;
		sourceListType = SL_EXCLUSIVE;
		
		captureChecker = new CaptureChecker(this);
		
		connect(this, SIGNAL(nameChanged(const QString&)), this, SIGNAL(changed()));
		connect(this, SIGNAL(typeChanged(int)), this, SIGNAL(changed()));
		connect(this, SIGNAL(groupChanged(const QString&)), this, SIGNAL(changed()));
		connect(this, SIGNAL(expressionsChanged(QStringList)), this, SIGNAL(changed()));
		connect(this, SIGNAL(sourceListTypeChanged(int)), this, SIGNAL(changed()));
		connect(this, SIGNAL(sourceListChanged(QStringList)), this, SIGNAL(changed()));
		connect(this, SIGNAL(multiMatchChanged(int)), this, SIGNAL(changed()));
		connect(this, SIGNAL(rereleaseChanged(int)), this, SIGNAL(changed()));
		connect(captureChecker, SIGNAL(changed()), this, SIGNAL(changed()));
		
		}
		
	Filter& Filter::operator=(const Filter& other)
		{
		name = other.getName();
		type = other.getType();
		group = other.getGroup();
		expressions = other.getExpressions();
		sourceListType = other.getSourceListType();
		sourceList = other.getSourceList();
		multiMatch = other.getMultiMatch();
		rerelease = other.getRerelease();
		captureChecker = other.getCaptureChecker();
		
		return *this;
		}
	
	Filter::~Filter()
		{
		
		}

	QString Filter::getName() const
		{
		QReadLocker readLock(&lock);
		return name;
		}
	
	QString Filter::getIconName() const
		{
		QReadLocker readLock(&lock);
		return type ? "news-subscribe" : "news-unsubscribe";
		}
		
	int Filter::getType() const
		{
		QReadLocker readLock(&lock);
		return type;
		}
	
	QString Filter::getGroup() const
		{
		QReadLocker readLock(&lock);
		return group;
		}
	
	QStringList Filter::getExpressions() const
		{
		QReadLocker readLock(&lock);
		return expressions;
		}
	
	int Filter::getSourceListType() const
		{
		QReadLocker readLock(&lock);
		return sourceListType;
		}
	
	QStringList Filter::getSourceList() const
		{
		QReadLocker readLock(&lock);
		return sourceList;
		}
	
	int Filter::getMultiMatch() const
		{
		QReadLocker readLock(&lock);
		return multiMatch;
		}
	
	int Filter::getRerelease() const
		{
		QReadLocker readLock(&lock);
		return rerelease;
		}
	
	CaptureChecker* Filter::getCaptureChecker() const
		{
		QReadLocker readLock(&lock);
		return captureChecker;
		}
	
	bool Filter::checkExpressionMatch(const QString& string) const
		{
		QReadLocker readLock(&lock);
		
		//if we have no expressions it should be considered a fail
		if (expressions.isEmpty())
			return false;
		
		QRegExp regEx;
		regEx.setCaseSensitivity(Qt::CaseInsensitive);
		QString curExp;
		bool find;
		bool found;
		
		//run through all the expressions checking if they match
		for (int i=0; i<expressions.count(); i++)
			{
			find = true;
			curExp = expressions.at(i);
			if (curExp.startsWith("!"))
				{
				curExp.remove(0, 1);
				find = false;
				}
			
			regEx.setPattern(curExp);
			
			found = regEx.indexIn(string) != -1;
			
			//if we've not matched (or matched when we didn't want to) 
			if (found != find)
				return false;
			}
		
		//if we make it this far we've got a match
		return true;
		}
	
	bool Filter::checkMatch(const QString& string) const
		{
		if (string.isEmpty())
			return false;
		
		QReadLocker readLock(&lock);
		
		//check we're good for expressions
		if (!checkExpressionMatch(string))
			return false;
		
		if (multiMatch == MM_ONCE_ONLY)
			{
			//we'll need to add history checking when history is done
			//for now just say it matches
			return true;
			}
		else if (multiMatch == MM_ALWAYS_MATCH)
			{
			//no need to check captures as we always match the expression
			return true;
			}
		
		//if we've got here we're set to check captures - so let's find the capture
		Capture curCap = captureChecker->findCapture(string);
		
		if (!curCap.isInRange(captureChecker->getMinCapture(), captureChecker->getMaxCapture()))
			return false;
		
		//made it this far? it's a match :)
		return true;
		}
	
	QDomElement Filter::getXmlElement() const
		{
		QReadLocker readLock(&lock);
		
		QDomDocument doc;
		QDomElement filter = doc.createElement("Filter");
		
// 		QString name;
		filter.setAttribute("Name", name);
// 		int type;
		filter.setAttribute("Type", FilterTypeText.at(type));
// 		QString group;
		filter.setAttribute("Group", group);
// 		int sourceListType;
		filter.setAttribute("SourceListType", SourceListTypeText.at(sourceListType));
// 		int multiMatch;
		filter.setAttribute("MultiMatch", MultiMatchText.at(multiMatch));
// 		int rerelease;
		filter.setAttribute("Rerelease", RereleaseText.at(rerelease));
// 		QStringList sourceList;
		for (int i=0; i<sourceList.count(); i++)
			{
			QDomElement source = doc.createElement("Source");
			QDomText sourceName = doc.createTextNode(sourceList.at(i));
			source.appendChild(sourceName);
			filter.appendChild(source);
			}
// 		QStringList expressions;
		for (int i=0; i<expressions.count(); i++)
			{
			QDomElement expression = doc.createElement("Expression");
			QDomText expressionText = doc.createTextNode(expressions.at(i));
			expression.appendChild(expressionText);
			filter.appendChild(expression);
			}
// 		CaptureChecker * captureChecker;
		filter.appendChild(captureChecker->getXmlElement());
		
		return filter;
		}
	
	void Filter::setName(const QString& value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = name != value;
		
		name = value;
		}//writeLock is out of scope now :)
		if (newValue)
			emit nameChanged(name);
		}
		
	void Filter::setType(int value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = type != value;
		
		type = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit typeChanged(type);
		}
		
	void Filter::setGroup(const QString& value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = group != value;
		group = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit groupChanged(group);
		}
	
	void Filter::setExpressions(QStringList value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = expressions != value;
		
		expressions = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit expressionsChanged(expressions);
		}
	
	void Filter::setSourceListType(int value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = sourceListType != value;
		
		sourceListType = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit sourceListTypeChanged(sourceListType);
		}
	
	void Filter::setSourceList(QStringList value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = sourceList != value;
		
		sourceList = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit sourceListChanged(sourceList);
		}
	
	void Filter::setMultiMatch(int value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = multiMatch != value;
		multiMatch = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit multiMatchChanged(multiMatch);
		}
		
	void Filter::setRerelease(int value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		newValue = rerelease != value;
		rerelease = value;
		
		}//writeLock is out of scope now :)
		if (newValue)
			emit rereleaseChanged(rerelease);
		}
		
	
	void Filter::run()
		{
		
		}
	
}
