/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <fcntl.h>
#include <exception>

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>

#include <KAboutData>
#include <KCrash>
#include <KDBusService>
#include <KLocalizedString>
#include <KStartupInfo>
#include <KWindowSystem>

#include <torrent/globals.h>
#include <interfaces/functions.h>
#include <utp/connection.h>
#include "gui.h"


#include "version.h"
#include "ktversion.h"
#include <torrent/globals.h>
#include <util/error.h>
#include <util/log.h>
#include <util/functions.h>
#ifndef Q_OS_WIN
#include <util/signalcatcher.h>
#endif


using namespace bt;

#ifndef Q_WS_WIN
bool GrabPIDLock()
{
    // open the PID file in the /tmp directory and attempt to lock it
    QString pid_file = QDir::tempPath() + QStringLiteral("/.ktorrent_kf5_%1.lock").arg(getuid());

    int fd = open(QFile::encodeName(pid_file).data(), O_RDWR | O_CREAT, 0640);
    if (fd < 0)
    {
        fprintf(stderr, "Failed to open KT lock file %s : %s\n", pid_file.toLatin1().constData(), strerror(errno));
        return false;
    }

    if (lockf(fd, F_TLOCK, 0) < 0)
    {
        fprintf(stderr, "Failed to get lock on %s : %s\n", pid_file.toLatin1().constData(), strerror(errno));
        return false;
    }

    char str[20];
    sprintf(str, "%d\n", getpid());
    write(fd, str, strlen(str)); /* record pid to lockfile */

    // leave file open, so nobody else can lock it until KT exists
    return true;
}
#endif


