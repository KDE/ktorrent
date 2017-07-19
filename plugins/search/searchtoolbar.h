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

#ifndef SEARCHTAB_H
#define SEARCHTAB_H

#include <QObject>

class QAction;
class KComboBox;
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
        SearchToolBar(KActionCollection* ac, SearchEngineList* sl, QObject* parent);
        ~SearchToolBar();

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
        void textChanged(const QString& str);
        void selectedEngineChanged(int idx);

    Q_SIGNALS:
        /// Emitted when the user presses enter or clicks search
        void search(const QString& text, int engine, bool external);

    private:
        void loadSearchHistory();
        void saveSearchHistory();

    private:
        KComboBox* m_search_text;
        KComboBox* m_search_engine;
        QAction* m_search_new_tab;
        int m_current_search_engine;
    };
}

#endif

