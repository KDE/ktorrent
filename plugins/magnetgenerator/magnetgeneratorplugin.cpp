/***************************************************************************
*   Copyright (C) 2010 by Jonas Lundqvist                                 *
*   jonas@gannon.se                                                       *
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
#include "magnetgeneratorplugin.h"

#include <kpluginfactory.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <QUrl>
#include <qclipboard.h>
#include <qtooltip.h>
#include <kpassivepopup.h>
#include <QIcon>

#include <interfaces/guiinterface.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include <tracker/tracker.h>
#include <util/sha1hash.h>
#include <ktorrent/gui.h>
#include "magnetgeneratorprefwidget.h"
#include "magnetgeneratorpluginsettings.h"

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_magnetgenerator, "ktorrent_magnetgenerator.json", registerPlugin<kt::MagnetGeneratorPlugin>();)

using namespace bt;
namespace kt
{
    MagnetGeneratorPlugin::MagnetGeneratorPlugin(QObject* parent, const QVariantList& args) : Plugin(parent)
    {
        Q_UNUSED(args);
        pref = 0;
        generate_magnet_action = new QAction(QIcon::fromTheme(QStringLiteral("kt-magnet")), i18n("Copy Magnet URI"), this);
        connect(generate_magnet_action, SIGNAL(triggered()), this, SLOT(generateMagnet()));
        actionCollection()->addAction(QStringLiteral("generate_magnet"), generate_magnet_action);
        setXMLFile(QStringLiteral("ktorrent_magnetgeneratorui.rc"));

    }

    MagnetGeneratorPlugin::~MagnetGeneratorPlugin()
    {
    }

    void MagnetGeneratorPlugin::load()
    {
        pref = new MagnetGeneratorPrefWidget(0);
        getGUI()->addPrefPage(pref);
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->addViewListener(this);
        currentTorrentChanged(ta->getCurrentTorrent());
    }

    bool MagnetGeneratorPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }

    void MagnetGeneratorPlugin::unload()
    {
        getGUI()->removePrefPage(pref);
        delete pref;
        pref = 0;
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->removeViewListener(this);
    }

    void MagnetGeneratorPlugin::currentTorrentChanged(bt::TorrentInterface* tc)
    {
        generate_magnet_action->setEnabled(tc && (!tc->getStats().priv_torrent || !MagnetGeneratorPluginSettings::onlypublic()));
    }

    void MagnetGeneratorPlugin::generateMagnet()
    {
        bt::TorrentInterface* tor = getGUI()->getTorrentActivity()->getCurrentTorrent();
        if (!tor)
            return;

        QUrl dn(tor->getStats().torrent_name);
        SHA1Hash ih(tor->getInfoHash());

        QString uri = QLatin1String("magnet:?xt=urn:btih:") + ih.toString();

        if (MagnetGeneratorPluginSettings::dn())
        {
            uri += QLatin1String("&dn=") + QUrl::toPercentEncoding(dn.toString(), QByteArrayLiteral("{}"), NULL);
        }

        if ((MagnetGeneratorPluginSettings::customtracker() && MagnetGeneratorPluginSettings::tr().length() > 0) && !MagnetGeneratorPluginSettings::torrenttracker())
        {
            uri += (QLatin1String("&tr=")) + QUrl::toPercentEncoding(QUrl(MagnetGeneratorPluginSettings::tr()).toString(), QByteArrayLiteral("{}"), NULL);
        }

        if (MagnetGeneratorPluginSettings::torrenttracker())
        {
            QList<bt::TrackerInterface*> trackers = tor->getTrackersList()->getTrackers();
            if (!trackers.isEmpty())
            {
                Tracker* trk = (Tracker*)trackers.first();

                uri += QLatin1String("&tr=") + QUrl::toPercentEncoding(QUrl(trk->trackerURL()).toString(), QByteArrayLiteral("{}"), NULL);
            }

        }

        addToClipboard(uri);

        if (MagnetGeneratorPluginSettings::popup())
            showPopup();

    }

    void MagnetGeneratorPlugin::addToClipboard(QString uri)
    {
        QClipboard* cb = QApplication::clipboard();
        cb->setText(uri, QClipboard::Clipboard);
        cb->setText(uri, QClipboard::Selection);
    }

    void MagnetGeneratorPlugin::showPopup()
    {
        KPassivePopup::message(i18n("Magnet"), i18n("Magnet link copied to clipboard"),
                               QIcon::fromTheme(QStringLiteral("kt-magnet")).pixmap(20, 20), getGUI()->getMainWindow(), 3000);
    }

}

#include "magnetgeneratorplugin.moc"
