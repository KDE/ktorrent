/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BTSEARCHWIDGET_H
#define BTSEARCHWIDGET_H

#include <QLineEdit>

#include "webview.h"
#include <ktoolbar.h>

class QProgressBar;
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
    SearchWidget(SearchPlugin *sp);
    ~SearchWidget() override;

    QString getSearchText() const
    {
        return search_text->text();
    }
    QUrl getCurrentUrl() const;
    QString getSearchBarText() const;
    int getSearchBarEngine() const;
    void setSearchBarEngine(int engine);
    bool backAvailable() const;
    void restore(const QUrl &url, const QString &text, const QString &sb_text, int engine);

Q_SIGNALS:
    void enableBack(bool on);
    void openNewTab(const QUrl &url);
    void changeTitle(SearchWidget *w, const QString &title);
    void changeIcon(SearchWidget *w, const QIcon &icon);

public Q_SLOTS:
    void search(const QString &text, int engine = 0);
    void home();
    void search();

private Q_SLOTS:
    void loadStarted();
    void loadFinished(bool ok);
    void loadProgress(int p);
    void iconChanged();
    void titleChanged(const QString &text);
    void downloadTorrentFile(QWebEngineDownloadItem *download);

private:
    QUrl searchUrl(const QString &search_text) override;
    QWebEngineView *newTab() override;
    void magnetUrl(const QUrl &magnet_url) override;

private:
    WebView *webview;
    KToolBar *sbar;
    SearchPlugin *sp;
    QProgressBar *prog;

    KComboBox *search_engine;
    QLineEdit *search_text;
};

}

#endif
