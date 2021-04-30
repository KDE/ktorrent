/*
    SPDX-FileCopyrightText: 2010 Jonas Lundqvist <jonas@gannon.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "magnetgeneratorplugin.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KMainWindow>
#include <KNotification>
#include <KPluginFactory>

#include <QClipboard>
#include <QIcon>
#include <QToolTip>
#include <QUrl>

#include "magnetgeneratorpluginsettings.h"
#include "magnetgeneratorprefwidget.h"
#include <interfaces/guiinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include <interfaces/torrentinterface.h>
#include <ktorrent/gui.h>
#include <tracker/tracker.h>
#include <util/sha1hash.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_magnetgenerator, "ktorrent_magnetgenerator.json", registerPlugin<kt::MagnetGeneratorPlugin>();)

using namespace bt;
namespace kt
{
MagnetGeneratorPlugin::MagnetGeneratorPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
{
    Q_UNUSED(args);
    pref = nullptr;
    generate_magnet_action = new QAction(QIcon::fromTheme(QStringLiteral("kt-magnet")), i18n("Copy Magnet URI"), this);
    connect(generate_magnet_action, &QAction::triggered, this, &MagnetGeneratorPlugin::generateMagnet);
    actionCollection()->addAction(QStringLiteral("generate_magnet"), generate_magnet_action);
    setXMLFile(QStringLiteral("ktorrent_magnetgeneratorui.rc"));
}

MagnetGeneratorPlugin::~MagnetGeneratorPlugin()
{
}

void MagnetGeneratorPlugin::load()
{
    pref = new MagnetGeneratorPrefWidget(nullptr);
    getGUI()->addPrefPage(pref);
    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->addViewListener(this);
    currentTorrentChanged(ta->getCurrentTorrent());
}

bool MagnetGeneratorPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}

void MagnetGeneratorPlugin::unload()
{
    getGUI()->removePrefPage(pref);
    delete pref;
    pref = nullptr;
    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->removeViewListener(this);
}

void MagnetGeneratorPlugin::currentTorrentChanged(bt::TorrentInterface *tc)
{
    generate_magnet_action->setEnabled(tc && (!tc->getStats().priv_torrent || !MagnetGeneratorPluginSettings::onlypublic()));
}

void MagnetGeneratorPlugin::generateMagnet()
{
    bt::TorrentInterface *tor = getGUI()->getTorrentActivity()->getCurrentTorrent();
    if (!tor)
        return;

    QUrl dn(tor->getStats().torrent_name);
    SHA1Hash ih(tor->getInfoHash());

    QString uri = QStringLiteral("magnet:?xt=urn:btih:") + ih.toString();

    if (MagnetGeneratorPluginSettings::dn()) {
        uri += QStringLiteral("&dn=") + QString::fromLatin1(QUrl::toPercentEncoding(dn.toString(), QByteArrayLiteral("{}"), nullptr));
    }

    if ((MagnetGeneratorPluginSettings::customtracker() && MagnetGeneratorPluginSettings::tr().length() > 0)
        && !MagnetGeneratorPluginSettings::torrenttracker()) {
        uri += (QStringLiteral("&tr="))
            + QString::fromLatin1(QUrl::toPercentEncoding(QUrl(MagnetGeneratorPluginSettings::tr()).toString(), QByteArrayLiteral("{}"), nullptr));
    }

    if (MagnetGeneratorPluginSettings::torrenttracker()) {
        QList<bt::TrackerInterface *> trackers = tor->getTrackersList()->getTrackers();
        if (!trackers.isEmpty()) {
            Tracker *trk = (Tracker *)trackers.first();

            uri += QLatin1String("&tr=") + QString::fromLatin1(QUrl::toPercentEncoding(QUrl(trk->trackerURL()).toString(), QByteArrayLiteral("{}"), nullptr));
        }
    }

    addToClipboard(uri);

    if (MagnetGeneratorPluginSettings::popup())
        showPopup();
}

void MagnetGeneratorPlugin::addToClipboard(QString uri)
{
    QClipboard *cb = QApplication::clipboard();
    cb->setText(uri, QClipboard::Clipboard);
    cb->setText(uri, QClipboard::Selection);
}

void MagnetGeneratorPlugin::showPopup()
{
    KNotification::event(QStringLiteral("MagnetLinkCopied"), i18n("Magnet link copied to clipboard"), QString(), QStringLiteral("kt-magnet"));
}

}

#include "magnetgeneratorplugin.moc"
