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

#ifndef KTFEEDRETRIEVER_H
#define KTFEEDRETRIEVER_H

#include <QFile>
#include <Syndication/DataRetriever>

class KJob;

namespace kt
{

    /**
        Class which downloads a feed and also saves a backup copy.
    */
    class FeedRetriever : public Syndication::DataRetriever
    {
    public:
        /// Constructor, does not save a backup copy
        FeedRetriever();

        /// Constructor, does save a backup copy
        FeedRetriever(const QString& file_name);

        ~FeedRetriever();

        /// Set the authentication cookie
        void setAuthenticationCookie(const QString& cookie);

        void abort() override;
        int errorCode() const override;
        void retrieveData(const QUrl& url) override;

        void finished(KJob* j);

    private:
        QString backup_file;
        KJob* job;
        int err;
        QString cookie;
    };

}

#endif
