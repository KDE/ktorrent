/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QWidget>
#include <ksharedconfig.h>
#include <ktcore_export.h>

namespace kt
{
	/**
	 * Base class for all activities
	 */
	class KTCORE_EXPORT Activity : public QWidget
	{
		Q_OBJECT
	public:
		Activity(const QString & name,const QString & icon,QWidget* parent);
		virtual ~Activity();
		
		const QString & name() const {return activity_name;}
		const QString & icon() const {return activity_icon;}
		
		void setName(const QString & name);
		void setIcon(const QString & icon);
		
		/// Load the state of the activity
		virtual void loadState(KSharedConfigPtr cfg) = 0;
		
		/// Save the state of the activity
		virtual void saveState(KSharedConfigPtr cfg) = 0;
		
	signals:
		void nameChanged(Activity* a,const QString & name);
		void iconChanged(Activity* a,const QString & icon);
		
	private:
		QString activity_name;
		QString activity_icon;
	};
}

#endif // ACTIVITY_H
