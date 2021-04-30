/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cstdio>
#include <cstdlib>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStandardPaths>

#include <KAboutData>
#include <KLocalizedString>

#include "upnptestwidget.h"
#include <util/functions.h>
#include <util/log.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData about(QStringLiteral("ktupnp"),
                     i18n("KTUPnPTest"),
                     QStringLiteral("1.0"),
                     i18n("KTorrent's UPnP test application"),
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

    if (!bt::InitLibKTorrent()) {
        fprintf(stderr, "Failed to initialize libktorrent\n");
        return -1;
    }

    QString str = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktorrent");
    if (!str.endsWith(bt::DirSeparator()))
        str += bt::DirSeparator();

    bt::InitLog(str + QStringLiteral("ktupnptest.log"));
    UPnPTestWidget *mwnd = new UPnPTestWidget();

    mwnd->show();
    app.exec();
    return 0;
}
