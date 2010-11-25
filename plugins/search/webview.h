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



#ifndef KT_WEBVIEW_H
#define KT_WEBVIEW_H

#include <KWebView>


namespace kt
{

	class WebView : public KWebView
	{
		Q_OBJECT
		
	public:
		WebView(QWidget* parentWidget = 0);
		virtual ~WebView();
		
		void openUrl(const KUrl & url);
		void home();
		
	protected:
		void loadHomePage();
		
	private:
		QString home_page_html;
		QString home_page_base_url;
	};

}

#endif // KT_HOMEPAGE_H
