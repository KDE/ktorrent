/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#ifndef UPNPTESTWIDGET_HH
#define UPNPTESTWIDGET_HH

#include <QWidget>
#include <interfaces/logmonitorinterface.h>
#include "ui_upnptestwidget.h"

namespace bt
{
    class UPnPMCastSocket;
    class UPnPRouter;
}

class UPnPTestWidget : public QWidget, public Ui_UPnPTestWidget, public bt::LogMonitorInterface
{
public:
    UPnPTestWidget(QWidget* parent = 0);
    ~UPnPTestWidget();

    void doForward();
    void undoForward();
    void findRouters();
    void discovered(bt::UPnPRouter* r);
    void verboseModeChecked(bool on);

private:
    void message(const QString& line, unsigned int arg) override;

    bt::UPnPMCastSocket* mcast_socket;
    bt::UPnPRouter* router;
};

#endif
