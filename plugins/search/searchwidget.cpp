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

#include <QLabel>
#include <QClipboard>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMenu>

#include <KIconLoader>
#include <KComboBox>
#include <klocalizedstring.h>
#include <knotification.h>
#include <KStandardAction>
#include <kio/job.h>
#include <KMessageBox>
#include <KActionCollection>
#include <KMainWindow>

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

    SearchWidget::SearchWidget(SearchPlugin* sp) : webview(0), sp(sp), prog(0), torrent_download(0)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        webview = new WebView(this);

        KActionCollection* ac = sp->getSearchActivity()->part()->actionCollection();
        sbar = new KToolBar(this);
        sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
        sbar->addAction(webview->pageAction(QWebPage::Back));
        sbar->addAction(webview->pageAction(QWebPage::Forward));
        sbar->addAction(webview->pageAction(QWebPage::Reload));
        sbar->addAction(ac->action(QStringLiteral("search_home")));
        search_text = new QLineEdit(sbar);
        sbar->addWidget(search_text);
        sbar->addAction(ac->action(QStringLiteral("search_tab_search")));
        sbar->addWidget(new QLabel(i18n(" Engine:")));
        search_engine = new KComboBox(sbar);
        search_engine->setModel(sp->getSearchEngineList());
        sbar->addWidget(search_engine);

        connect(search_text, SIGNAL(returnPressed()), this, SLOT(search()));;

        layout->addWidget(sbar);
        layout->addWidget(webview);

        search_text->setClearButtonEnabled(true);

        connect(webview, SIGNAL(loadStarted()), this, SLOT(loadStarted()));
        connect(webview, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished(bool)));
        connect(webview, SIGNAL(loadProgress(int)), this, SLOT(loadProgress(int)));
        connect(webview->page(), SIGNAL(unsupportedContent(QNetworkReply*)),
                this, SLOT(unsupportedContent(QNetworkReply*)));
        connect(webview, SIGNAL(linkMiddleOrCtrlClicked(QUrl)), this, SIGNAL(openNewTab(QUrl)));
        connect(webview, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
        connect(webview, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged(QString)));
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
        changeTitle(this, text);
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
        KNotification::event("MagnetLinkDownloadStarted", msg, QPixmap(), sp->getGUI()->getMainWindow());
    }

    void SearchWidget::unsupportedContent(QNetworkReply* r)
    {
        if (r->url().scheme() == QLatin1String("magnet"))
        {
            magnetUrl(r->url());
        }
        else if (r->header(QNetworkRequest::ContentTypeHeader).toString() == QLatin1String("application/x-bittorrent") ||
                 r->url().path().endsWith(QLatin1String(".torrent")))
        {
            torrent_download = r;

            if (!r->isFinished())
                connect(r, SIGNAL(finished()), this, SLOT(torrentDownloadFinished()));
            else
                torrentDownloadFinished();
        }
        else
        {
            webview->downloadResponse(r);
        }
    }

    void SearchWidget::torrentDownloadFinished()
    {
        if (!torrent_download)
            return;

        if (torrent_download->error() != QNetworkReply::NoError)
        {
            KMessageBox::error(this, torrent_download->errorString());
            torrent_download = 0;
            return;
        }

        int ret = KMessageBox::questionYesNoCancel(0,

                  i18n("Do you want to download or save the torrent?"),
                  i18n("Download Torrent"),
                  KGuiItem(i18n("Download"), "ktorrent"),
                  KStandardGuiItem::save(),
                  KStandardGuiItem::cancel(),
                  ":TorrentDownloadFinishedQuestion");

        if (ret == KMessageBox::Yes)
            sp->getCore()->load(torrent_download->readAll(), torrent_download->url(), QString(), QString());
        else if (ret == KMessageBox::No)
            webview->downloadResponse(torrent_download);

        torrent_download = 0;
    }

    void SearchWidget::search()
    {
        search(search_text->text(), search_engine->currentIndex());
    }

    QWebView* SearchWidget::newTab()
    {
        return sp->getSearchActivity()->newTab()->webview;
    }


    void SearchWidget::home()
    {
        webview->home();
    }

    bool SearchWidget::backAvailable() const
    {
        return webview->pageAction(QWebPage::Back)->isEnabled();
    }
}