int main(int argc, char** argv)
{
#ifndef Q_WS_WIN
    // ignore SIGPIPE and SIGXFSZ
    signal(SIGPIPE, SIG_IGN);
    signal(SIGXFSZ, SIG_IGN);
#endif

    if (!bt::InitLibKTorrent())
    {
        fprintf(stderr, "Failed to initialize libktorrent\n");
        return -1;
    }

    bt::SetClientInfo(QStringLiteral("KTorrent"), kt::MAJOR, kt::MINOR, kt::RELEASE, kt::VERSION_TYPE, QStringLiteral("KT"));

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    KLocalizedString::setApplicationDomain("ktorrent");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("ktorrent")));
    KCrash::initialize();

    QCommandLineParser parser;
    KAboutData about(QStringLiteral("ktorrent"), i18nc("@title", "KTorrent"), QStringLiteral(KT_VERSION_MACRO), i18n("Bittorrent client by KDE"),
                     KAboutLicense::GPL, i18nc("@info:credit", "(C) 2005 - 2011 Joris Guisson and Ivan Vasic"), QString(),
                     QStringLiteral("http://www.kde.org/applications/internet/ktorrent/"));

    about.setOrganizationDomain(QByteArray("kde.org"));
    about.setDesktopFileName(QStringLiteral("org.kde.ktorrent"));

    about.addAuthor(i18n("Joris Guisson"), QString(), QStringLiteral("joris.guisson@gmail.com"), QStringLiteral("http://kde.org/applications/internet/ktorrent"));
    about.addAuthor(i18n("Ivan Vasic"), QString(), QStringLiteral("ivasic@gmail.com"));
    about.addAuthor(i18n("Alan Jones"), i18n("BitFinder Plugin"), QStringLiteral("skyphyr@gmail.com"));
    about.addCredit(i18n("Diego Rosario Brogna"), i18n("Webinterface Plugin, global max share ratio patch"), QStringLiteral("dierbro@gmail.com"));
    about.addAuthor(i18n("Krzysztof Kundzicz"), i18n("Statistics Plugin"), QStringLiteral("athantor@gmail.com"));
    about.addAuthor(i18n("Christian Weilbach"), i18n("kio-magnet"), QStringLiteral("christian_weilbach@web.de"));
    about.addCredit(i18n("Mladen Babic"),  i18n("Application icon and a couple of others"), QStringLiteral("bmladen@EUnet.yu"));
    about.addCredit(i18n("Adam Treat"), QString(), QStringLiteral("treat@kde.org"));
    about.addCredit(i18n("Danny Allen"), i18n("1.0 application icon"), QStringLiteral("danny@dannyallen.co.uk"));
    about.addCredit(i18n("Vincent Wagelaar"), QString(), QStringLiteral("vincent@ricardis.tudelft.nl"));
    about.addCredit(i18n("Knut Morten Johansson"), QString(), QStringLiteral("knut@johansson.com"));
    about.addCredit(i18n("Felix Berger"),  i18n("ChunkBar's tooltip and IWFileTreeItem sorting"), QStringLiteral("bflat1@gmx.net"));
    about.addCredit(i18n("Andreas Kling"), QString(), QStringLiteral("kling@impul.se"));
    about.addCredit(i18n("Felipe Sateler"), QString(), QStringLiteral("fsateler@gmail.com"));
    about.addCredit(i18n("Maxmind"), i18n("Country locator for InfoWidget plugin. Flags are taken from http://flags.blogpotato.de/ so thanks to them too."), QString(), QStringLiteral("http://www.maxmind.com/"));
    about.addCredit(i18n("Adam Forsyth"), i18n("File prioritization and some other patches"), QStringLiteral("agforsyth@gmail.com"));
    about.addCredit(i18n("Thomas Bernard"), i18n("Miniupnp was used as an example for our own UPnP implementation"), QString(), QStringLiteral("http://miniupnp.free.fr/"));
    about.addCredit(i18n("Lesly Weyts"), i18n("Zeroconf enhancements"));
    about.addCredit(i18n("Kevin Andre"), i18n("Zeroconf enhancements"), QString(), QStringLiteral("http://users.edpnet.be/hyperquantum/"));
    about.addCredit(i18n("Dagur Valberg Johannsson"), i18n("Coldmilk webgui"), QStringLiteral("dagurval@pvv.ntnu.no"));
    about.addCredit(i18n("Alexander Dymo"), i18n("IDEAl code from KDevelop"), QStringLiteral("adymo@kdevelop.org"));
    about.addCredit(i18n("Scott Wolchok"), i18n("Conversion speed improvement in ipfilter plugin"), QStringLiteral("swolchok@umich.edu"));
    about.addCredit(i18n("Bryan Burns of Juniper Networks"), i18n("Discovered 2 security vulnerabilities (both are fixed)"));
    about.addCredit(i18n("Goten Xiao"), i18n("Patch to load silently with a save location"));
    about.addCredit(i18n("Rapsys"), i18n("Fixes in PHP code of webinterface"));
    about.addCredit(i18n("Athantor"), i18n("XFS specific disk preallocation"));
    about.addCredit(i18n("twisted_fall"), i18n("Patch to not show very low speeds"), QStringLiteral("twisted.fall@gmail.com"));
    about.addCredit(i18n("Lucke"), i18n("Patch to show potentially firewalled status"));
    about.addCredit(i18n("Modestas Vainius"), i18n("Several patches"), QStringLiteral("modestas@vainius.eu"));
    about.addCredit(i18n("Stefan Monov"), i18n("Patch to hide menu bar"), QStringLiteral("logixoul@gmail.com"));
    about.addCredit(i18n("The_Kernel"), i18n("Patch to change file priorities in the webgui"), QStringLiteral("kernja@cs.earlham.edu"));
    about.addCredit(i18n("Rafał Miłecki"), i18n("Several webgui patches"), QStringLiteral("zajec5@gmail.com"));
    about.addCredit(i18n("Ozzi"), i18n("Fixes for several warnings"), QStringLiteral("ossi@masiina.no-ip.info"));
    about.addCredit(i18n("Markus Brueffer"), i18n("Patch to fix free diskspace calculation on FreeBSD"), QStringLiteral("markus@brueffer.de"));
    about.addCredit(i18n("Lukas Appelhans"), i18n("Patch to fix a crash in ChunkDownloadView"), QStringLiteral("l.appelhans@gmx.de"));
    about.addCredit(i18n("Rickard Närström"), i18n("A couple of bugfixes"), QStringLiteral("rickard.narstrom@gmail.com"));
    about.addCredit(i18n("caruccio"), i18n("Patch to load torrents silently from the command line"), QStringLiteral("mateus@caruccio.com"));
    about.addCredit(i18n("Lee Olson"), i18n("New set of icons"), QStringLiteral("leetolson@gmail.com"));
    about.addCredit(i18n("Aaron J. Seigo"), i18n("Drag and drop support for Plasma applet"), QStringLiteral("aseigo@kde.org"));
    about.addCredit(i18n("Ian Higginson"), i18n("Patch to cleanup the plugin list"), QStringLiteral("xeriouxi@fastmail.fm"));
    about.addCredit(i18n("Amichai Rothman"), i18n("Patch to make the Plasma applet a popup applet"), QStringLiteral("amichai@amichais.net"));
    about.addCredit(i18n("Leo Trubach"), i18n("Patch to add support for IP ranges in IP filter dialog"), QStringLiteral("leotrubach@gmail.com"));
    about.addCredit(i18n("Andrei Barbu"), i18n("Feature which adds the date a torrent was added"), QStringLiteral("andrei@0xab.com"));
    about.addCredit(i18n("Jonas Lundqvist"), i18n("Feature to disable authentication in the webinterface"), QStringLiteral("jonas@gannon.se"));
    about.addCredit(i18n("Jaroslaw Swierczynski"), i18n("Exclusion patterns in the syndication plugin"), QStringLiteral("swiergot@gmail.com"));
    about.addCredit(i18n("Alexey Shildyakov "), i18n("Patch to rename single file torrents to the file inside"), QStringLiteral("ashl1future@gmail.com"));
    about.addCredit(i18n("Maarten De Meyer"), i18n("Fix for bug 305379"), QStringLiteral("de.meyer.maarten@gmail.com"));
    about.addCredit(i18n("Rex Dieter"), i18n("Add support for x-scheme-handler/magnet mimetype"), QStringLiteral("rdieter@gmail.com"));
    about.addCredit(i18n("Leszek Lesner"), i18n("Fix for bug 339584"), QStringLiteral("leszek.lesner@web.de"));
    about.addCredit(i18n("Andrius Štikonas"), i18n("KF5 porting"), QStringLiteral("andrius@stikonas.eu"));
    about.addCredit(i18n("Nick Shaforostoff"), i18n("KF5 porting"), QStringLiteral("shaforostoff@gmail.com"));

    KAboutData::setApplicationData(about);
    about.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("verbose"), i18n("Enable logging to standard output")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("silent"), i18n( "Silently open torrent given on URL")));
    parser.addOption(QCommandLineOption(QStringList() <<  QStringLiteral("+[URL]"), i18n( "Document to open" )));
    parser.process(app);
    about.processCommandLine(&parser);

    const KDBusService dbusService(KDBusService::Unique);

