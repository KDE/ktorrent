/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#include <kgenericfactory.h>

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/plugin.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

#include <QDir>

#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <kglobal.h>

#include "scanfolder.h"
#include "scanfolderplugin.h"
#include "scanfolderprefpage.h"
#include "scanfolderpluginsettings.h"
#include "torrentloadqueue.h"
#include "scanthread.h"

using namespace bt;

K_EXPORT_COMPONENT_FACTORY(ktscanfolderplugin, KGenericFactory<kt::ScanFolderPlugin>("scanfolderplugin"))

namespace kt
{

    ScanFolderPlugin::ScanFolderPlugin(QObject* parent, const QStringList& args)
        : Plugin(parent),
          tlq(0)
    {
        Q_UNUSED(args);
    }


    ScanFolderPlugin::~ScanFolderPlugin()
    {
    }

    void ScanFolderPlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18nc("plugin name", "Scan Folder"), SYS_SNF);
        tlq = new TorrentLoadQueue(getCore(), this);
        scanner = new ScanThread();
        connect(scanner, SIGNAL(found(KUrl::List)), tlq, SLOT(add(KUrl::List)), Qt::QueuedConnection);
        pref = new ScanFolderPrefPage(this, 0);
        getGUI()->addPrefPage(pref);
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(updateScanFolders()));
        scanner->start(QThread::IdlePriority);
        updateScanFolders();
    }

    void ScanFolderPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18nc("plugin name", "Scan Folder"));
        getGUI()->removePrefPage(pref);
        scanner->stop();
        delete scanner;
        scanner = 0;
        delete pref;
        pref = 0;
        delete tlq;
        tlq = 0;
    }

    void ScanFolderPlugin::updateScanFolders()
    {
        QStringList folders = ScanFolderPluginSettings::folders();

        // make sure folders end with /
        for (QStringList::iterator i = folders.begin(); i != folders.end(); i++)
        {
            if (!i->endsWith(bt::DirSeparator()))
                (*i) += bt::DirSeparator();
        }

        if (ScanFolderPluginSettings::actionDelete())
            tlq->setLoadedTorrentAction(DeleteAction);
        else if (ScanFolderPluginSettings::actionMove())
            tlq->setLoadedTorrentAction(MoveAction);
        else
            tlq->setLoadedTorrentAction(DefaultAction);

        scanner->setRecursive(ScanFolderPluginSettings::recursive());
        scanner->setFolderList(folders);
    }

    bool ScanFolderPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }
}
