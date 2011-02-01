/*  This software is a thinlayer between KDE's IO-infrastructure and KTorrent.
    Copyright (C) 2010  Christian Weilbach < christian_weilbach 4T web D0T de >

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KIO_MAGNET_H
#define KIO_MAGNET_H

#include <kio/global.h>
#include <kio/slavebase.h>

#include <QtCore/QMutex>

class KUrl;
class DBusHandler;
class QStringList;

class MagnetProtocol : public KIO::SlaveBase
{
public:
    MagnetProtocol( const QByteArray &pool, const QByteArray &app );
    virtual ~MagnetProtocol();

    virtual void stat( const KUrl & url );
    virtual void listDir( const KUrl & url );
    virtual void mimetype( const KUrl & url );
    virtual void get( const KUrl & url );
    virtual void open( const KUrl &url, QIODevice::OpenMode mode );
    virtual void read( KIO::filesize_t );
    virtual void seek( KIO::filesize_t );
    virtual void setSize( qint64 );
    virtual void setNumFiles( int );
    virtual void setPath( const QString& );
    virtual void setFiles( const QStringList& );
    virtual void added( const QString& );
    virtual void downloaded(bool);

private:
    virtual void load( const KUrl& url );
    virtual bool isDir( const KUrl& url );
    QMutex m_loadMutex;
    KUrl m_url;
    DBusHandler* m_dbusHandler;
    bool m_downloaded;
    QString m_path, m_filename;
    QStringList m_files;
    qint64 m_size, m_position, m_numFiles;
};

#endif // KIO_MAGNET_H