#if 0 //ndef Q_WS_WIN
    // need to grab lock after the fork call in start, otherwise this will not work properly
    if (!GrabPIDLock())
    {
        fprintf(stderr, "ktorrent is already running !\n");
        return 0;

    }
#endif

    try
    {
#ifndef Q_WS_WIN
        bt::SignalCatcher catcher;
        catcher.catchSignal(SIGINT);
        catcher.catchSignal(SIGTERM);
        QObject::connect(&catcher, &bt::SignalCatcher::triggered, &app, &QApplication::quit);
#endif

        const bool logToStdout = parser.isSet(QStringLiteral("verbose"));
        bt::InitLog(kt::DataDir(kt::CreateIfNotExists) + QLatin1String("log"), true, true, logToStdout);

        kt::GUI widget;
        widget.show();

        auto handleCmdLine = [&widget, &parser](const QStringList &arguments, const QString &workingDirectory)
        {
            if (!arguments.isEmpty())
            {
                parser.parse(arguments);
                KStartupInfo::setNewStartupId(&widget, KStartupInfo::startupId());
                KWindowSystem::forceActiveWindow(widget.winId());
            }
            QString oldCurrent = QDir::currentPath();
            if (!workingDirectory.isEmpty())
                QDir::setCurrent(workingDirectory);

            bool silent = parser.isSet(QStringLiteral("silent"));
            auto loadMethod = silent ? &kt::GUI::loadSilently : &kt::GUI::load;
            const auto positionalArguments = parser.positionalArguments();
            for (const QString& filePath : positionalArguments)
            {
                QUrl url = QFile::exists(filePath) ? QUrl::fromLocalFile(filePath) : QUrl(filePath);
                (widget.*loadMethod)(url);
            }

            if (!workingDirectory.isEmpty())
                QDir::setCurrent(oldCurrent);
        };
        QObject::connect(&dbusService, &KDBusService::activateRequested, handleCmdLine);
        handleCmdLine(QStringList(), QString());

        app.setQuitOnLastWindowClosed(false);
        app.exec();
    }
    catch (bt::Error& err)
    {
        Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.toString() << endl;
    }
    catch (std::exception& err)
    {
        Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.what() << endl;
    }
    catch (...)
    {
        Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught unknown exception " << endl;
    }
    bt::Globals::cleanup();
    return 0;
}



