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

#include <QLineEdit>

#include <ktoolbar.h>
#include "webview.h"

class QProgressBar;
class QNetworkReply;
class QMenu;
class KComboBox;

namespace kt
{
    class SearchWidget;
    class SearchPlugin;


    /**
        @author Joris Guisson

        Widget which shows a KHTML window with the users search in it
    */
    class SearchWidget : public QWidget, public WebViewClient
    {
        Q_OBJECT
    public:
        SearchWidget(SearchPlugin* sp);
        ~SearchWidget();

        QString getSearchText() const {return search_text->text();}
        QUrl getCurrentUrl() const;
        QString getSearchBarText() const;
        int getSearchBarEngine() const;
        void setSearchBarEngine(int engine);
        bool backAvailable() const;
        void restore(const QUrl &url, const QString& text, const QString& sb_text, int engine);

    signals:
        void enableBack(bool on);
        void openNewTab(const QUrl &url);
        void changeTitle(SearchWidget* w, const QString& title);
        void changeIcon(SearchWidget* w, const QIcon& icon);

    public slots:
        void search(const QString& text, int engine = 0);
        void home();
        void search();

    private slots:
        void loadStarted();
        void loadFinished(bool ok);
        void loadProgress(int p);
        void unsupportedContent(QNetworkReply* reply);
        void torrentDownloadFinished();
        void iconChanged();
        void titleChanged(const QString& text);

    private:
        QUrl searchUrl(const QString& search_text) override;
        QWebView* newTab() override;
        void magnetUrl(const QUrl& magnet_url) override;

    private:
        WebView* webview;
        KToolBar* sbar;
        SearchPlugin* sp;
        QProgressBar* prog;
        QNetworkReply* torrent_download;

        KComboBox* search_engine;
        QLineEdit* search_text;
    };

}

#endif
