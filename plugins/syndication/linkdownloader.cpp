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
#include <QRegExp>
#include <kmimetype.h>
#include <klocalizedstring.h>
#include <kmessagebox.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <magnet/magnetlink.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include "linkdownloader.h"


using namespace bt;

namespace kt
{

    LinkDownloader::LinkDownloader(const KUrl& url,
                                   kt::CoreInterface* core,
                                   bool verbose,
                                   const QString& group,
                                   const QString& location,
                                   const QString& move_on_completion)
        : url(url), core(core), verbose(verbose), group(group),
          location(location), move_on_completion(move_on_completion)
    {
        base_url = url.protocol() + "://" + url.host();
        if (url.port(80) != 80)
            base_url += ":" + QString::number(url.port(80));

        QString path = url.path();
        if (path.size() > 0)
        {
            int idx = -1;
            if (path.endsWith("/"))
                base_url += (!path.startsWith("/") ? "/" : "") + path;
            else if ((idx = path.lastIndexOf("/")) != -1)
                base_url += path.mid(0, idx + 1);
            else
                base_url += "/";
        }
        else
            base_url += "/";
    }


    LinkDownloader::~LinkDownloader()
    {
    }

    void LinkDownloader::downloadFinished(KJob* j)
    {
        KIO::StoredTransferJob* job = (KIO::StoredTransferJob*)j;
        if (job->error())
        {
            Out(SYS_SYN | LOG_NOTICE) << "Failed to download " << url.prettyUrl() << " : " << job->errorString() << endl;
            if (verbose)
                job->ui()->showErrorMessage();

            finished(false);
            deleteLater();
            return;
        }

        if (isTorrent(job->data()))
        {
            bt::TorrentInterface* tc = 0;
            if (verbose)
                tc = core->load(job->data(), url, group, location);
            else
                tc = core->loadSilently(job->data(), url, group, location);

            if (tc && !move_on_completion.isEmpty())
                tc->setMoveWhenCompletedDir(move_on_completion);

            finished(true);
            deleteLater();
        }
        else
        {
            KMimeType::Ptr data_type = KMimeType::findByContent(job->data());
            if (data_type && data_type->name().contains("html"))
                handleHtmlPage(job->data());
        }
    }

    void LinkDownloader::start()
    {
        KIO::StoredTransferJob* j = KIO::storedGet(url, KIO::Reload, verbose ? KIO::DefaultFlags : KIO::HideProgressInfo);
        connect(j, SIGNAL(result(KJob*)), this, SLOT(downloadFinished(KJob*)));
    }

    bool LinkDownloader::isTorrent(const QByteArray& data) const
    {
        bool ret = false;
        try
        {
            BDecoder decoder(data, false);
            BNode* node = decoder.decode();
            if (node)
                ret = true;

            delete node;
        }
        catch (...)
        {
            ret = false;
        }
        return ret;
    }

    void LinkDownloader::handleHtmlPage(const QByteArray& data)
    {
        QRegExp rx("href\\s*=\"([^\"]*)\"", Qt::CaseInsensitive);
        QString str(data);
        int pos = 0;
        while ((pos = rx.indexIn(str, pos)) != -1)
        {
            QString href_link = rx.cap(1);
            if (href_link.startsWith("magnet:") && href_link.contains("xt=urn:btih:"))
            {
                MagnetLinkLoadOptions options;
                options.silently = verbose;
                options.group = group;
                options.location = location;
                options.move_on_completion = move_on_completion;
                core->load(bt::MagnetLink(href_link), options);
                finished(true);
                deleteLater();
                return;
            }
            else if (!href_link.startsWith("http://") && !href_link.startsWith("https://"))
            {
                if (!href_link.startsWith("/"))
                    href_link = base_url + href_link;
                else
                    href_link = url.protocol() + "://" + url.authority() + href_link;
            }

            link_url = KUrl(href_link);
            if (link_url.isValid())
                links.append(link_url);

            pos += rx.matchedLength();
        }

        tryTorrentLinks();
    }

    void LinkDownloader::tryTorrentLinks()
    {
        // First try links ending with .torrent
        foreach (KUrl u, links)
        {
            if (u.path().endsWith(".torrent") || u.path().endsWith(".TORRENT"))
            {
                Out(SYS_SYN | LOG_DEBUG) << "Trying torrent link: " << u.prettyUrl() << endl;
                link_url = u;
                KIO::StoredTransferJob* j = KIO::storedGet(u, KIO::Reload, verbose ? KIO::DefaultFlags : KIO::HideProgressInfo);
                connect(j, SIGNAL(result(KJob*)), this, SLOT(torrentDownloadFinished(KJob*)));
                links.removeAll(u);
                return;
            }
        }

        // Try the next link in the list if there are no torrent links
        tryNextLink();
    }

    void LinkDownloader::tryNextLink()
    {
        if (links.count() == 0)
        {
            Out(SYS_SYN | LOG_DEBUG) << "Couldn't find a valid link to a torrent on " << url.prettyUrl() << endl;
            if (verbose)
                KMessageBox::error(0, i18n("Could not find a valid link to a torrent on %1", url.prettyUrl()));

            finished(false);
            deleteLater();
            return;
        }

        link_url = links.front();
        links.pop_front();
        KIO::StoredTransferJob* j = KIO::storedGet(link_url, KIO::Reload, KIO::HideProgressInfo);
        connect(j, SIGNAL(result(KJob*)), this, SLOT(torrentDownloadFinished(KJob*)));
        Out(SYS_SYN | LOG_DEBUG) << "Trying " << link_url.prettyUrl() << endl;
    }

    void LinkDownloader::torrentDownloadFinished(KJob* j)
    {
        KIO::StoredTransferJob* job = (KIO::StoredTransferJob*)j;
        if (j->error())
        {
            if (links.count() == 0)
            {
                Out(SYS_SYN | LOG_NOTICE) << "Failed to download torrent: " << job->errorString() << endl;
                if (verbose)
                    job->ui()->showErrorMessage();

                finished(false);
                deleteLater();
            }
            else
                tryTorrentLinks();
        }
        else if (isTorrent(job->data()))
        {
            bt::TorrentInterface* tc = 0;
            if (verbose)
                tc = core->load(job->data(), link_url, group, location);
            else
                tc = core->loadSilently(job->data(), link_url, group, location);

            if (tc && !move_on_completion.isEmpty())
                tc->setMoveWhenCompletedDir(move_on_completion);

            finished(true);
            deleteLater();
        }
        else
            tryTorrentLinks();
    }
}
