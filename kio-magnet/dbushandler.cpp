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
#include "interfaces/ktorrentcoreinterface.h"
#include "interfaces/ktorrenttorrentinterface.h"
#include "interfaces/ktorrenttorrentfilestreaminterface.h"
#include "interfaces/ktorrentgroupinterface.h"

#include "dbushandler.h"
#include "kio_magnet.h"
#include "dbusthread.h"
#include "magnetsettings.h"

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusReply>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <knotification.h>
#include <KUrl>
#include <KProcess>
#include <kdebug.h>
#include <KMimeType>
#include <KMessageBox>
#include <kio/jobclasses.h>
#include <kio/job.h>

const int timeout = 5000;
const int repeat = 6;

DBusHandler::DBusHandler(MagnetProtocol* slave) :
        m_coreInt( 0 )
        , m_torrentInt( 0 )
        , m_streamInt( 0 )
        , m_process( 0 )
        , m_slave( slave )
        , m_file( -1 )
        , m_passedTime( 0 )
        , m_loadInProgress( true )
{
    kDebug() << "Thread: " << thread();
    m_thread = new DBusThread(this);
    m_thread->start();
    moveToThread(m_thread);
}

DBusHandler::~DBusHandler()
{
    kDebug();
    m_thread->exit();
    if (!m_tor.isEmpty()) {
        QStringList rt = MagnetSettings::runningTorrents();
        rt.removeAll(m_tor);
        MagnetSettings::setRunningTorrents(rt);
    }
    MagnetSettings::self()->writeConfig();
    if ( m_torrentInt )
        m_torrentInt->removeStream(m_file);
    delete m_process;
    delete m_streamInt;
    delete m_torrentInt;
    delete m_coreInt;
    delete m_thread;
}

void DBusHandler::init()
{
    kDebug();
    m_initMutex.lock();
    if (!MagnetSettings::enabled()) {
        m_slave->error(KIO::ERR_ABORTED,
                       i18n("The Web share for magnet-links is disabled. \
        You can set it up at settings:/network-and-connectivity/sharing"));
        m_initMutex.unlock();
    } else {
        initDBus();
    }
}

void DBusHandler::initDBus()
{
    kDebug() << "Thread: " << thread();
    m_coreInt = new org::ktorrent::core("org.ktorrent.ktorrent", "/core",
                                        QDBusConnection::sessionBus());

    if (!m_coreInt->isValid()) {
        kDebug() << "Could not initialize org.ktorrent.ktorrent /core.\
                     KTorrent seems to be not running.";
        m_process = new KProcess();
        if (m_process->startDetached("ktorrent")==-2) {
            m_slave->error(KIO::ERR_COULD_NOT_CONNECT,
                           i18n("Cannot start process for KTorrent.\
                           This should not happen, even if KTorrent is not installed.\
                           Check your machines resources and limits."));
            m_initMutex.unlock();
            return;
        }
        m_process->waitForStarted();
        QTimer::singleShot(timeout,this,SLOT(connectToDBus()));
    } else {
        connectToDBus();
    }
}

void DBusHandler::connectToDBus()
{
    kDebug();

    // TODO check if this test is necessary with the dbus wrapper interface
    if (!m_coreInt->isValid()) {
        delete m_coreInt;
        m_coreInt = new org::ktorrent::core("org.ktorrent.ktorrent", "/core",
                                            QDBusConnection::sessionBus());
        if (!m_coreInt->isValid()) {
            if (m_passedTime<repeat*timeout) {
                m_passedTime+=timeout;
                QTimer::singleShot(timeout,this,SLOT(initializeDBus()));
            } else {
                m_slave->error(KIO::ERR_COULD_NOT_CONNECT,
                               i18n("Could not connect to KTorrent via DBus\
                                     after %1 seconds. Is it broken?")
                               .arg(m_passedTime));
                m_initMutex.unlock();
                return;
            }
        }
    } else {
        setupDBus();
    }
}

