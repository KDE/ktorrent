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
#include <stdio.h>
#include <signal.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include "app.h"


int main(int argc, char **argv)
{
	// ignore SIGPIPE and SIGXFSZ
	signal(SIGPIPE,SIG_IGN);
	signal(SIGXFSZ,SIG_IGN);
	
	KAboutData about(
		"ktorrent", 0, ki18n("KTorrent"),
		"3.0dev", ki18n("Bittorrent client for KDE"),
		KAboutData::License_GPL, 
		ki18n("(C) 2005 - 2007 Joris Guisson and Ivan Vasic"), 
		KLocalizedString(),
		"http://www.ktorrent.org/");

	about.addAuthor(ki18n("Joris Guisson"), KLocalizedString(), "joris.guisson@gmail.com","http://ktorrent.org" );
	about.addAuthor(ki18n("Ivan Vasic"),KLocalizedString(),"ivasic@gmail.com");
	about.addAuthor(ki18n("Alan Jones"),ki18n("RSS Plugin"),"skyphyr@gmail.com");
	about.addAuthor(ki18n("Diego R. Brogna"),ki18n("Webinterface Plugin"),"dierbro@gmail.com");
	about.addAuthor(ki18n("Krzysztof Kundzicz"),ki18n("Statistics Plugin"),"athantor@gmail.com"); 

	about.addCredit(ki18n("Mladen Babic"),	ki18n("Application icon and a couple of others"),"bmladen@EUnet.yu");
	about.addCredit(ki18n("Adam Treat"), KLocalizedString(), "treat@kde.org" );
	about.addCredit(ki18n("Danny Allen"),ki18n("1.0 application icon"),"danny@dannyallen.co.uk");
	about.addCredit(ki18n("Vincent Wagelaar"),KLocalizedString(),"vincent@ricardis.tudelft.nl");
	about.addCredit(ki18n("Knut Morten Johansson"),KLocalizedString(),"knut@johansson.com");
	about.addCredit(ki18n("Felix Berger"),	ki18n("ChunkBar's tooltip and IWFileTreeItem sorting"),"bflat1@gmx.net");
	about.addCredit(ki18n("Andreas Kling"),KLocalizedString(),"kling@impul.se");
	about.addCredit(ki18n("Felipe Sateler"),KLocalizedString(),"fsateler@gmail.com");
	about.addCredit(ki18n("Maxmind"), ki18n("Country locator for InfoWidget plugin. Flags are taken from http://flags.blogpotato.de/ so thanks to them too."),0, "http://www.maxmind.com/");
	about.addCredit(ki18n("Adam Forsyth"),ki18n("File prioritization"),"agforsyth@gmail.com");
	about.addCredit(ki18n("Thomas Bernard"),ki18n("Miniupnp was used as an example for our own UPnP implementation"),0,"http://miniupnp.free.fr/");
	about.addCredit(ki18n("Diego Rosario Brogna"),ki18n("Global max share ratio patch"),0,"dierbro@gmail.com");
	about.addCredit(ki18n("Lesly Weyts"),ki18n("Zeroconf enhancements"),0,0);
	about.addCredit(ki18n("Kevin Andre"),ki18n("Zeroconf enhancements"),0,"http://users.edpnet.be/hyperquantum/");
	about.addCredit(ki18n("Dagur Valberg Johannsson"),ki18n("Coldmilk webgui"),"dagurval@pvv.ntnu.no");
	about.addCredit(ki18n("Alexander Dymo"),ki18n("IDEAl code from KDevelop"),"adymo@kdevelop.org");
	about.addCredit(ki18n("Scott Wolchok"),ki18n("Conversion speed improvement in ipfilter plugin"),"swolchok@umich.edu");
	about.addCredit(ki18n("Bryan Burns of Juniper Networks"),ki18n("Discovered 2 security vulnerabilities (both are fixed)"),0);
	about.addCredit(ki18n("Goten Xiao"),ki18n("Patch to load silently with a save location"),0);
	about.addCredit(ki18n("Rapsys"),ki18n("Fixes in PHP code of webinterface"),0);
	about.addCredit(ki18n("Athantor"),ki18n("XFS specific disk preallocation"),0);
	about.addCredit(ki18n("twisted_fall"),ki18n("Patch to not show very low speeds"),"twisted.fall@gmail.com");
	about.addCredit(ki18n("Lucke"),ki18n("Patch to show potentially firewalled status"),0);
	about.addCredit(ki18n("Modestas Vainius"),ki18n("Several patches"),"modestas@vainius.eu");
	about.addCredit(ki18n("Stefan Monov"),ki18n("Patch to hide menu bar"),"logixoul@gmail.com");

	KCmdLineArgs::init(argc, argv, &about);

	KCmdLineOptions options;
	options.add("+[Url]", ki18n("Document to open"));
	KCmdLineArgs::addCmdLineOptions(options);
	
	kt::App::addCmdLineOptions();
	if (!kt::App::start())
	{
		fprintf(stderr, "ktorrent is already running!\n");
		return 0;
	}
	
	try
	{
		kt::App app;
		app.exec();
	}
	catch (...)
	{
	//	fprintf(stderr, "Aborted by error : %s\n",e.toString().ascii());
	}
	return 0;
}



