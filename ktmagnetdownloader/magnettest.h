/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAGNETTEST_H
#define MAGNETTEST_H

#include <QObject>

#include <magnet/magnetdownloader.h>
#include <magnet/magnetlink.h>

namespace bt
{
class UPnPRouter;
class UPnPMCastSocket;
}

class MagnetTest : public QObject
{
public:
    MagnetTest(const bt::MagnetLink &mlink, QObject *parent = nullptr);
    ~MagnetTest() override;

    void routerDiscovered(bt::UPnPRouter *router);
    void start();
    void update();
    void foundMetaData(bt::MagnetDownloader *md, const QByteArray &data);

private:
    bt::MagnetLink mlink;
    bt::UPnPMCastSocket *upnp;
    bt::MagnetDownloader *mdownloader;
    QTimer timer;
};

#endif // MAGNETTEST_H
