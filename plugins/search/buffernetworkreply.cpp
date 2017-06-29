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

#include "buffernetworkreply.h"
#include <QByteArray>
#include <QTimer>

namespace kt
{
    BufferNetworkReply::BufferNetworkReply(const QByteArray& data, const QString& content_type, QObject* parent)
        : QNetworkReply(parent)
    {
        buf.open(ReadWrite);
        buf.write(data);
        buf.seek(0);

        open(ReadOnly | Unbuffered);
        setHeader(QNetworkRequest::ContentTypeHeader, content_type);
        setHeader(QNetworkRequest::ContentLengthHeader, data.size());
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
        setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QStringLiteral("OK"));

        QTimer::singleShot(0, this, SIGNAL(readyRead()));
        QTimer::singleShot(0, this, SIGNAL(finished()));
    }

    BufferNetworkReply::~BufferNetworkReply()
    {

    }

    void BufferNetworkReply::abort()
    {
        // Do nothing
    }

    qint64 BufferNetworkReply::readData(char* data, qint64 maxlen)
    {
        return buf.read(data, maxlen);
    }

    qint64 BufferNetworkReply::bytesAvailable() const
    {
        return buf.size();
    }

}

