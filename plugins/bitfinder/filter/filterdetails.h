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

#ifndef KTFILTERDETAILS_H
#define KTFILTERDETAILS_H

#include <kaction.h>
#include <kactionmenu.h>

#include "filter.h"

#include "ui_filterdetails.h"

#include <interfaces/coreinterface.h>

namespace kt
	{
	
	class FilterDetails : public QDialog, private Ui::FilterDetailsWidget
		{
			Q_OBJECT
		
		public:
			FilterDetails(CoreInterface* core, QWidget * parent = 0);
			virtual ~FilterDetails() { }
			
		public slots:
			void updateGroupList(QString oldName=QString(), QString newName=QString());
			void onMultiMatchChange(int curIndex);
			void onTypeChange(int curIndex);
			void refreshSizes();
			
			void connectFilter(Filter * value);
			void setFilter(Filter * value);
			
			//functions to handle translation of data to format wanted for Filter connection
			void emitType();
			void emitExpressions();
			void emitSourceListType();
			void emitSourceList();
			void emitMultiMatch();
			void emitRerelease();
			
			//function to translate from Filter format for data
			void setType(int value);
			void setSourceListType(int value);
			void setMultiMatch(int value);
			void setRerelease(int value);
			
		
		signals:
			void nameChanged(const QString& name);
			void typeChanged(int type);
			void groupChanged(const QString& group);
			void expressionsChanged(QStringList expressions);
			void sourceListTypeChanged(int sourceListType);
			void sourceListChanged(QStringList sourceList);
			void multiMatchChanged(int multiMatch);
			void rereleaseChanged(int rerelease);
			
			
		
		private:
			CoreInterface* core;
			
			Filter * filter;
			
			KActionMenu* sourceAdd;
			KAction* sourceRemove;
			
		
		};
	
	}

#endif