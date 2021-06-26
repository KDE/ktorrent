/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UPNPTESTWIDGET_HH
#define UPNPTESTWIDGET_HH

#include "ui_upnptestwidget.h"
#include <QWidget>
#include <interfaces/logmonitorinterface.h>

namespace bt
{
class UPnPMCastSocket;
class UPnPRouter;
}

class UPnPTestWidget : public QWidget, public Ui_UPnPTestWidget, public bt::LogMonitorInterface
{
public:
    UPnPTestWidget(QWidget *parent = nullptr);
    ~UPnPTestWidget() override;

    void doForward();
    void undoForward();
    void findRouters();
    void discovered(bt::UPnPRouter *r);
    void verboseModeChecked(bool on);

private:
    void message(const QString &line, unsigned int arg) override;

    bt::UPnPMCastSocket *mcast_socket;
    bt::UPnPRouter *router;
};

#endif