void DBusHandler::setupDBus()
{
    kDebug();
    QDBusConnection bus = QDBusConnection::sessionBus();
    QDBusReply<QStringList>groupList = m_coreInt->groups();
    if (!groupList.isValid()) {
        m_slave->error(KIO::ERR_COULD_NOT_CONNECT,
                       i18n("Could not get the group list, do you have a"
                            " compatible KTorrent version running?"));
        m_initMutex.unlock();
        return;
    } else {
        if (!groupList.value().contains("MagnetShare")) {
            m_coreInt->addGroup("MagnetShare");
            org::ktorrent::group groupInt("org.ktorrent.ktorrent",
                                          "/group/MagnetShare", bus );
            KStandardDirs *dirs = new KStandardDirs();

            groupInt.setDefaultSaveLocation(
                dirs->saveLocation("data","kio_magnet/") );
            groupInt.setMaxShareRatio(MagnetSettings::maxShareRatio());
            delete dirs;
        } else {
            org::ktorrent::group groupInt("org.ktorrent.ktorrent",
                                          "/group/MagnetShare", bus );
            // respect changes inside KTorrent
            if (groupInt.maxShareRatio().isValid())
                MagnetSettings::setMaxShareRatio(groupInt.maxShareRatio());
            else
                groupInt.setMaxShareRatio(MagnetSettings::maxShareRatio());
        }
    }


    connect(m_coreInt,SIGNAL(torrentAdded(const QString&))
            ,this,SLOT(slotTorrentAdded (const QString&)));
    connect(m_coreInt,SIGNAL(finished(const QString&))
            ,this,SLOT(slotFinished(const QString&)));
    connect(m_coreInt,SIGNAL(torrentRemoved(const QString&))
            ,this,SLOT(slotTorrentRemoved(const QString&)));
    connect(m_coreInt
            ,SIGNAL(torrentStoppedByError(const QString&, const QString&))
            ,this
            ,SLOT(slotTorrentStoppedByError(const QString&, const QString&)));

    startTimer( 1000 );

    m_initMutex.unlock();
}

void DBusHandler::timerEvent(QTimerEvent* event)
{
    Q_UNUSED( event );

    kDebug();
    if ( !m_torrentInt || m_file == -1 )
        return;

    if ( m_torrentInt->filePercentage(m_file)==100 ) {
        m_slave->downloaded(true);
    }

}

bool DBusHandler::seek(qint64 pos)
{
    if ( !m_streamInt )
        return false;
    else
        return m_streamInt->seek( pos );
}

qlonglong DBusHandler::fileSize(qint32 idx)
{
    kDebug();
    Q_ASSERT( m_torrentInt->isValid() );

    return m_torrentInt->fileSize(idx);
}

bool DBusHandler::createFileStream( int file )
{
    kDebug();
    Q_ASSERT( m_torrentInt->isValid() );

    int speed = m_url.queryItem("sp").toInt();
    // only one stream per torrent is allowed atm
    if ( !speed || m_streamInt ) {
        return false;
    }

    if ( m_torrentInt->createStream( file == -1 ? 0 : file ) ) {
        m_streamInt = new org::ktorrent::torrentfilestream(
            "org.ktorrent.ktorrent",
            "/torrent/"+m_tor+"/stream",
            QDBusConnection::sessionBus() );
        return true;
    } else {
        return false;
    }
}

