/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include "homepage.h"
#include <QFile>
#include <QTextCodec>
#include <QApplication>
#include <KStandardDirs>
#include <KLocale>
#include <util/log.h>

using namespace bt;

namespace kt
{
	
	HomePage::HomePage(QWidget* parentWidget, QObject* parent, KHTMLPart::GUIProfile prof)
		: KHTMLPart(parentWidget, parent, prof)
	{
		QTextCodec* codec = KGlobal::locale()->codecForEncoding();
		if (codec)
			setEncoding(codec->name(), true);
		else
			setEncoding("iso-8859-1", true);
	}
		
	HomePage::~HomePage()
	{
	}

	bool HomePage::openFile()
	{
		return true;
	}

	bool HomePage::openUrl(const KUrl& url)
	{
		if (url.url() == "about:ktorrent")
			home();
		else
			KHTMLPart::openUrl(url);
		return true;
	}
	
	void HomePage::home()
	{
		emit started(0);
		Out(SYS_SRC|LOG_DEBUG) << "Opening about:ktorrent" << endl;
		begin(KUrl("about:ktorrent"));
		write(serve());
		end();
		addToHistory(KUrl("about:ktorrent"));
		emit completed();
	}


	bool HomePage::urlSelected(const QString& url, int button, int state, const QString& target, const KParts::OpenUrlArguments& args, const KParts::BrowserArguments& browserArgs)
	{
		if (url == "about:ktorrent")
		{
			home();
			return true;
		}
		else
			return KHTMLPart::urlSelected(url, button, state, target, args, browserArgs);
	}

	QString HomePage::serve()
	{
		if (home_page_html.isEmpty())
			loadHomePage();
		
		return home_page_html;
	}

	void HomePage::loadHomePage()
	{
		QString file = KStandardDirs::locate("data","ktorrent/search/home/home.html");
		QFile fptr(file);
		if (fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_SRC|LOG_DEBUG) << "Loading home page from " << file << endl;
			home_page_html = QTextStream(&fptr).readAll();
			// otherwise all embedded objects are referenced as about:/...
			QString basehref = QLatin1String("<BASE HREF=\"file:") +
				file.left( file.lastIndexOf( '/' )) + QLatin1String("/\">\n");
			home_page_html.replace("<head>", "<head>\n\t" + basehref, Qt::CaseInsensitive);
			
			// %1
			home_page_html = home_page_html.arg("ktorrent_infopage.css");
			// %2
			if (qApp->layoutDirection() == Qt::RightToLeft)
				home_page_html = home_page_html.arg("@import \"%1\";").arg(KStandardDirs::locate("data", "kdeui/about/kde_infopage_rtl.css"));
			else
				home_page_html = home_page_html.arg( "" );
			
			KIconLoader *iconloader = KIconLoader::global();
			int icon_size = iconloader->currentSize(KIconLoader::Desktop);
			home_page_html = home_page_html
				.arg(i18n("Home")) // %3 Title
				.arg(i18n("KTorrent")) // %4
				.arg(i18nc("KDE 4 tag line, see http://kde.org/img/kde40.png", "Be free.")) // %5
				.arg(i18n("Search the web for torrents.")) // %6
				.arg(i18n("Search")) // %7
				.arg("search_text") // %8
				.arg(icon_size).arg(icon_size) // %9 and %10
				;
		}
		else
		{
			Out(SYS_SRC|LOG_IMPORTANT) << "Failed to load " << file << " : " << fptr.errorString() << endl;
		}
	}


}

