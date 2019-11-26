/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson, Ivan Vasic                  *
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

#include "searchwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QLabel>
#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KComboBox>
#include <KIconLoader>
#include <KIO/Job>
#include <KLocalizedString>
#include <KMainWindow>
#include <KMessageBox>
#include <KNotification>
#include <KStandardAction>

#include <util/log.h>
#include <magnet/magnetlink.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include "searchplugin.h"
#include "searchenginelist.h"
#include "webview.h"
#include "searchactivity.h"


using namespace bt;

namespace kt
{

    SearchWidget::SearchWidget(SearchPlugin* sp) : webview(nullptr), sp(sp), prog(nullptr)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        webview = new WebView(this, sp->getProxy());

        KActionCollection* ac = sp->getSearchActivity()->part()->actionCollection();
        sbar = new KToolBar(this);
        sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        sbar->addAction(webview->pageAction(QWebEnginePage::Back));
        sbar->addAction(webview->pageAction(QWebEnginePage::Forward));
        sbar->addAction(webview->pageAction(QWebEnginePage::Reload));
        sbar->addAction(ac->action(QStringLiteral("search_home")));
        search_text = new QLineEdit(sbar);
        sbar->addWidget(search_text);
        sbar->addAction(ac->action(QStringLiteral("search_tab_search")));
        sbar->addWidget(new QLabel(i18n(" Engine: "))); // same i18n string as in SearchToolBar()
        search_engine = new KComboBox(sbar);
        search_engine->setModel(sp->getSearchEngineList());
        sbar->addWidget(search_engine);

        connect(search_text, SIGNAL(returnPressed()), this, SLOT(search()));;

        layout->addWidget(sbar);
        layout->addWidget(webview);

        search_text->setClearButtonEnabled(true);

        connect(webview, &WebView::loadStarted, this, &SearchWidget::loadStarted);
        connect(webview, &WebView::loadFinished, this, &SearchWidget::loadFinished);
        connect(webview, &WebView::loadProgress, this, &SearchWidget::loadProgress);
        connect(webview, &WebView::iconChanged, this, &SearchWidget::iconChanged);
        connect(webview, &WebView::titleChanged, this, &SearchWidget::titleChanged);
        connect(webview, &WebView::torrentFileDownloadRequested, this, &SearchWidget::downloadTorrentFile);
    }


    SearchWidget::~SearchWidget()
    {
        if (prog)
        {
            sp->getGUI()->getStatusBar()->removeProgressBar(prog);
            prog = 0;
        }
    }

    void SearchWidget::iconChanged()
    {
        changeIcon(this, webview->icon());
    }

    void SearchWidget::titleChanged(const QString& text)
    {
        if (!text.isEmpty()) {
            changeTitle(this, text);
        }
        else { // no empty tab titles allowed
            changeTitle(this, webview->url().toString());
        }
    }

    QUrl SearchWidget::getCurrentUrl() const
    {
        return webview->url();
    }

    QString SearchWidget::getSearchBarText() const
    {
        return search_text->text();
    }

    int SearchWidget::getSearchBarEngine() const
    {
        return search_engine->currentIndex();
    }

    void SearchWidget::restore(const QUrl &url, const QString& text, const QString& sb_text, int engine)
    {
        Q_UNUSED(text);

        if (url.scheme() == QLatin1String("home"))
            webview->home();
        else
            webview->openUrl(url);

        search_text->setText(sb_text);

        search_engine->setCurrentIndex(engine);
    }

    void SearchWidget::search(const QString& text, int engine)
    {
        if (search_text->text() != text)
            search_text->setText(text);

        if (search_engine->currentIndex() != engine)
            search_engine->setCurrentIndex(engine);

        QUrl url = sp->getSearchEngineList()->search(engine, text);

        webview->openUrl(url);
    }

    QUrl SearchWidget::searchUrl(const QString& search_text)
    {
        return sp->getSearchEngineList()->search(search_engine->currentIndex(), search_text);
    }

    void SearchWidget::setSearchBarEngine(int engine)
    {
        search_engine->setCurrentIndex(engine);
    }

    void SearchWidget::loadProgress(int perc)
    {
        if (!prog)
            prog = sp->getGUI()->getStatusBar()->createProgressBar();

        if (prog)
            prog->setValue(perc);
    }

    void SearchWidget::loadStarted()
    {
        if (!prog)
        {
            prog = sp->getGUI()->getStatusBar()->createProgressBar();

            if (prog)
                prog->setValue(0);
        }
    }

    void SearchWidget::loadFinished(bool ok)
    {
        Q_UNUSED(ok);

        if (prog)
        {
            sp->getGUI()->getStatusBar()->removeProgressBar(prog);
            prog = 0;
        }
    }

    void SearchWidget::magnetUrl(const QUrl& magnet_url)
    {
        MagnetLinkLoadOptions options;
        options.silently = false;
        sp->getCore()->load(bt::MagnetLink(magnet_url.toString()), options);
        QString msg = i18n("Downloading:<br/><b>%1</b>", magnet_url.toString());
        KNotification::event(QStringLiteral("MagnetLinkDownloadStarted"), msg, QPixmap(), sp->getGUI()->getMainWindow());
    }

    void SearchWidget::downloadTorrentFile(QWebEngineDownloadItem *download)
    {
        int ret = KMessageBox::questionYesNoCancel(0,

                  i18n("Do you want to download or save the torrent?"),
                  i18n("Download Torrent"),
                  KGuiItem(i18n("Download"), QStringLiteral("ktorrent")),
                  KStandardGuiItem::save(),
                  KStandardGuiItem::cancel(),
                  QStringLiteral(":TorrentDownloadFinishedQuestion"));

        if (ret == KMessageBox::Yes){
            sp->getCore()->load(download->url(), QString());
        } else if (ret == KMessageBox::No) {
            webview->downloadFile(download);
        }
    }

    void SearchWidget::search()
    {
        search(search_text->text(), search_engine->currentIndex());
    }

    QWebEngineView* SearchWidget::newTab()
    {
        return sp->getSearchActivity()->newTab()->webview;
    }


    void SearchWidget::home()
    {
        webview->home();
    }

    bool SearchWidget::backAvailable() const
    {
        return webview->pageAction(QWebEnginePage::Back)->isEnabled();
    }
}
