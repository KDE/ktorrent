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

#ifndef KTSEARCHPREFPAGE_H
#define KTSEARCHPREFPAGE_H

#include <QString>

#include <interfaces/prefpageinterface.h>
#include "ui_searchpref.h"
class KJob;

namespace kt
{
    class SearchPlugin;
    class SearchEngineList;

    /**
     * @author Joris Guisson
     *
     * Preference page for the search plugin.
    */
    class SearchPrefPage : public PrefPageInterface, public Ui_SearchPref
    {
        Q_OBJECT
    public:
        SearchPrefPage(SearchPlugin* plugin, SearchEngineList* sl, QWidget* parent);
        ~SearchPrefPage();

        void loadSettings() override;
        void loadDefaults() override;

    public slots:
        void customToggled(bool toggled);

    private slots:
        void addClicked();
        void removeClicked();
        void addDefaultClicked();
        void removeAllClicked();
        void clearHistory();
        void openInExternalToggled(bool on);
        void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void downloadJobFinished(KJob* j);
        void resetDefaultAction();

    signals:
        void clearSearchHistory();

    private:
        SearchPlugin* plugin;
        SearchEngineList* engines;
    };

}

#endif
