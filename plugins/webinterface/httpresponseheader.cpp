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

#include "httpresponseheader.h"

namespace kt
{
    static QString ResponseCodeToString(int r)
    {
        switch (r)
        {
        case 200: return "OK";
        case 301: return "Moved Permanently";
        case 304: return "Not Modified";
        case 404: return "Not Found";
        }
        return QString::null;
    }

    HttpResponseHeader::HttpResponseHeader(int response_code, int major_version, int minor_version)
        : response_code(response_code), major_version(major_version), minor_version(minor_version)
    {
    }

    HttpResponseHeader::HttpResponseHeader(const HttpResponseHeader& hdr)
    {
        response_code = hdr.response_code;
        fields = hdr.fields;
        major_version = hdr.major_version;
        minor_version = hdr.minor_version;
    }

    HttpResponseHeader::~HttpResponseHeader()
    {
    }

    void HttpResponseHeader::setResponseCode(int rc)
    {
        response_code = rc;
    }

    void HttpResponseHeader::setValue(const QString& key, const QString& value)
    {
        fields[key] = value;
    }

    QString HttpResponseHeader::toString() const
    {
        QString str;
        str += QString("HTTP/%1.%2 %3 %4\r\n").arg(major_version).arg(minor_version).arg(response_code).arg(ResponseCodeToString(response_code));

        QMap<QString, QString>::const_iterator itr = fields.begin();
        while (itr != fields.end())
        {
            str += QString("%1: %2\r\n").arg(itr.key()).arg(itr.value());
            itr++;
        }
        str += "\r\n";
        return str;
    }



}
