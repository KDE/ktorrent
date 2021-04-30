/*
    SPDX-FileCopyrightText: 2010 Jonas Lundqvist <jonas@gannon.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_MAGNETGENERATORPLUGIN_H
#define KT_MAGNETGENERATORPLUGIN_H

#include <interfaces/plugin.h>
#include <interfaces/torrentactivityinterface.h>

namespace kt
{
class MagnetGeneratorPrefWidget;

class MagnetGeneratorPlugin : public Plugin, public ViewListener
{
    Q_OBJECT
public:
    MagnetGeneratorPlugin(QObject *parent, const QVariantList &args);
    ~MagnetGeneratorPlugin() override;

    void load() override;
    void unload() override;
    bool versionCheck(const QString &version) const override;
    QString parentPart() const override
    {
        return QStringLiteral("torrentactivity");
    }
    void currentTorrentChanged(bt::TorrentInterface *tc) override;

private Q_SLOTS:
    void generateMagnet();

private:
    MagnetGeneratorPrefWidget *pref;
    QAction *generate_magnet_action;
    void addToClipboard(QString uri);
    void showPopup();
};

}

#endif // KT_MAGNETGENERATORPLUGIN_H
