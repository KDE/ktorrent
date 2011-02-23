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

#include "kio_magnet.h"
#include "dbushandler.h"

#include <stdlib.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <KUrl>
#include <KMimeType>
#include <kmessagebox.h>
#include <kdebug.h>

const long max_ipc_size = 1024*4; // was 1024*32

using namespace KIO;

extern "C" {
    int KDE_EXPORT kdemain(int argc, char **argv);
}

int kdemain( int argc, char **argv )
{
    kDebug() << "Starting" << getpid();
    KComponentData componentData("kio_magnet");
    QCoreApplication app( argc, argv );

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_magnet protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    MagnetProtocol slave(argv[2], argv[3]);
    slave.dispatchLoop();

    kDebug() << "Done";
    return ( 0 );
}

MagnetProtocol::MagnetProtocol( const QByteArray &pool, const QByteArray &app )
        : SlaveBase( "magnet", pool, app )
        , m_downloaded(false)
        , m_size(-1)
        , m_position(0)
        , m_numFiles(0)
{
    kDebug();
    m_dbusHandler = new DBusHandler(this);
}

MagnetProtocol::~MagnetProtocol()
{
    kDebug();
    delete m_dbusHandler;
}

void MagnetProtocol::load(const KUrl& url)
{
    m_loadMutex.lock();
    kDebug();

    m_url = url;
    m_downloaded = false;
    m_size = -1;
    m_position = 0;
    m_numFiles = 0;
    m_files.clear();
    m_path.clear();

    if( !m_dbusHandler->load( url ) ) {
        m_loadWaiter.wait( &m_loadMutex ); 
    }
    m_loadMutex.unlock();
}

/**
 * We do this completely on the url only, since
 * the files get only created when they are downloaded.
 * Therefore we cannot check it on m_path, since it is empty
 * by default. When we actually load the files in get or open
 * we can operate on them locally with QFileInfo for example.
 * TODO export a proper file tree from KTorrent
 */
bool MagnetProtocol::isDir(const KUrl& url)
{
    kDebug();
    QString pt = url.queryItem("pt");
    // TODO is the tailing "/" ok for a standard?
    if ( !pt.isEmpty() && pt.endsWith("/") )
        return true;
    if ( url.path().endsWith("/") ) {
        return true;
    }
    if ( url.hasHost() && !url.hasPath() )
        return true;
    return false;
}


void MagnetProtocol::stat( const KUrl& url )
{
    kDebug() << url.url();
    load(url);

    UDSEntry entry;
    entry.insert( KIO::UDSEntry::UDS_TARGET_URL, url.url() );
    entry.insert( KIO::UDSEntry::UDS_NAME, m_filename );

    if ( isDir(url) ) {
        entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
    } else {
        entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );
    }
    // readable by everybody
    entry.insert( KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH );

    statEntry(entry);
    finished();
}

void MagnetProtocol::listDir(const KUrl& u)
{
    kDebug() << u.url();
    load(u);

    totalSize( m_numFiles );

    UDSEntryList eList;
    QStringList subdirList;
    QString dirPath = u.path().remove(QRegExp("^/"));
    if ( dirPath.isEmpty() )
        dirPath = u.queryItem("pt").remove(QRegExp("^/"));
    if ( !dirPath.isEmpty() && !dirPath.endsWith("/") )
        dirPath += "/";

    for ( int i=0; i<m_files.count(); i++) {
        QString file = m_files[i];
        // skip files in other subpaths
        if (!dirPath.isEmpty() && !file.contains(dirPath)) {
            continue;
        }

        UDSEntry entry;
        // HACK ish for files in subdirectories and subdirectory parsing
        //      hope this is stable enough for now.
        QString ts;
        if (file.remove(dirPath).contains(QRegExp("/[^$]+"))) {
            QString subdir = file.remove(QRegExp("^"+QRegExp::escape(dirPath)));
            subdir = subdir.remove(QRegExp("/[^$]+"));
            if ( subdirList.contains(subdir) ) {
                continue;
            } else {
                file = subdir;
                ts = "/";
                entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
                subdirList << subdir;
            }
        } else {
            entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFREG );
            entry.insert( KIO::UDSEntry::UDS_SIZE, m_dbusHandler->fileSize(i) );
        }
        KUrl url = u;

        QString path = dirPath;
        path += file+ts;

        if (url.hasHost()) {
            url.setPath(path);
        } else {
            url.removeAllQueryItems("pt");
            url.addQueryItem("pt",path);
        }
        if (!url.hasQueryItem("sp") &&
                file.contains(QRegExp("\\.(avi|mp2|mp3|mp4|m4v|webm|ogv|ogg|mpeg|mpg|wmv)$"))) {
            //HACK enable streaming for seemless playback on known containers for now
            url.addQueryItem("sp","200");
        }

        entry.insert( KIO::UDSEntry::UDS_NAME, file );
        // TODO KUrl doesn't like to have URNs without a "/" after the protocol as it seems
        entry.insert( KIO::UDSEntry::UDS_TARGET_URL, url.url().replace( "magnet:/?", "magnet:?" ) );
        // readable by everybody
        entry.insert( KIO::UDSEntry::UDS_ACCESS, S_IRUSR | S_IRGRP | S_IROTH );
        eList << entry;
    }
    listEntries(eList);
    finished();
}

