/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef KTSCRIPT_H
#define KTSCRIPT_H

#include <QObject>

namespace Kross
{
	class Action;
}

namespace kt
{

	/**
		Keeps track of a script
	*/
	class Script : public QObject
	{
		Q_OBJECT
	public:
		Script(const QString & file,QObject* parent);
		virtual ~Script();
		
		/**
		 * Load and execute the script
		 * @return true upon success
		 */
		bool execute();
		
		/**
		 * Stop the script
		 */
		void stop();
		
		/// Is the script running
		bool running() const {return executing;}
		
		/// Get the name of the script
		QString name() const;
		
		/// Get the icon name of the script
		QString iconName() const;
		
		/// Get the file
		QString scriptFile() const {return file;}
	private:
		QString file;
		Kross::Action* action;
		bool executing;
	};

}

#endif
