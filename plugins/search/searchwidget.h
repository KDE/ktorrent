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
#ifndef BTSEARCHWIDGET_H
#define BTSEARCHWIDGET_H

#include <qwidget.h>
#include <kurl.h>
#include <ktoolbar.h>
#include <klineedit.h>

class QProgressBar;
class KMenu;
class KComboBox;

namespace KParts
{
	class Part;
}

namespace kt
{
	class HTMLPart;
	class SearchWidget;
	class SearchPlugin;
	
	
	/**
		@author Joris Guisson
		
		Widget which shows a KHTML window with the users search in it
	*/
	class SearchWidget : public QWidget
	{
		Q_OBJECT
	public:
		SearchWidget(SearchPlugin* sp);
		virtual ~SearchWidget();
	
		KMenu* rightClickMenu();
		
		QString getSearchText() const {return search_text->text();}
		KUrl getCurrentUrl() const;
		QString getSearchBarText() const;
		int getSearchBarEngine() const;
		void setSearchBarEngine(int engine);
		
		bool backAvailable() const;
		
	signals:
		void enableBack(bool on);
		void openNewTab(const KUrl & url);
		void changeTitle(SearchWidget* w,const QString & title);
	
	public slots:
		void search(const QString & text,int engine = 0);
		void copy();
		void copyUrl();
		void find();
		void search();
		void back();
		void reload();
		void onShutDown();
		void home();
		void restore(const KUrl & url,const QString & text,const QString & sb_text,int engine);
		void onSearchRequested(const QString & text);
	
	private slots:
		void onUrlHover(const QString & url);
		void onFinished();
		void onOpenTorrent(const KUrl & url);
		void onSaveTorrent(const KUrl & url);
		void showPopupMenu(const QString & s,const QPoint & p);
		void onBackAvailable(bool available);
		void onFrameAdded(KParts::Part* p);
		void statusBarMsg(const QString & url);
		void openTorrent(const KUrl & url);
		void loadingProgress(int perc);
		void openNewTab();
		
		
	private:
		HTMLPart* html_part;
		KToolBar* sbar;
		KMenu* right_click_menu;
		SearchPlugin* sp;
		QProgressBar* prog;
		
		KComboBox* search_engine;
		KLineEdit* search_text;
		QAction* open_url_action;
		QAction* copy_url_action;
		KUrl url_to_open;
	};

}

#endif
