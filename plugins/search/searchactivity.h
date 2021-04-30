/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHACTIVITY_H
#define SEARCHACTIVITY_H

#include <QAction>
#include <QList>
#include <QTabWidget>
#include <QUrl>

#include <interfaces/activity.h>

namespace kt
{
class SearchToolBar;
class SearchWidget;
class SearchPlugin;

class SearchActivity : public kt::Activity
{
    Q_OBJECT
public:
    SearchActivity(SearchPlugin *sp, QWidget *parent);
    ~SearchActivity() override;

    /// Add a SearchWidget
    void search(const QString &text, int engine);

    /// Save all current searches
    void saveCurrentSearches();

    /// Load current searches
    void loadCurrentSearches();

    void loadState(KSharedConfigPtr cfg);
    void saveState(KSharedConfigPtr cfg);

    /// Create a new empty search tab
    SearchWidget *newTab();

public Q_SLOTS:
    void home();
    void openNewTab(const QUrl &url);
    void currentTabChanged(int idx);
    void closeTab();
    void openTab();
    void setTabTitle(SearchWidget *sw, const QString &title);
    void setTabIcon(SearchWidget *sw, const QIcon &icon);
    void clearSearchHistory();
    void search();
    void find();

private:
    SearchWidget *newSearchWidget(const QString &text);
    void setupActions();

private:
    QTabWidget *tabs;
    QList<SearchWidget *> searches;
    SearchPlugin *sp;
    SearchToolBar *toolbar;

    QAction *find_action;
    QAction *search_action;
    QAction *home_action;
};
}

#endif // SEARCHACTIVITY_H
