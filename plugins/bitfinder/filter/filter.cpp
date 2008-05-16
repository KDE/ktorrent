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

	Filter::Filter(const QString& baseDir, const QString& name) : name(name)
		{
		if (!baseDir.isEmpty() && !name.isEmpty())
			createMatches(baseDir);
		
		type = FT_ACCEPT;
		group = "Ungrouped";
		multiMatch = MM_ALWAYS_MATCH;
		rerelease = RR_IGNORE;
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
		connect(this, SIGNAL(rereleaseTermsChanged(const QString&)), this, SIGNAL(changed()));
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
	
	void Filter::start()
		{
		//start the thread going
		QThread::start();
		}
	
	void Filter::createMatches(const QString& baseDir)
		{
		if (matches)
			matches->deleteLater();
		
		matches = new Matches(baseDir + name + "/");
		
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
	
	QString Filter::getRereleaseTerms() const
		{
		QReadLocker readLock(&lock);
		return rereleaseTerms;
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
		
		//TODO: we should check the history - which isn't implemented yet
		
		//TODO: if we're already in the history we should check if it's a rerelease if that's enabled
		
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
//		//QString rereleaseTerms
		filter.setAttribute("RereleaseTerms", rereleaseTerms);
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
	
	void Filter::loadXmlElement(const QDomElement& filter)
		{
// 		QString name;
		if (filter.hasAttribute("Name"))
			name = filter.attribute("Name");
// 		int type;
		if (filter.hasAttribute("Type"))
			type = FilterTypeText.indexOf(filter.attribute("Type"));
// 		QString group;
		if (filter.hasAttribute("Group"))
			group = filter.attribute("Group");
// 		int sourceListType;
		if (filter.hasAttribute("SourceListType"))
			sourceListType = SourceListTypeText.indexOf(filter.attribute("SourceListType"));
// 		int multiMatch;
		if (filter.hasAttribute("MultiMatch"))
			multiMatch = MultiMatchText.indexOf(filter.attribute("MultiMatch"));
// 		int rerelease;
		if (filter.hasAttribute("Rerelease"))
			rerelease = RereleaseText.indexOf(filter.attribute("Rerelease"));
//		//QString rereleaseTerms
		if (filter.hasAttribute("RereleaseTerms"))
			rereleaseTerms = filter.attribute("RereleaseTerms");
// 		QStringList sourceList;
		QDomNodeList sourceNodes = filter.elementsByTagName("Source");
		sourceList.clear();
		for (int i=0; i<sourceNodes.count(); i++)
			{
			sourceList.append(sourceNodes.at(i).firstChild().toText().data());
			}
// 		QStringList expressions;
		QDomNodeList expressionNodes = filter.elementsByTagName("Expression");
		expressions.clear();
		for (int i=0; i<expressionNodes.count(); i++)
			{
			expressions.append(expressionNodes.at(i).firstChild().toText().data());
			}
// 		CaptureChecker * captureChecker;
		QDomNodeList captureCheckerNode = filter.elementsByTagName("CaptureChecker");
		if (captureCheckerNode.count())
			captureChecker->loadXmlElement(captureCheckerNode.at(0).toElement());
		}
	
	void Filter::enqueueItem(BFItem * item)
		{
		bool startQueue = false;
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		//if the queue is empty then we'll need to kick off processing
		if (!itemQueue.count())
			startQueue = true;
		
		itemQueue.enqueue(item);
		}//writeLock is out of scope now :)
		
		if (startQueue)
			emit startProcessing();
		}
	
	void Filter::processQueue()
		{
		BFItem * curItem;
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		//if the queue is empty then we've got nothing to do
		if (!itemQueue.count())
			return;
		
		curItem = itemQueue.dequeue();
		}//writeLock is out of scope now :)
		
		//now let's check the item to see if it's a match
		{//limiting the scope of the read lock
		QReadLocker readLock(&lock);
		
		if (sourceListType == SL_INCLUSIVE)
			{
			//the source needs to be in this list or don't process
			if (!sourceList.contains(curItem->getSource()))
				{
				emit unmatched(curItem);
				emit startProcessing();
				return;
				}
			}
		else
			{
			//if the source is in this list don't process
			if (sourceList.contains(curItem->getSource()))
				{
				emit unmatched(curItem);
				emit startProcessing();
				return;
				}
			}
		
		//put together a list of strings we wish to check for a match. A match on any one is sufficient
		QStringList checkStrings = QStringList() << curItem->getName() << curItem->getLink() << curItem->getDescription() << curItem->getFilenames();
		
		for (int i=0; i<checkStrings.count(); i++)
			{
			//first check we've got a match - no match move on to the next
			if (!checkMatch(checkStrings.at(i)))
				continue;
			
			//we've made it all the way here which means we're a match that's either not in the history
			//or is a rerelease that we want to download so let's download
			Capture * curCap = 0;
			if (multiMatch == MM_CAPTURE_CHECKING)
				{
				curCap = new Capture();
				*curCap = captureChecker->findCapture(checkStrings.at(i));
				}
			
			emit download(curItem, curCap);
			emit startProcessing();
			return;
			}
		
		}//readLock scope done
		
		//have check them all and still have no match
		emit unmatched(curItem);
		
		//we're done so let's say process the next one
		emit startProcessing();
		}
	
	void Filter::removeExpression(const QString& value)
		{
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		expressions.removeAll(value);
		}//writeLock is out of scope now :)
		
		emit expressionsChanged(expressions);
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
		
	void Filter::setRereleaseTerms(const QString& value)
		{
		bool newValue;
		
		{//limit the scope of the writeLock
		QWriteLocker writeLock(&lock);
		
		newValue = rereleaseTerms != value;
		
		rereleaseTerms = value;
		}//writeLock is out of scope now :)
		if (newValue)
			emit rereleaseTermsChanged(rereleaseTerms);
		}
		
	
	void Filter::run()
		{
		//connect up what's required for all the processing to work
		connect(this, SIGNAL(startProcessing()), this, SLOT(processQueue()));
		
		//start the thread event loop
		exec();
		}
	
}
