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

#include <kgenericfactory.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>

#include "bitfinderplugin.h"

K_EXPORT_COMPONENT_FACTORY (ktmediaplayerplugin, KGenericFactory<kt::BitFinderPlugin> ("ktbitfinderplugin"))

using namespace bt;

namespace kt
	{

	BitFinderPlugin::BitFinderPlugin (QObject* parent, const QStringList& args) : Plugin (parent)
		{
		Q_UNUSED (args);
		}


	BitFinderPlugin::~BitFinderPlugin()
		{

		}

	void BitFinderPlugin::load()
		{

		}

	void BitFinderPlugin::unload()
		{

		}

	bool BitFinderPlugin::versionCheck (const QString& version) const
		{

		}

	void BitFinderPlugin::tabCloseRequest (kt::GUIInterface* gui, QWidget* tab)
		{
		//Check through the list of Source tabs

		//Check through the list of Filter tabs

		}

	}