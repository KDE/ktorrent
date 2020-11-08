/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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

#ifndef KTSCANFORLOSTFILESPLUGIN_H
#define KTSCANFORLOSTFILESPLUGIN_H

#include <interfaces/plugin.h>

class QString;
class QDockWidget;


namespace kt
{
class ScanForLostFilesPrefPage;
class ScanForLostFilesWidget;

enum SFLFPosition {
    SEPARATE_ACTIVITY = 0,
    DOCKABLE_WIDGET = 1,
    TORRENT_ACTIVITY = 2
};

/**
 * @author Alexander Trufanov <trufanovan@gmail.com>
 * @brief KTorrent ScanForLostFiles plugin
 * Display files in selected folder that do not belong to any torrent.
 */
class ScanForLostFilesPlugin : public Plugin
{
    Q_OBJECT
public:
    ScanForLostFilesPlugin(QObject* parent, const QVariantList& args);
    ~ScanForLostFilesPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString& version) const override;

public Q_SLOTS:
    void updateScanForLostFiles();

private:
    void addToGUI();
    void removeFromGUI();

private:
    ScanForLostFilesWidget* m_view;
    QDockWidget* m_dock;
    ScanForLostFilesPrefPage* m_pref;
    SFLFPosition m_pos;
};

}

#endif
