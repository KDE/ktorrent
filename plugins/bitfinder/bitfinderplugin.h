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

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

namespace kt
	{

	class BitFinderPlugin : public Plugin, public CloseTabListener
		{
			Q_OBJECT

		public:
			BitFinderPlugin (QObject* parent, const QStringList& args);
			virtual ~BitFinderPlugin();

			virtual void load();
			virtual void unload();
			virtual bool versionCheck (const QString& version) const;

		private:
// 			void setupActions();
			virtual void tabCloseRequest (kt::GUIInterface* gui, QWidget* tab);

		private slots:
// 			void enableActions(unsigned int flags);
// 			void onSelectionChanged(const QModelIndex & idx);
// 			void onDoubleClicked(const QModelIndex & idx);

		private:
// 			KAction* play_action;
// 			int action_flags;
// 			QModelIndex curr_item;
		};

	}

#endif