bool DBusHandler::load(const KUrl& u)
{
    kDebug() << u.url();
    
    m_initMutex.lock();
    m_initMutex.unlock();

    m_url = u; //"magnet:"+u.query();
    QString xt = u.queryItem("xt");
    m_path = u.queryItem("pt");

    // support extended magnet address space to something url like
    // magnet://urn.btih.HASH/path?blabla
    if ( u.hasHost() && u.host().contains("btih") )  {
        if ( xt.isEmpty() || !xt.contains("urn:btih:") ) {
            QRegExp btihHash("([^\\.]+).btih");
            if ( btihHash.indexIn(u.host()) != -1 ) {
                xt = "urn:btih:"+btihHash.cap(1);
            }
        }
        if ( u.hasPath() && u.path() != "/" ) {
            // TODO find out why RemoveTrailingSlash does not work
            m_path = u.path(KUrl::RemoveTrailingSlash).remove(QRegExp("^/"));
        }
    }

    if ( xt.isEmpty() || !xt.contains( "urn:btih:" ) ) {
        m_slave->error(KIO::ERR_ABORTED
                       ,i18n("The link for %1 does not contain the required btih hash-parameter.")
                       .arg(u.url()
                           ));
        return true;
    }

    if ( m_tor != xt.remove("urn:btih:") ) {
        if ( m_torrentInt ) {
            m_torrentInt->removeStream( m_file );
            delete m_streamInt;
            m_streamInt = 0;
            delete m_torrentInt;
        }
        m_file=-1;
        m_files.clear();
        m_loadInProgress = false;
        m_tor = xt.remove("urn:btih:");
        m_torrentInt = new org::ktorrent::torrent("org.ktorrent.ktorrent"
                , "/torrent/"+m_tor
                , QDBusConnection::sessionBus());
    }

    QDBusReply<QString>name = m_torrentInt->name();
    if (name.isValid()) {
        kDebug() << "Torrent " + name.value()
        + "("+ m_tor + ") already loaded in KTorrent.";
        selectFiles(false);
        // HACK speed things up by forcing a restart
        if ( m_torrentInt->downloadSpeed() == 0
                && m_torrentInt->bytesLeftToDownload() != 0
           ) {
            m_coreInt->stop(m_tor);
            m_coreInt->start(m_tor);
        }
        torrentLoaded();
        return true;
    } else {
//         QString torrentUrl = m_url.queryItem("to");
//         KUrl source( torrentUrl );
//         if (!MagnetSettings::trustedHosts().contains(source.host())) {
//             if ( m_slave->messageBox(KIO::SlaveBase::WarningYesNo,
//                                      i18n("The host \"%1\" is not known yet.
//Do you want to trust its shared sources?")
//                                      .arg(source.host()),
//                                      i18n("Host not known")) !=
//KMessageBox::Yes ) {
//                 kDebug()<<"host rejected.";
//                 m_slave->error(KIO::ERR_ABORTED,i18n("Host rejected."));
//                 return;
//             } else {
//                 QStringList hosts = MagnetSettings::trustedHosts();
//                 hosts << source.host();
//                 MagnetSettings::setTrustedHosts(hosts);
//             }
//         }
        if (!m_loadInProgress) {
            QString dn = m_url.queryItem("dn");
            if ( dn.isEmpty() )
                dn = i18n( "(no name given)" );
            QString pt = m_url.queryItem("pt");
            if ( pt.isEmpty() )
                pt = i18n( "(no path given)" );
            QString to = m_url.queryItem("to");
            if ( to.isEmpty() )
                to = i18n( "(magnet dht)" );
            // Does not work, why? TODO
//                      KNotification::event(KNotification::Notification, i18n(
//"Magnet-Link information" ),
//                                          i18n( "Loaded torrent \"%1\" from
//\"%2\" for file \"%3\"." ).arg(dn).arg(to).arg(pt) );
//                                                              KIcon(
//"kt-magnet" ).pixmap( QSize(64,64) ));
            /*if ( m_slave->messageBox( KIO::SlaveBase::QuestionYesNo,
                                      i18n( "Do you want to download and share
            the file: \"%1\" of torrent: \"%2\"?" ).arg(pt).arg(dn),
                                      i18n( "Magnet-Link confirmation" ) ) !=
            KMessageBox::No ) {
            } else {
                m_slave->error(KIO::ERR_USER_CANCELED,i18n("Torrent \"%1\"
            rejected by user.").arg(dn));
            }*/
            m_loadInProgress = true;
            m_coreInt->loadSilently(m_url.url(),"MagnetShare");
        }
    }
    return false;
}

void DBusHandler::loadFiles()
{
    kDebug();
    Q_ASSERT(m_torrentInt->isValid());

    if (m_files.isEmpty()) {
        qint64 n = m_torrentInt->numFiles();
        for (int i=0; i<n; i++) {
            m_files << m_torrentInt->filePath(i);
        }
    }
    m_slave->setFiles(m_files);
}

void DBusHandler::selectFiles(bool init)
{
    kDebug();
    Q_ASSERT(m_torrentInt->isValid());
    loadFiles();

    if (m_files.isEmpty()) {
        m_file=-1;
        m_slave->setNumFiles(0);
        return;
    }

    m_file=m_files.indexOf(m_path);
    kDebug() << "file: " << m_path << "id: " << m_file;

    qint32 n =  m_torrentInt->numFiles();
    kDebug() << "Number of files: " << n;
    m_slave->setNumFiles(n);

    QStringList prefetches = m_url.queryItem("pf").split(",");
    // TODO somehow files get deselected on finished torrents sometimes
    // so avoid selecting when the torrent is completely downloaded
    // as this makes sense anyway
    if ( m_torrentInt->bytesDownloaded() >= m_torrentInt->totalSize() )
        return;

    // give the actually requested file an even higher priority
    // this is not achievable in the KTorrent UI, TODO
    m_torrentInt->setFilePriority(m_file,60);
    if ( !prefetches.contains("all") ) {
        for (qint32 i=0; i<n; i++) {
            bool dwnld = ( i==m_file || prefetches.contains( m_files[i] ) );
            if ( init || dwnld ) {
                m_torrentInt->setDoNotDownload(i, !dwnld);
            }
            if ( dwnld ) {
                m_torrentInt->setFilePriority(i, 50);
            }
        }
    } else {
        for ( int i = 0; i < m_files.count(); i++ ) {
            m_torrentInt->setDoNotDownload(i, false);
            foreach ( QString pf, prefetches ) {
                if ( m_files[i].contains(QRegExp("^"+QRegExp::escape(pf))) ) {
                    m_torrentInt->setFilePriority(i,50);
                }
            }
        }
    }
}

