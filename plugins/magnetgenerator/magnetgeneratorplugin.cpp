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
#include <kgenericfactory.h>
#include <kmainwindow.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <qurl.h>
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
#include "magnetgeneratorplugin.h"
#include "magnetgeneratorpluginsettings.h"

K_EXPORT_COMPONENT_FACTORY(ktmagnetgeneratorplugin, KGenericFactory<kt::MagnetGeneratorPlugin>("ktmagnetgeneratorplugin"))

using namespace bt;
namespace kt
{
    MagnetGeneratorPlugin::MagnetGeneratorPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
    {
        Q_UNUSED(args);
        pref = 0;
        generate_magnet_action = new QAction(QIcon::fromTheme("kt-magnet"), i18n("Copy Magnet URI"), this);
        connect(generate_magnet_action, SIGNAL(triggered()), this, SLOT(generateMagnet()));
        actionCollection()->addAction("generate_magnet", generate_magnet_action);
        setXMLFile("ktmagnetgeneratorpluginui.rc");

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

        QString uri("magnet:?xt=urn:btih:");
        uri.append(ih.toString());

        if (MagnetGeneratorPluginSettings::dn())
        {
            uri.append("&dn=");
            uri.append(QUrl::toPercentEncoding(dn.toString(), "{}", NULL));
        }

        if ((MagnetGeneratorPluginSettings::customtracker() && MagnetGeneratorPluginSettings::tr().length() > 0) && !MagnetGeneratorPluginSettings::torrenttracker())
        {
            uri.append("&tr=");
            QUrl tr(MagnetGeneratorPluginSettings::tr());
            uri.append(QUrl::toPercentEncoding(tr.toString(), "{}", NULL));
        }

        if (MagnetGeneratorPluginSettings::torrenttracker())
        {
            QList<bt::TrackerInterface*> trackers = tor->getTrackersList()->getTrackers();
            if (!trackers.isEmpty())
            {
                Tracker* trk = (Tracker*)trackers.first();
                QUrl tr(trk->trackerURL());

                uri.append("&tr=");
                uri.append(QUrl::toPercentEncoding(tr.toString(), "{}", NULL));
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
                               QIcon::fromTheme("kt-magnet").pixmap(20, 20), getGUI()->getMainWindow(), 3000);
    }

}

#include "magnetgeneratorplugin.moc"
