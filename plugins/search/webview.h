/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_WEBVIEW_H
#define KT_WEBVIEW_H

#include <QWebEngineView>

#include <QNetworkReply>
#include <QUrl>

#include "proxy_helper.h"

namespace kt
{
class WebViewClient
{
public:
    virtual ~WebViewClient()
    {
    }

    /// Get a search url for a search text
    virtual QUrl searchUrl(const QString &search_text) = 0;

    /// Create a new tab
    virtual QWebEngineView *newTab() = 0;

    /// Handle magnet urls
    virtual void magnetUrl(const QUrl &magnet_url) = 0;
};

/**
    WebView provides a webkit view which supports for the ktorrent homepage.
 */
class WebView : public QWebEngineView
{
    Q_OBJECT
public:
    WebView(WebViewClient *client, ProxyHelper *proxy, QWidget *parentWidget = nullptr);
    ~WebView() override;

    /**
     * Open a url
     * @param url The QUrl
     */
    void openUrl(const QUrl &url);

    /**
     * Show the home page
     */
    void home();

    /**
     * Get a search url for a search text
     * @param search_text The text to search
     * @return A QUrl to load
     */
    QUrl searchUrl(const QString &search_text);

    /// Get the html code of the homepage
    QString homePageData();

    /// Get the home page base directory
    QString homePageBaseDir() const
    {
        return home_page_base_url;
    }

    /// Handle magnet url
    void handleMagnetUrl(const QUrl &magnet_url);

    /// Get heloper object that applies proxy settings
    ProxyHelper *getProxy() const
    {
        return m_proxy;
    }

    void downloadFile(QWebEngineDownloadItem *download);

protected:
    void loadHomePage();
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;

public Q_SLOTS:
    /**
     * Download a QWebEngineDownloadItem
     * @param download The QWebEngineDownloadItem
     */
    void downloadRequested(QWebEngineDownloadItem *download);
    void magnetUrlDetected(const QUrl &url);

Q_SIGNALS:
    void torrentFileDownloadRequested(QWebEngineDownloadItem *download);

private:
    QString home_page_html;
    QString home_page_base_url;
    WebViewClient *client;
    QUrl clicked_url;
    QUrl image_url;
    ProxyHelper *m_proxy;
};

}

#endif // KT_HOMEPAGE_H
