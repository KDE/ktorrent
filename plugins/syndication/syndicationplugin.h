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
#ifndef KTSYNDICATIONPLUGIN_H
#define KTSYNDICATIONPLUGIN_H

#include <interfaces/plugin.h>

class KAction;

namespace kt
{
	class SyndicationActivity;
	

	/**
		@author
	*/
	class SyndicationPlugin : public Plugin
	{
		Q_OBJECT
	public:
		SyndicationPlugin(QObject* parent,const QStringList& args);
		virtual ~SyndicationPlugin();

		virtual bool versionCheck(const QString& version) const;
		virtual void load();
		virtual void unload();
	
	private:
		void setupActions();
	
	private:
		KAction* add_feed;
		KAction* remove_feed;
		KAction* show_feed;
		KAction* add_filter;
		KAction* remove_filter;
		KAction* edit_filter;
		KAction* manage_filters;
		SyndicationActivity* activity;
		
		friend class SyndicationActivity;
	};

}

#endif
