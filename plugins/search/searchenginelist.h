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

#ifndef KTSEARCHENGINELIST_H
#define KTSEARCHENGINELIST_H

#include <QAbstractListModel>
#include <QList>
#include <util/constants.h>
#include "searchengine.h"
#include "proxy_helper.h"

namespace kt
{
    class OpenSearchDownloadJob;

    /**
        @author Joris Guisson <joris.guisson@gmail.com>
    */
    class SearchEngineList : public QAbstractListModel
    {
        Q_OBJECT

        QList<SearchEngine*> engines;
        QList<QUrl> default_opensearch_urls;
        QList<QUrl> default_urls;
        ProxyHelper* m_proxy;
        QString data_dir;
    public:
        SearchEngineList(ProxyHelper* proxy, const QString& data_dir);
        ~SearchEngineList();

        /// Load all engines
        void loadEngines();

        /// Search with an engine
        QUrl search(bt::Uint32 engine, const QString& terms);

        /// Get the name of an engine
        QString getEngineName(bt::Uint32 engine) const;

        /// Get the number of engines
        bt::Uint32 getNumEngines() const {return engines.count();}

        int rowCount(const QModelIndex& parent) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        bool insertRows(int row, int count, const QModelIndex& parent) override;
        bool removeRows(int row, int count, const QModelIndex& parent) override;

        /**
         * Remove all engines in a list
         * @param sel The list
         */
        void removeEngines(const QModelIndexList& sel);

        /**
         * Remove all engines
         */
        void removeAllEngines();

        /**
         * Add all defaults engines (if they are not added yet)
         */
        void addDefaults();

        /**
         * Add an engine from an OpenSearchDownloadJob
         * @param j The OpenSearchDownloadJob
         */
        void addEngine(OpenSearchDownloadJob* j);


        /**
         * Add an engine from a search URL
         * @param dir The directory to use
         * @param url The url
         */
        void addEngine(const QString& dir, const QString& url);

    private:
        void convertSearchEnginesFile();
        void loadDefault(bool removed_to);
        bool alreadyLoaded(const QString& user_dir);
        void loadEngine(const QString& global_dir, const QString& user_dir, bool load_removed);

    private slots:
        void openSearchDownloadJobFinished(KJob* j);
    };

}

#endif
