/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <QObject>

class QAction;
class KComboBox;
class QComboBox;
class KActionCollection;

namespace kt
{
class SearchEngineList;

/**
    Holds all widgets of the toolbar of the search plugin.
*/
class SearchToolBar : public QObject
{
    Q_OBJECT

public:
    SearchToolBar(KActionCollection *ac, SearchEngineList *sl, QObject *parent);
    ~SearchToolBar() override;

    /// Save settings like current search engine
    void saveSettings();

    /// Get the index of the current search engine
    int currentSearchEngine() const;

public Q_SLOTS:
    /// Clear the search history
    void clearHistory();

protected Q_SLOTS:
    void searchNewTabPressed();
    void searchBoxReturn();
    void textChanged(const QString &str);
    void selectedEngineChanged(int idx);

Q_SIGNALS:
    /// Emitted when the user presses enter or clicks search
    void search(const QString &text, int engine, bool external);

private:
    void loadSearchHistory();
    void saveSearchHistory();

private:
    KComboBox *m_search_text;
    QComboBox *m_search_engine;
    QAction *m_search_new_tab;
    int m_current_search_engine;
};
}

#endif
