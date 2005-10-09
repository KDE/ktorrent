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
#include <kgenericfactory.h>
#include "searchplugin.h"


K_EXPORT_COMPONENT_FACTORY(ktsearchplugin,KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))

namespace kt
{
	const QString NAME = "searchplugin";
	const QString AUTHOR = "Joris Guisson";
	const QString EMAIL = "joris.guisson@gmail.com";
	const QString DESCRIPTION = "KTorrent's search plugin";

	SearchPlugin::SearchPlugin(QObject* parent, const char* name, const QStringList& args)
	: Plugin(parent, name, args,NAME,AUTHOR,EMAIL,DESCRIPTION)
	{
		// setXMLFile("ktsearchpluginui.rc");
	}


	SearchPlugin::~SearchPlugin()
	{}


	void SearchPlugin::load()
	{}

	void SearchPlugin::unload()
	{}

}
#include "searchplugin.moc"
