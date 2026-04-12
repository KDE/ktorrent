/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BTSEARCHWIDGET_H
#define BTSEARCHWIDGET_H

#include <QPoint>
#include <QPointer>

#include <KComboBox>
#include <KToolBar>
#include <QLineEdit>

#include "torznabsearchjob.h"
#include "webview.h"

class QAction;
class QLabel;
class QProgressBar;
class QStackedWidget;
class QTreeWidget;
class QTreeWidgetItem;

namespace kt
{
class SearchPlugin;
class SearchEngine;

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
        return current_search_text;
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
    void clearHistory();

private Q_SLOTS:
    void currentEngineChanged(int index);
    void loadStarted();
    void loadFinished(bool ok);
    void loadProgress(int p);
    void iconChanged();
    void titleChanged(const QString &text);
    void downloadTorrentFile(QWebEngineDownloadRequest *download);
    void torznabResultActivated(QTreeWidgetItem *item, int column);
    void showTorznabResultMenu(const QPoint &pos);
    void downloadSelectedTorznabResult();
    void openSelectedTorznabDescription();
    void torznabResultFound(const kt::TorznabSearchResult &result);
    void torznabSearchFinished(int totalIndexers, int failedIndexers);
    void torznabSearchFailed(const QString &message);
    void torznabSearchProgress(int completed, int total);

private:
    enum class ContentMode {
        Web,
        Torznab,
    };

    QUrl searchUrl(const QString &search_text) override;
    QWebEngineView *newTab() override;
    void magnetUrl(const QUrl &magnet_url) override;
    void loadSearchHistory();
    void saveSearchHistory();
    void searchWeb(const QString &text, int engine);
    void searchTorznab(const QString &text, const SearchEngine &engine);
    SearchEngine *selectedEngine() const;
    void showTorznabHome();
    void clearTorznabResults();
    void switchContentMode(ContentMode mode);
    void ensureProgressBar(int maximum = 100);
    void removeProgressBar();

private:
    QStackedWidget *content_stack;
    QWidget *torznab_page;
    WebView *webview;
    KToolBar *sbar;
    SearchPlugin *sp;
    QProgressBar *prog;
    QLabel *torznab_summary;
    QTreeWidget *torznab_results;
    QAction *back_action;
    QAction *forward_action;
    QAction *reload_action;
    QPointer<TorznabSearchJob> torznab_job;

    KComboBox *search_engine;
    KComboBox *search_text;
    ContentMode content_mode = ContentMode::Web;
    QString current_search_text;
    QUrl current_url;
    int torznab_result_count = 0;
    bool torznab_tracker_first = false;
    QString torznab_engine_name;
};

}

#endif
