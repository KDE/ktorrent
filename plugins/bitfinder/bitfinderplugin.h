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
#ifndef KTBITFINDERPLUGIN_H
#define KTBITFINDERPLUGIN_H

#include <kaction.h>
#include <kactionmenu.h>

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

#include "source/sourcesview.h"

#include "filter/filterlistmodel.h"
#include "filter/filtersview.h"
#include "filter/filterdetails.h"

namespace kt
	{

	class BitFinderPlugin : public Plugin
		{
			Q_OBJECT

		public:
			BitFinderPlugin (QObject* parent, const QStringList& args);
			virtual ~BitFinderPlugin();

			virtual void load();
			virtual void unload();
			virtual bool versionCheck (const QString& version) const;

		private:
 			void setupSourcesActions();

		private slots:
// 			void onDoubleClicked(const QModelIndex & idx);

		private:
			//the config directory name
			QString configDirName ;
			
			//Sources Variables
			SourcesView * sourcesView;
 			KActionMenu * addSourceMenu;
 			KAction * removeSource;
 			
 			//source types
 			KAction* addRssSource;
 			
 			//Filters Variables
 			FilterListModel * filterListModel;
 			FiltersView * filtersView;

		};

	}

#endif

