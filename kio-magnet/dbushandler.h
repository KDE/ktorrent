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

#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

#include "interfaces/ktorrentcoreinterface.h"
#include "interfaces/ktorrenttorrentinterface.h"
#include "interfaces/ktorrenttorrentfilestreaminterface.h"

#include <KUrl>

#include <QObject>
#include <QtCore/QMutex>
#include <QtDBus/QDBusConnection>

class KProcess;;
class MagnetProtocol;
class DBusThread;
class QThread;

class DBusHandler : public QObject
{
    Q_OBJECT
public:
    DBusHandler( MagnetProtocol* );
    ~DBusHandler();
    void init();
    void load(const KUrl&);
    bool seek(qint64 pos);
    qlonglong fileSize(qint32 idx);

private slots:
    void cleanup();
    void connectToDBus();
    void slotFinished(const QString&);
    void slotTorrentAdded(const QString&);
    void slotTorrentRemoved(const QString&);
    void slotTorrentStoppedByError( const QString&, const QString&);
private:
    void initDBus();
    void setupDBus();
    void timerEvent(QTimerEvent *event);
    void loadFiles();
    void selectFiles(bool);
    void torrentLoaded();
    bool createFileStream( int file );
    org::ktorrent::core* m_coreInt;
    org::ktorrent::torrent* m_torrentInt;
    org::ktorrent::torrentfilestream* m_streamInt;
    KProcess* m_process;
    QDBusConnection* m_bus;
    KUrl m_url;
    QMutex m_initMutex;
    QThread* m_thread;
    MagnetProtocol* m_slave;
    QString m_tor, m_path;
    QStringList m_files;
    int m_file, m_passedTime;
    bool m_loadInProgress;
};

#endif // DBUSHANDLER_H
