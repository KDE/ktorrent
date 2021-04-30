/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <QString>
#include <ktcore_export.h>
#include <util/constants.h>

namespace kt
{
enum CreationMode {
    DoNotCheckDirPresence,
    CreateIfNotExists,
};
/// Get the data directory of ktorrent (~/.local/share/ktorrent most of the time)
KTCORE_EXPORT QString DataDir(CreationMode mode = DoNotCheckDirPresence);

/// Apply all settings
KTCORE_EXPORT void ApplySettings();

/// Get the filter string for torrent files used file dialogs
KTCORE_EXPORT QString TorrentFileFilter(bool all_files_included);

}

#endif