void DBusHandler::torrentLoaded()
{
    kDebug();
    QString filename, path;
    qlonglong size;
    bool downloaded = false;
    if (m_file==-1) {
        filename = m_torrentInt->path();
        path = m_torrentInt->pathOnDisk();
        size = m_torrentInt->totalSize();
        downloaded = ( m_torrentInt->bytesLeftToDownload()==0 );
    } else {
        filename = m_torrentInt->filePath(m_file);
        path = m_torrentInt->filePathOnDisk(m_file);
        size = m_torrentInt->fileSize(m_file);
        downloaded = ( m_torrentInt->filePercentage(m_file)==100 );
    }
    m_slave->setPath( path );
    m_slave->setSize( size );
    m_slave->downloaded( downloaded );

    if ( !downloaded ) {
        createFileStream(m_file);
    }

    m_slave->added(filename);
}

void DBusHandler::cleanup()
{
    kDebug();
    /*    if (!m_torrentInt)
            return;

        qulonglong totalSize=0;
        QHash<QString,double> shareRatioList;
        foreach( QString torrent, MagnetSettings::managedTorrents() ) {
            if (torrent==m_tor ||
    MagnetSettings::runningTorrents().contains((torrent)))
                break;
            KTorrentTorrentInterface* torrentInt = new
    org::ktorrent::torrent("org.ktorrent.ktorrent", "/torrent/"+torrent,
                    QDBusConnection::sessionBus());
            if (!torrentInt->isValid() ||
                    (torrentInt->bytesLeft()==0 &&
    torrentInt->shareRatio()>=MagnetSettings::maxShareRatio())) {
                QStringList mt = MagnetSettings::managedTorrents();
                mt.removeAll(torrent);
                MagnetSettings::setManagedTorrents( mt );
                delete torrentInt;
                torrentInt = 0;
                m_coreInt->remove(torrent,true);
                kDebug() << "removed " << torrent << " torrent.";
                continue;
            }
            totalSize+=torrentInt->bytesDownloaded(); // TODO might get wrong
    results if network connection is buggy?
            if (torrentInt->bytesLeft()==0) {
                shareRatioList[torrent]=torrentInt->shareRatio();
                kDebug() << torrent << "-> " << torrentInt->shareRatio();
            }
            delete torrentInt;
        }
        // TODO add ktorrent plugin/support to allow sane group size management?
        // this only removes torrents when we run out of configured space,
    starting with the most shared ones
        if ( totalSize>(MagnetSettings::shareSize()*1024*1024) &&
    m_torrentInt->isValid() ) {
            totalSize+=m_file!=-1 ? m_torrentInt->fileSize(m_file) :
    m_torrentInt->totalSize();

            QList<double> sortedRatios = shareRatioList.values();
            qSort( sortedRatios );
            foreach ( double ratio, sortedRatios ) {
                QString tor = shareRatioList.key(ratio);
                KTorrentTorrentInterface* torrentInt = new
    org::ktorrent::torrent("org.ktorrent.ktorrent", "/torrent/"+tor,
                        QDBusConnection::sessionBus());
                totalSize-=torrentInt->bytesDownloaded();
                delete torrentInt;
                shareRatioList.take(tor);
                m_coreInt->remove(tor,true);
                if ( totalSize<MagnetSettings::shareSize()*1024*1024 ) {
                    break;
                }
            }
        }

        MagnetSettings::self()->writeConfig();*/
}

void DBusHandler::slotFinished(const QString& tor)
{
    kDebug()<< tor;
    m_slave->downloaded(true);
}

void DBusHandler::slotTorrentAdded(const QString& tor)
{
    kDebug()<< tor;

    if ( tor != m_tor )
        return;

    // wait until we have selected the files (can take some time)
    m_coreInt->stop(m_tor);
    selectFiles(true);

    QStringList mt = MagnetSettings::managedTorrents();
    QStringList rt = MagnetSettings::runningTorrents();
    mt.append(m_tor);
    rt.append(m_tor);
    MagnetSettings::setManagedTorrents(mt);
    MagnetSettings::setRunningTorrents(rt);
    MagnetSettings::self()->writeConfig();

    cleanup();
    m_coreInt->start(m_tor);
    torrentLoaded();
}

void DBusHandler::slotTorrentRemoved(const QString& tor)
{
    kDebug()<< tor;
    if ( m_tor == tor) {
        m_slave->error(KIO::ERR_DOES_NOT_EXIST,i18n("Torrent has been removed from KTorrent."));
        QStringList mt = MagnetSettings::managedTorrents();
        mt.removeAll(m_tor);
        MagnetSettings::setManagedTorrents(mt);
    }
}

void DBusHandler::slotTorrentStoppedByError(const QString& tor, const QString& error)
{
    kDebug()<< tor << error;
    if ( m_tor == tor) {
        m_slave->error(KIO::ERR_ABORTED,error);
        QStringList mt = MagnetSettings::managedTorrents();
        mt.removeAll(m_tor);
        MagnetSettings::setManagedTorrents(mt);
    }
}


#include "dbushandler.moc"
