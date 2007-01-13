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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KTZEROCONFPLUGIN_H
#define KTZEROCONFPLUGIN_H

#include <util/ptrmap.h>
#include <interfaces/plugin.h>

namespace kt
{
	class TorrentInterface;
	class AvahiService;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Plugin which handles the zeroconf service.
	 */
	class ZeroConfPlugin : public Plugin
	{
		Q_OBJECT
	public:
		ZeroConfPlugin(QObject* parent, const char* name, const QStringList& args);
		virtual ~ZeroConfPlugin();
		
		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
		
	private slots:
		/**
		 * A TorrentInterface was added
		 * @param tc 
		 */
		void torrentAdded(kt::TorrentInterface* tc);

		/**
		 * A TorrentInterface was removed
		 * @param tc
		 */
		void torrentRemoved(kt::TorrentInterface* tc);
	
		/**
		 * An AvahiService has been destroyed by the psman
		 */
		void avahiServiceDestroyed(AvahiService* av);
		
	private:
		bt::PtrMap<kt::TorrentInterface*,AvahiService> services;
	};

}

#endif
