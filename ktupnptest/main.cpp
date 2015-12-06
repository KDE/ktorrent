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
#include <stdio.h>
#include <stdlib.h>
#include <klocalizedstring.h>
#include <K4AboutData>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <util/functions.h>
#include <interfaces/functions.h>
#include <util/log.h>
#include "upnptestwidget.h"

using namespace kt;
using namespace bt;




int main(int argc, char** argv)
{
    K4AboutData about("ktupnp", 0, ki18n("KTUPnPTest"),
                     "1.0", ki18n("KTorrent's UPnP test application"),
                     K4AboutData::License_GPL,
                     ki18n("(C) 2005 - 2007 Joris Guisson and Ivan Vasic"),
                     KLocalizedString(),
                     "http://www.ktorrent.org/");
    KCmdLineArgs::init(argc, argv, &about);
    KApplication app;

    if (!bt::InitLibKTorrent())
    {
        fprintf(stderr, "Failed to initialize libktorrent\n");
        return -1;
    }

    QString str = KGlobal::dirs()->saveLocation("data", "ktorrent");
    if (!str.endsWith(bt::DirSeparator()))
        str += bt::DirSeparator();

    bt::InitLog(str + "ktupnptest.log");
    UPnPTestWidget* mwnd = new UPnPTestWidget();

    app.setTopWidget(mwnd);
    mwnd->show();
    app.exec();
    return 0;
}
