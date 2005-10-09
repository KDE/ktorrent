/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTPLUGIN_H
#define KTPLUGIN_H

#include <qobject.h>
#include <kxmlguiclient.h>

namespace kt
{

	/**
	 * @author Joris Guisson
	 * @brief Base class for all plugins
	 *
	 * This is the base class for all plugins. 
	 */
	class Plugin : public QObject, public KXMLGUIClient
	{
		Q_OBJECT
	public:
		Plugin(QObject *parent = 0, const char *name = 0);
		virtual ~Plugin();

		virtual void load() = 0;
		virtual void unload() = 0;
	};

}

#endif
