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

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <util/constants.h>
#include <ktcore_export.h>

namespace kt
{
    enum CreationMode {DoNotCheckDirPresence, CreateIfNotExists};
    /// Get the data directory of ktorrent (~/.local/share/ktorrent most of the time)
    KTCORE_EXPORT QString DataDir(CreationMode mode = DoNotCheckDirPresence);

    /// Apply all settings
    KTCORE_EXPORT void ApplySettings();

    /// Get the filter string for torrent files used file dialogs
    KTCORE_EXPORT QString TorrentFileFilter(bool all_files_included);

}

#endif
