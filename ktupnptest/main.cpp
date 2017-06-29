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

#include <cstdio>
#include <cstdlib>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>

#include <KAboutData>
#include <KLocalizedString>

#include <util/functions.h>
#include <util/log.h>
#include "upnptestwidget.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    KAboutData about(QStringLiteral("ktupnp"), i18n("KTUPnPTest"),
                     QStringLiteral("1.0"), i18n("KTorrent's UPnP test application"),
                     KAboutLicense::GPL,
                     i18n("&copy; 2005 - 2007 Joris Guisson and Ivan Vasic"),
                     QString(),
                     QStringLiteral("http://www.kde.org/applications/internet/ktorrent/"));
    KAboutData::setApplicationData(about);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    if (!bt::InitLibKTorrent())
    {
        fprintf(stderr, "Failed to initialize libktorrent\n");
        return -1;
    }

    QString str = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktorrent");
    if (!str.endsWith(bt::DirSeparator()))
        str += bt::DirSeparator();

    bt::InitLog(str + QStringLiteral("ktupnptest.log"));
    UPnPTestWidget* mwnd = new UPnPTestWidget();

    mwnd->show();
    app.exec();
    return 0;
}
