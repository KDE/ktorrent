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

#ifndef KTFILTER_H
#define KTFILTER_H

#include <QThread>
#include <QString>
#include <QStringList>
#include <QReadWriteLock>
#include <QDomElement>

#include <util/constants.h>

#include "filterconstants.h"
#include "capturechecker.h"

namespace kt
{
	class Filter : public QThread
	{
		Q_OBJECT
	public:
		Filter(const QString& name = QString());
		Filter& operator=(const Filter& other);
		~Filter();
		
		QString getName() const;
		QString getIconName() const;
		int getType() const;
		QString getGroup() const;
		QStringList getExpressions() const;
		int getSourceListType() const;
		QStringList getSourceList() const;
		int getMultiMatch() const;
		int getRerelease() const;
		QString getRereleaseTerms() const;
		CaptureChecker* getCaptureChecker() const;
		
		bool checkExpressionMatch(const QString& string) const;
		bool checkMatch(const QString& string) const;
		
		QDomElement getXmlElement() const;
		void loadXmlElement(const QDomElement& filter);
		
	public slots:
		void removeExpression(const QString& value);
	
		void setName(const QString& value);
		void setType(int value);
		void setGroup(const QString& value);
		void setExpressions(QStringList value);
		void setSourceListType(int value);
		void setSourceList(QStringList value);
		void setMultiMatch(int value);
		void setRerelease(int value);
		void setRereleaseTerms(const QString& value);
	
	signals:
		void changed();
		void nameChanged(const QString& name);
		void typeChanged(int type);
		void groupChanged(const QString& group);
		void expressionsChanged(QStringList expressions);
		void sourceListTypeChanged(int sourceListType);
		void sourceListChanged(QStringList expressions);
		void multiMatchChanged(int multiMatch);
		void rereleaseChanged(int rerelease);
		void rereleaseTermsChanged(const QString& name);
		
	protected:
		virtual void run();
	
	private:
		mutable QReadWriteLock lock;
		QString name;
		int type;
		QString group;
		QStringList expressions;
		int multiMatch;
		int rerelease;
		QString rereleaseTerms;
		CaptureChecker * captureChecker;
		int sourceListType;
		QStringList sourceList;
		
	};

}

#endif
