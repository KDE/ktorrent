/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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

#ifndef KTSEARCHENGINE_H
#define KTSEARCHENGINE_H

#include <QIcon>
#include <QObject>
#include <QUrl>

class KJob;

namespace kt
{

    /**
        Keeps track of a search engine
    */
    class SearchEngine : public QObject
    {
        Q_OBJECT
    public:
        /**
         * Constructor, sets the data dir
         * @param data_dir Directory where all the information regarding the engine is stored
         */
        SearchEngine(const QString& data_dir);
        ~SearchEngine();

        /**
         * Load the engine from an opensearch XML file
         * @param xml_file Local XML file
         * @return true upon success
         */
        bool load(const QString& xml_file);

        /**
         * Fill in search terms into the search url and create the QUrl to use
         * @param terms Tersm to search for
         * @return The url
         */
        QUrl search(const QString& terms);

        /// Get the name of the engine
        QString engineName() const {return name;}

        /// Get the icon
        QIcon engineIcon() const {return icon;}

        /// Get the engine directory
        QString engineDir() const {return data_dir;}

        /// Get the URL
        QString engineUrl() const {return url;}

        /// Get the description
        QString engineDescription() const {return description;}

    private slots:
        void iconDownloadFinished(KJob* job);

    private:
        QString data_dir;
        QString name;
        QString description;
        QString url;
        QString icon_url;
        QIcon icon;

        friend class OpenSearchHandler;
    };

}

#endif
