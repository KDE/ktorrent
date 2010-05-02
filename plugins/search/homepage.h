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



#ifndef KT_HOMEPAGE_H
#define KT_HOMEPAGE_H

#include <khtml_part.h>


namespace kt
{

	class HomePage : public KHTMLPart
	{
		Q_OBJECT
		
	public:
		HomePage(QWidget* parentWidget = 0, QObject* parent = 0, GUIProfile prof = DefaultGUI);
		virtual ~HomePage();
		
		virtual bool openUrl(const KUrl & url);
		virtual bool openFile();
		
		void home();
		
		virtual void addToHistory(const KUrl & url) = 0;
		
	protected:
		virtual bool urlSelected(const QString &url, int button, int state, const QString &target,
								 const KParts::OpenUrlArguments& args = KParts::OpenUrlArguments(),
								 const KParts::BrowserArguments& browserArgs = KParts::BrowserArguments());
		
		void loadHomePage();
		QString serve();
		
	private:
		QString home_page_html;
	};

}

#endif // KT_HOMEPAGE_H
