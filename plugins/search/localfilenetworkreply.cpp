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

#include "localfilenetworkreply.h"
#include <QTimer>
#include <util/log.h>

using namespace bt;

namespace kt
{
    LocalFileNetworkReply::LocalFileNetworkReply(const QString& file, QObject* parent): QNetworkReply(parent), fptr(0)
    {
        fptr = new QFile(file, this);
        if (fptr->open(QIODevice::ReadOnly))
        {
            open(ReadOnly | Unbuffered);
            setHeader(QNetworkRequest::ContentLengthHeader, QVariant(fptr->size()));
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
            setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QStringLiteral("OK"));

            QTimer::singleShot(0, this, SIGNAL(readyRead()));
            QTimer::singleShot(0, this, SIGNAL(finished()));
        }
        else
        {
            Out(SYS_SRC | LOG_IMPORTANT) << "Cannot open " << file << ": " << fptr->errorString() << endl;
            delete fptr;
            fptr = 0;
            setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 500);
            setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, QStringLiteral("Internal server error"));
            QTimer::singleShot(0, this, SIGNAL(finished()));
        }
    }

    LocalFileNetworkReply::~LocalFileNetworkReply()
    {
    }

    void LocalFileNetworkReply::abort()
    {
        delete fptr;
        fptr = 0;
    }

    qint64 LocalFileNetworkReply::readData(char* data, qint64 maxlen)
    {
        return fptr ? fptr->read(data, maxlen) : 0;
    }

    bool LocalFileNetworkReply::atEnd() const
    {
        return !fptr || fptr->atEnd();
    }

    qint64 LocalFileNetworkReply::bytesAvailable() const
    {
        return fptr ? fptr->size() : 0;
    }
}


