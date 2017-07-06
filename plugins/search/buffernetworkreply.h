/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#ifndef KT_BUFFERNETWORKREPLY_H
#define KT_BUFFERNETWORKREPLY_H

#include <QBuffer>
#include <QNetworkReply>


namespace kt
{
    /**
     * QNetworkReply which reads from a buffer
     */
    class BufferNetworkReply : public QNetworkReply
    {
    public:
        /**
         * @param data The data to put into the buffer
         * @param content_type Content type of the data
         * @param parent Parent of the BufferNetworkReply
         */
        BufferNetworkReply(const QByteArray& data, const QString& content_type, QObject* parent = 0);
        ~BufferNetworkReply();

        void abort() override;
        bool isSequential() const override {return true;}
        qint64 bytesAvailable() const override;

    protected:
        qint64 readData(char* data, qint64 maxlen) override;

    private:
        QBuffer buf;
    };

}

#endif // KT_BUFFERNETWORKREPLY_H