void MagnetProtocol::mimetype(const KUrl& url)
{
    kDebug();
    load(url);

    KMimeType::Ptr mt = KMimeType::findByUrl( m_filename, 0, false /* local URL */ );
    emit mimeType( mt->name() );
    finished();
}

void MagnetProtocol::get( const KUrl& url )
{
    kDebug() << url.url() << "path: " << m_path;
    load(url);

    totalSize( m_size );

    QFileInfo info(m_path);
    QFile file(m_path);
    if (info.isFile()) {
        if (!file.open(QIODevice::ReadOnly)) {
            error(KIO::ERR_ABORTED,
                  i18n("File exists in KTorrent, but cannot open it on disk at path \"%1\"."
                       " Have you removed the file manually?" ).arg(m_path));
            return;
        }
    } else if (info.isDir()) {
        emit mimeType( "inode/directory" );
        finished();
        return;
    }

    bool emitMimetype=true;
    qint64 ps = 0;
    // !file.end does not work, since the file can be empty when preallocation
    // in ktorrent is disabled
    while ( ps != m_size ) {
        if (m_downloaded || m_dbusHandler->seek(ps+max_ipc_size)) {
            if ( emitMimetype ) {
                KMimeType::Ptr mt = KMimeType::findByUrl( m_path, 0, true /* local URL */ );
                kDebug() << "mimetype: " << mt->name();
                emit mimeType( mt->name() );
                emitMimetype=false;
            }

            qint64 len = 0;
            if ( (m_size - ps) > max_ipc_size )
                len = max_ipc_size;
            else
                len = m_size - ps;
            ps = ps + len;
            QByteArray d = file.read(len);
            data(d);
            kDebug() << "processed size: " << ps;
            processedSize(ps);
        } else { // TODO check if we can optimize that
            usleep(10000);
        }
    }
    kDebug() << "reading ended.";
    file.close();
    finished();
}

void MagnetProtocol::added( const QString& fn )
{
    kDebug();

    m_filename = fn;
    m_loadWaiter.wakeOne();
}

void MagnetProtocol::downloaded(bool dl)
{
    kDebug();
    m_downloaded = dl;
}

void MagnetProtocol::open(const KUrl& url, QIODevice::OpenMode mode)
{
    kDebug() << url.url() << "path: " << m_path;
    if ( mode != QIODevice::ReadOnly ) {
        error(KIO::ERR_CANNOT_OPEN_FOR_WRITING, i18n("You cannot write to magnet resources."));
        return;
    }

    load(url);

    if (m_size!=-1) {
        totalSize( m_size );
    }

    QFileInfo info(m_path);
    QFile file(m_path);
    if (info.isFile()) {
        if (!file.open(QIODevice::ReadOnly)) {
            error(KIO::ERR_ABORTED,
                  i18n("File exists in KTorrent, but cannot open it on disk at path \"%1\"."
                       " Have you removed the file manually?").arg(m_path));
            return;
        }
    } else if (info.isDir()) {
        error(KIO::ERR_ABORTED,
              i18n("File \"%1\" is a directory. This should not happen. Please file a bug.")
              .arg(m_path));
        return;
    }

    m_position = 0;
    position( 0 );
    emit opened();
}

void MagnetProtocol::read(filesize_t size)
{
    kDebug() << size;

    QFile file(m_path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (file.error()) {
            error(KIO::ERR_CANNOT_OPEN_FOR_READING,file.errorString());
        } else {
            error(KIO::ERR_CANNOT_OPEN_FOR_READING,
                  i18n("File exists in KTorrent, but cannot open it on disk at path \"%1\"."
                       " Have you removed the file manually?").arg(m_path));
        }
        return;
    }

    while (true) {
        if (m_downloaded || m_dbusHandler->seek(m_position+size)) {
            file.seek(m_position);
            QByteArray d = file.read(size);
            data(d);
            m_position += size;
            break;
        } else {
            usleep(10000);
        }
    }
    file.close();
    finished();
}

void MagnetProtocol::seek(filesize_t offset)
{
    kDebug() << offset;
    if (m_dbusHandler->seek(offset)) {
        m_position=offset;
        position(offset);
    } else {
        position(m_position);
    }
}

void MagnetProtocol::setPath(const QString& path)
{
    kDebug() << path;
    m_path = path;
}

void MagnetProtocol::setFiles(const QStringList& files )
{
    kDebug() << files;
    m_files = files;
}


void MagnetProtocol::setSize(qint64 size)
{
    kDebug() << size;
    m_size = size;
}

void MagnetProtocol::setNumFiles(int n)
{
    kDebug() << n;
    m_numFiles = n;
}

#include "kio_magnet.moc"

