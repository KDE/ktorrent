/***************************************************************************
*   Copyright (C) 2006 by Diego R. Brogna                                 *
*   dierbro@gmail.com                                                     *
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

#include <ctime>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QHostAddress>
#include <QRegExp>
#include <QSocketNotifier>
#include <QStringList>
#include <QTimer>

#include <KApplication>
#include <KCodecs>
#include <KGenericFactory>
#include <KGlobal>
#include <KStandardDirs>
#include <KTemporaryFile>
#include <k3streamsocket.h>
#include <k3resolver.h>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/mmapfile.h>
#include <util/sha1hash.h>
#include "ktversion.h"
#include "httpserver.h"
#include "httpclienthandler.h"
#include "httpresponseheader.h"
#include "webinterfacepluginsettings.h"
#include "torrentlistgenerator.h"
#include "challengegenerator.h"
#include "loginhandler.h"
#include "logouthandler.h"
#include "actionhandler.h"
#include "iconhandler.h"
#include "torrentposthandler.h"
#include "torrentfilesgenerator.h"
#include "globaldatagenerator.h"
#include "settingsgenerator.h"



using namespace bt;

namespace kt
{
    QString DataDir();


    HttpServer::HttpServer(CoreInterface* core, bt::Uint16 port) : core(core), cache(10), port(port)
    {
        qsrand(time(0));
        content_generators.setAutoDelete(true);
        addContentGenerator(new TorrentListGenerator(core, this));
        addContentGenerator(new ChallengeGenerator(this));
        addContentGenerator(new LoginHandler(this));
        addContentGenerator(new LogoutHandler(this));
        addContentGenerator(new ActionHandler(core, this));
        addContentGenerator(new TorrentPostHandler(core, this));
        addContentGenerator(new TorrentFilesGenerator(core, this));
        addContentGenerator(new IconHandler(this));
        addContentGenerator(new GlobalDataGenerator(core, this));
        addContentGenerator(new SettingsGenerator(core, this));

        QStringList dirList = KGlobal::dirs()->findDirs("data", "ktorrent/www");
        if (!dirList.empty())
        {
            rootDir = dirList.front();
            Out(SYS_WEB | LOG_DEBUG) << "WWW Root Directory " << rootDir << endl;
        }
        session.logged_in = false;


        QStringList bind_addresses;
        bind_addresses << QHostAddress(QHostAddress::Any).toString();
        bind_addresses << QHostAddress(QHostAddress::AnyIPv6).toString();
        foreach (const QString& addr, bind_addresses)
        {
            net::ServerSocket::Ptr sock(new net::ServerSocket(this));
            if (sock->bind(addr, port))
                sockets.append(sock);
        }

        if (!rootDir.isEmpty())
        {
            skin_list = QDir(rootDir).entryList(QDir::Dirs);
            skin_list.removeAll("common");
            skin_list.removeAll(".");
            skin_list.removeAll("..");
            foreach (QString s, skin_list)
                Out(SYS_WEB | LOG_DEBUG) << "skin: " << s << endl;
        }
    }

    HttpServer::~HttpServer()
    {
        qDeleteAll(clients);
    }

    QString HttpServer::skinDir() const
    {
        QString skin;
        if (skin_list.count() == 0)
        {
            skin = "default";
        }
        else
        {
            int s = WebInterfacePluginSettings::skin();
            if (s < 0 || s >= skin_list.count())
                s = 0;

            skin = skin_list.at(s);
            if (skin.length() == 0)
                skin = "default";
        }

        return rootDir + bt::DirSeparator() + skin;
    }

    QString HttpServer::commonDir() const
    {
        return rootDir + bt::DirSeparator() + "common";
    }

    void HttpServer::newConnection(int fd, const net::Address& addr)
    {
        HttpClientHandler* handler = new HttpClientHandler(this, fd);
        connect(handler, &HttpClientHandler::closed, this, &HttpServer::slotConnectionClosed);
        Out(SYS_WEB | LOG_NOTICE) << "connection from " << addr.toString()  << endl;
        clients.append(handler);
    }


    bool HttpServer::checkLogin(const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        // Authentication is disabled
        if (!WebInterfacePluginSettings::authentication())
        {
            session.logged_in = true;
            session.sessionId = rand();
            session.last_access = QTime::currentTime();
            Out(SYS_WEB | LOG_NOTICE) << "Webgui login successful ! (auth disable)" << endl;
            challenge = QString();
            return true;
        }

        if (hdr.contentType() != "application/x-www-form-urlencoded")
        {
            Out(SYS_WEB | LOG_NOTICE) << "Webgui login failed ! 1" << endl;
            challenge = QString();
            return false;
        }

        QString username;
        QString challenge_hash;
        QStringList params = QString(data).split("&");
        for (QStringList::iterator i = params.begin(); i != params.end(); i++)
        {
            QString t = *i;
            if (t.section("=", 0, 0) == "username")
                username = t.section("=", 1, 1);
            else if (t.section("=", 0, 0) == "challenge")
                challenge_hash = t.section("=", 1, 1);
        }

        if (username.isEmpty() || challenge.isNull() || username != WebInterfacePluginSettings::username())
        {
            Out(SYS_WEB | LOG_NOTICE) << "Webgui login failed ! 2" << endl;
            challenge = QString();
            return false;
        }

        QByteArray s = (QString(challenge + WebInterfacePluginSettings::password())).toUtf8();
        bt::SHA1Hash hash = bt::SHA1Hash::generate((const bt::Uint8*)s.data(), s.length());
        if (hash.toString() == challenge_hash)
        {
            session.logged_in = true;
            session.sessionId = rand();
            session.last_access = QTime::currentTime();
            Out(SYS_WEB | LOG_NOTICE) << "Webgui login successful !" << endl;
            challenge = QString();
            return true;
        }
        challenge = QString();
        Out(SYS_WEB | LOG_NOTICE) << "Webgui login failed ! 3" << endl;
        return false;
    }


    void HttpServer::logout()
    {
        session.logged_in = false;
        session.sessionId = 0;
        challenge = QString();
        Out(SYS_WEB | LOG_NOTICE) << "Webgui logout" << endl;
    }

    bool HttpServer::checkSession(const QHttpRequestHeader& hdr)
    {
        // check session in cookie
        int session_id = 0;
        if (hdr.hasKey("Cookie"))
        {
            QString cookie = hdr.value("Cookie");
            QRegExp rx("KT_SESSID=(\\d+)", Qt::CaseInsensitive);
            int pos = 0;

            while ((pos = rx.indexIn(cookie, pos)) != -1)
            {
                session_id = rx.cap(1).toInt();
                if (session_id == session.sessionId)
                    break;
                pos += rx.matchedLength();
            }
        }


        if (session_id == session.sessionId)
        {
            // check if the session hasn't expired yet
            if (session.last_access.secsTo(QTime::currentTime()) < WebInterfacePluginSettings::sessionTTL())
            {
                session.last_access = QTime::currentTime();
            }
            else
            {
                return false;
            }
        }
        else
            return false;

        return true;
    }

    static QString ExtensionToContentType(const QString& ext)
    {
        if (ext == "html")
            return "text/html";
        else if (ext == "css")
            return "text/css";
        else if (ext == "js")
            return "text/javascript";
        else if (ext == "gif" || ext == "png" || ext == "ico")
            return "image/" + ext;
        else if (ext == "jpg" || ext == "jpeg")
            return "image/jpeg";
        else if (ext == "svg" || ext == "svgz")
            return "image/svg+xml";
        else
            return QString::null;
    }

    // HTTP needs non translated dates
    static QString days[] =
    {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    static QString months[] =
    {
        "Jan", "Feb", "Mar", "Apr",
        "May", "Jun", "Jul", "Aug",
        "Sep", "Oct", "Nov", "Dec"
    };

    static QString DateTimeToString(const QDateTime& now, bool cookie)
    {
        if (!cookie)
            return now.toString("%1, dd %2 yyyy hh:mm:ss UTC")
                   .arg(days[now.date().dayOfWeek() - 1])
                   .arg(months[now.date().month() - 1]);
        else
            return now.toString("%1, dd-%2-yyyy hh:mm:ss GMT")
                   .arg(days[now.date().dayOfWeek() - 1])
                   .arg(months[now.date().month() - 1]);
    }

    void HttpServer::setDefaultResponseHeaders(HttpResponseHeader& hdr, const QString& content_type, bool with_session_info)
    {
        hdr.setValue("Server", "KTorrent/" KT_VERSION_MACRO);
        hdr.setValue("Date", DateTimeToString(QDateTime::currentDateTime().toUTC(), false));
        if (!content_type.isEmpty())
            hdr.setValue("Content-Type", content_type);

        if (with_session_info && session.sessionId && session.logged_in)
        {
            hdr.setValue("Set-Cookie", QString("KT_SESSID=%1").arg(session.sessionId));
        }
    }

    void HttpServer::redirectToLoginPage(HttpClientHandler* hdlr)
    {
        HttpResponseHeader rhdr(302);
        setDefaultResponseHeaders(rhdr, "text/html", false);
        rhdr.setValue("Location", "login.html");
        rhdr.setValue("Content-Length", "0");
        hdlr->sendResponse(rhdr);
        Out(SYS_WEB | LOG_NOTICE) << "Redirecting to /login.html" << endl;
    }



    void HttpServer::handleGet(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        if (rootDir.isEmpty())
        {
            HttpResponseHeader rhdr(500, hdr.majorVersion(), hdr.minorVersion());
            setDefaultResponseHeaders(rhdr, "text/html", false);
            hdlr->send500(rhdr, i18n("Cannot find web interface skins."));
            return;
        }

        QString file = hdr.path();
        if (file == "/" && WebInterfacePluginSettings::authentication())
            file = "/login.html";
        else if (file == "/")
            file = "/interface.html";

        KUrl url;
        url.setEncodedPathAndQuery(file);

        Out(SYS_WEB | LOG_DEBUG) << "GET " << hdr.path() << endl;
        WebContentGenerator* gen = content_generators.find(url.path());
        if (gen)
        {
            if ((gen->getPermissions() == WebContentGenerator::LOGIN_REQUIRED && (!session.logged_in || !checkSession(hdr))) && WebInterfacePluginSettings::authentication())
            {
                // redirect to login page
                redirectToLoginPage(hdlr);
            }
            else
            {
                gen->get(hdlr, hdr);
            }
        }
        else
        {
            QString path = commonDir() + url.path();
            // first try the common dir
            if (!bt::Exists(path)) // doesn't exist so it must be in the skin dir
                path = skinDir() + url.path();

            handleFile(hdlr, hdr, path);
        }
    }

    void HttpServer::handleFile(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QString& path)
    {
        // check if the file exists (if not send 404)
        if (!bt::Exists(path))
        {
            HttpResponseHeader rhdr(404, hdr.majorVersion(), hdr.minorVersion());
            setDefaultResponseHeaders(rhdr, "text/html", false);
            hdlr->send404(rhdr, path);
            return;
        }

        QString file = hdr.path();
        if (file == "/" && WebInterfacePluginSettings::authentication())
            file = "/login.html";
        else if (file == "/")
            file = "/interface.html";

        QFileInfo fi(path);
        QString ext = fi.suffix();;

        if (ext == "html")
        {
            // html pages require a login unless it is the login.html page
            if ((file != "/login.html" && (!session.logged_in || !checkSession(hdr))) && WebInterfacePluginSettings::authentication())
            {
                // redirect to login page
                redirectToLoginPage(hdlr);
                return;
            }

            HttpResponseHeader rhdr(200, hdr.majorVersion(), hdr.minorVersion());
            setDefaultResponseHeaders(rhdr, "text/html", true);
            if (path.endsWith("login.html"))
            {
                // clear cookie in case of login page
                QDateTime dt = QDateTime::currentDateTime().addDays(-1);
                QString cookie = QString("KT_SESSID=666; expires=%1 +0000").arg(DateTimeToString(dt, true));
                rhdr.setValue("Set-Cookie", cookie);
            }

            if (!hdlr->sendFile(rhdr, path))
            {
                HttpResponseHeader nhdr(404, hdr.majorVersion(), hdr.minorVersion());
                setDefaultResponseHeaders(nhdr, "text/html", false);
                hdlr->send404(nhdr, path);
            }
        }
        else if (ext == "css" || ext == "js" || ext == "png" || ext == "ico" || ext == "gif" || ext == "jpg")
        {
            handleNormalFile(hdlr, hdr, path);
        }
        else
        {
            HttpResponseHeader rhdr(404, hdr.majorVersion(), hdr.minorVersion());
            setDefaultResponseHeaders(rhdr, "text/html", false);
            hdlr->send404(rhdr, file);
        }
    }

    void HttpServer::handleNormalFile(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QString& path)
    {
        QFileInfo fi(path);
        QString ext = fi.suffix();

        if (hdr.hasKey("If-Modified-Since"))
        {
            QDateTime dt = parseDate(hdr.value("If-Modified-Since"));
            if (dt.isValid() && dt < fi.lastModified())
            {
                HttpResponseHeader rhdr(304, hdr.majorVersion(), hdr.minorVersion());
                setDefaultResponseHeaders(rhdr, "text/html", true);
                rhdr.setValue("Cache-Control", "max-age=0");
                rhdr.setValue("Last-Modified", DateTimeToString(fi.lastModified(), false));
                rhdr.setValue("Expires", DateTimeToString(QDateTime::currentDateTime().toUTC().addSecs(3600), false));
                hdlr->sendResponse(rhdr);
                return;
            }
        }

        HttpResponseHeader rhdr(200, hdr.majorVersion(), hdr.minorVersion());
        setDefaultResponseHeaders(rhdr, ExtensionToContentType(ext), true);
        rhdr.setValue("Last-Modified", DateTimeToString(fi.lastModified(), false));
        rhdr.setValue("Expires", DateTimeToString(QDateTime::currentDateTime().toUTC().addSecs(3600), false));
        rhdr.setValue("Cache-Control", "private");
        if (!hdlr->sendFile(rhdr, path))
        {
            HttpResponseHeader nhdr(404, hdr.majorVersion(), hdr.minorVersion());
            setDefaultResponseHeaders(nhdr, "text/html", false);
            hdlr->send404(nhdr, path);
        }
    }

    void HttpServer::handlePost(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        Out(SYS_WEB | LOG_DEBUG) << "POST " << hdr.path() << endl;
        KUrl url;
        url.setEncodedPathAndQuery(hdr.path());
        WebContentGenerator* gen = content_generators.find(url.path());
        if (gen)
        {
            if ((gen->getPermissions() == WebContentGenerator::LOGIN_REQUIRED && (!session.logged_in || !checkSession(hdr))) && WebInterfacePluginSettings::authentication())
            {
                // redirect to login page
                redirectToLoginPage(hdlr);
            }
            else
            {
                gen->post(hdlr, hdr, data);
            }
        }
        else
        {
            KUrl url;
            url.setEncodedPathAndQuery(hdr.path());
            QString path = commonDir() + url.path();
            // first try the common dir
            if (!bt::Exists(path)) // doesn't exist so it must be in the skin dir
                path = skinDir() + url.path();

            handleFile(hdlr, hdr, path);
        }
    }

    void HttpServer::handleUnsupportedMethod(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        HttpResponseHeader rhdr(500, hdr.majorVersion(), hdr.minorVersion());
        setDefaultResponseHeaders(rhdr, "text/html", false);
        hdlr->send500(rhdr, i18n("Unsupported HTTP method"));
    }

    void HttpServer::slotConnectionClosed()
    {
        HttpClientHandler* client = (HttpClientHandler*)sender();
        clients.removeAll(client);
        client->deleteLater();
    }

    QDateTime HttpServer::parseDate(const QString& str)
    {
        /*
        Potential date formats :
            Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
            Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
            Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
        */
        QStringList sl = str.split(" ");
        if (sl.count() == 6)
        {
            // RFC 1123 format
            QDate d;
            QString month = sl[2];
            int m = -1;
            for (int i = 1; i <= 12 && m < 0; i++)
                if (QDate::shortMonthName(i) == month)
                    m = i;

            d.setYMD(sl[3].toInt(), m, sl[1].toInt());

            QTime t = QTime::fromString(sl[4], Qt::ISODate);
            return QDateTime(d, t);
        }
        else if (sl.count() == 4)
        {
            //  RFC 1036
            QStringList dl = sl[1].split("-");
            if (dl.count() != 3)
                return QDateTime();

            QDate d;
            QString month = dl[1];
            int m = -1;
            for (int i = 1; i <= 12 && m < 0; i++)
                if (QDate::shortMonthName(i) == month)
                    m = i;

            d.setYMD(2000 + dl[2].toInt(), m, dl[0].toInt());

            QTime t = QTime::fromString(sl[2], Qt::ISODate);
            return QDateTime(d, t);
        }
        else if (sl.count() == 5)
        {
            // ANSI C
            QDate d;
            QString month = sl[1];
            int m = -1;
            for (int i = 1; i <= 12 && m < 0; i++)
                if (QDate::shortMonthName(i) == month)
                    m = i;

            d.setYMD(sl[4].toInt(), m, sl[2].toInt());

            QTime t = QTime::fromString(sl[3], Qt::ISODate);
            return QDateTime(d, t);
        }
        else
            return QDateTime();
    }

    bt::MMapFile* HttpServer::cacheLookup(const QString& name)
    {
        return cache.object(name);
    }

    void HttpServer::insertIntoCache(const QString& name, bt::MMapFile* file)
    {
        cache.insert(name, file);
    }

    static char RandomLetterOrNumber()
    {
        int i = qrand() % 62;
        if (i < 26)
            return 'a' + i;
        else if (i < 52)
            return 'A' + (i - 26);
        else
            return '0' + (i - 52);
    }

    QString HttpServer::challengeString()
    {
        // regenerate challenge string
        challenge = QString();
        for (int i = 0; i < 20; i++)
            challenge.append(RandomLetterOrNumber());
        return challenge;
    }

    void HttpServer::addContentGenerator(WebContentGenerator* g)
    {
        content_generators.insert(g->getPath(), g);
    }

}

#include "httpserver.moc"
