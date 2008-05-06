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


#include "filter.h"

namespace kt
{

	Filter::Filter()
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
		
		}
		
	Filter::~Filter()
		{
		
		}

	QString Filter::getName()
		{
		QReadLocker readLock(&lock);
		return name;
		}
	
	QString Filter::getIconName()
		{
		QReadLocker readLock(&lock);
		return type ? "news-unsubscribe" : "news-subscribe";
		}
		
	int Filter::getType()
		{
		QReadLocker readLock(&lock);
		return type;
		}
	
	QString Filter::getGroup()
		{
		QReadLocker readLock(&lock);
		return group;
		}
	
	QStringList Filter::getExpressions()
		{
		QReadLocker readLock(&lock);
		return expressions;
		}
	
	int Filter::getSourceListType()
		{
		QReadLocker readLock(&lock);
		return sourceListType;
		}
	
	QStringList Filter::getSourceList()
		{
		QReadLocker readLock(&lock);
		return sourceList;
		}
	
	int Filter::getMultiMatch()
		{
		QReadLocker readLock(&lock);
		return multiMatch;
		}
	
	int Filter::getRerelease()
		{
		QReadLocker readLock(&lock);
		return rerelease;
		}
	
	CaptureChecker* Filter::getCaptureChecker()
		{
		QReadLocker readLock(&lock);
		return captureChecker;
		}
	
	void Filter::setName(const QString& value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = name != value;
		
		name = value;
		
		if (newValue)
			emit nameChanged(name);
		}
		
	void Filter::setType(int value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = type != value;
		
		type = value;
		
		if (newValue)
			emit typeChanged(type);
		}
		
	void Filter::setGroup(const QString& value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = group != value;
		group = value;
		
		if (newValue)
			emit groupChanged(group);
		}
	
	void Filter::setExpressions(QStringList value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = expressions != value;
		
		expressions = value;
		
		if (newValue)
			emit expressionsChanged(expressions);
		}
	
	void Filter::setSourceListType(int value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = sourceListType != value;
		
		sourceListType = value;
		
		if (newValue)
			emit sourceListTypeChanged(sourceListType);
		}
	
	void Filter::setSourceList(QStringList value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = sourceList != value;
		
		sourceList = value;
		
		if (newValue)
			emit sourceListChanged(sourceList);
		}
	
	void Filter::setMultiMatch(int value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = multiMatch != value;
		
		multiMatch = value;
		
		if (newValue)
			emit multiMatchChanged(multiMatch);
		}
		
	void Filter::setRerelease(int value)
		{
		QWriteLocker writeLock(&lock);
		
		bool newValue = rerelease != value;
		
		rerelease = value;
		
		if (newValue)
			emit rereleaseChanged(rerelease);
		}
		
	
	void Filter::run()
		{
		
		}
	
}