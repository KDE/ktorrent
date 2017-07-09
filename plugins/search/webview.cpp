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

#include "webview.h"

#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <QWebHistoryInterface>
#include <QWebHitTestResult>

#include <KIconLoader>
#include <KIO/AccessManager>
#include <KIO/CopyJob>
#include <KIO/Job>
#include <KLocalizedString>
#include <KMainWindow>
#include <KWebPage>

#include <util/log.h>
#include "buffernetworkreply.h"
#include "localfilenetworkreply.h"

using namespace bt;

namespace kt
{

    class NetworkAccessManager : public KIO::AccessManager
    {
    public:
        NetworkAccessManager(WebView* parent) : KIO::AccessManager(parent), webview(parent)
        {
            webview->getProxy()->ApplyProxy(sessionMetaData());
        }

        ~NetworkAccessManager()
        {}

        QNetworkReply* createRequest(Operation op, const QNetworkRequest& req, QIODevice* outgoingData) override
        {
            if (req.url().scheme() == QStringLiteral("magnet"))
            {
                webview->handleMagnetUrl(req.url());
                return KIO::AccessManager::createRequest(op, req, outgoingData);
            }
            else if (req.url().host() == QStringLiteral("ktorrent.searchplugin"))
            {
                QString search_text = QUrlQuery(req.url()).queryItemValue(QStringLiteral("search_text"));

                if (!search_text.isEmpty())
                {
                    QUrl url(webview->searchUrl(search_text));
                    QNetworkRequest request(url);
                    return KIO::AccessManager::createRequest(op, request, outgoingData);
                }
                else if (req.url().path() == QStringLiteral("/"))
                {
                    return new BufferNetworkReply(webview->homePageData().toLocal8Bit(), QStringLiteral("text/html"), this);
                }
                else
                {
                    return new LocalFileNetworkReply(webview->homePageBaseDir() + req.url().path(), this);
                }
            }

            return KIO::AccessManager::createRequest(op, req, outgoingData);
        }

        WebView* webview;
    };


    //////////////////////////////////////////////////////

    WebView::WebView(kt::WebViewClient* client, ProxyHelper* proxy, QWidget* parentWidget)
        : KWebView(parentWidget), client(client), m_proxy(proxy)
    {
        page()->setNetworkAccessManager(new NetworkAccessManager(this));
        page()->setForwardUnsupportedContent(true);

        connect(page(), SIGNAL(downloadRequested(QNetworkRequest)), this, SLOT(downloadRequested(QNetworkRequest)));
    }

    WebView::~WebView()
    {
    }

    void WebView::handleMagnetUrl(const QUrl& magnet_url)
    {
        if (client)
            client->magnetUrl(magnet_url);
    }

    void WebView::openUrl(const QUrl &url)
    {
        if (url.host() == QStringLiteral("ktorrent.searchplugin"))
            home();
        else
            load(url);
    }

    void WebView::home()
    {
        if (home_page_html.isEmpty())
            loadHomePage();

        load(QUrl(QStringLiteral("http://ktorrent.searchplugin/")));
    }

    QString WebView::homePageData()
    {
        if (home_page_html.isEmpty())
            loadHomePage();

        return home_page_html;
    }


    void WebView::loadHomePage()
    {
        QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("ktorrent/search/home/home.html"));
        QFile fptr(file);

        if (fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_SRC | LOG_DEBUG) << "Loading home page from " << file << endl;
            home_page_base_url = file.left(file.lastIndexOf(QLatin1Char('/')) + 1);
            home_page_html = QTextStream(&fptr).readAll();

            // %1
            home_page_html = home_page_html.arg(QStringLiteral("ktorrent_infopage.css"));
            // %2

            if (qApp->layoutDirection() == Qt::RightToLeft)
            {
                QString link = QStringLiteral("<link rel=\"stylesheet\" type=\"text/css\" href=\"%1\" />");
                link = link.arg(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("kdeui/about/kde_infopage_rtl.css")));
                home_page_html = home_page_html.arg(link);
            }
            else
                home_page_html = home_page_html.arg(QString());

            KIconLoader* iconloader = KIconLoader::global();

            int icon_size = iconloader->currentSize(KIconLoader::Desktop);

            home_page_html = home_page_html
                             .arg(i18n("Home")) // %3 Title
                             .arg(i18n("KTorrent")) // %4
                             .arg(i18nc("KDE 4 tag line, see http://kde.org/img/kde40.png", "Be free.")) // %5
                             .arg(i18n("Search the web for torrents.")) // %6
                             .arg(i18n("Search")) // %7
                             .arg(QStringLiteral("search_text")) // %8
                             .arg(icon_size).arg(icon_size); // %9 and %10
        }
        else
        {
            Out(SYS_SRC | LOG_IMPORTANT) << "Failed to load " << file << " : " << fptr.errorString() << endl;
        }
    }

    QUrl WebView::searchUrl(const QString& search_text)
    {
        if (client)
            return client->searchUrl(search_text);
        else
            // client is broken -> browse to home
            return QUrl(QStringLiteral("http://ktorrent.searchplugin/"));
    }

    QWebView* WebView::createWindow(QWebPage::WebWindowType type)
    {
        Q_UNUSED(type);
        return client->newTab();
    }

    void WebView::downloadRequested(const QNetworkRequest& req)
    {
        QString filename = QFileInfo(req.url().path()).fileName();
        QString path = QFileDialog::getExistingDirectory(this, i18n("Save %1 to"), QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));

        if (!path.isEmpty())
            KIO::copy(req.url(), QUrl(path));
    }

    void WebView::downloadResponse(QNetworkReply* reply)
    {
        KWebPage* p = (KWebPage*)page();
        p->downloadResponse(reply);
    }

}

