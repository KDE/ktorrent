/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QMimeDatabase>
#include <QMimeType>
#include <QRegExp>

#include <KIO/JobUiDelegate>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>

#include "linkdownloader.h"
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <magnet/magnetlink.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
LinkDownloader::LinkDownloader(const QUrl &url,
                               kt::CoreInterface *core,
                               bool verbose,
                               const QString &group,
                               const QString &location,
                               const QString &move_on_completion)
    : url(url)
    , core(core)
    , verbose(verbose)
    , group(group)
    , location(location)
    , move_on_completion(move_on_completion)
{
    base_url = url.scheme() + QStringLiteral("://") + url.host();
    if (url.port(80) != 80)
        base_url += QLatin1Char(':') + QString::number(url.port(80));

    QString path = url.path();
    if (path.size() > 0) {
        int idx = -1;
        if (path.endsWith(QLatin1Char('/')))
            base_url += (!path.startsWith(QStringLiteral("/")) ? QStringLiteral("/") : QString()) + path;
        else if ((idx = path.lastIndexOf(QStringLiteral("/"))) != -1)
            base_url += path.mid(0, idx + 1);
        else
            base_url += QLatin1Char('/');
    } else
        base_url += QLatin1Char('/');
}

LinkDownloader::~LinkDownloader()
{
}

void LinkDownloader::downloadFinished(KJob *j)
{
    KIO::StoredTransferJob *job = (KIO::StoredTransferJob *)j;
    if (job->error()) {
        Out(SYS_SYN | LOG_NOTICE) << "Failed to download " << url.toDisplayString() << " : " << job->errorString() << endl;
        if (verbose)
            job->uiDelegate()->showErrorMessage();

        Q_EMIT finished(false);
        deleteLater();
        return;
    }

    if (isTorrent(job->data())) {
        bt::TorrentInterface *tc = nullptr;
        if (verbose)
            tc = core->load(job->data(), url, group, location);
        else
            tc = core->loadSilently(job->data(), url, group, location);

        if (tc && !move_on_completion.isEmpty())
            tc->setMoveWhenCompletedDir(move_on_completion);

        Q_EMIT finished(true);
        deleteLater();
    } else {
        QMimeType data_type = QMimeDatabase().mimeTypeForData(job->data());
        if (data_type.isValid() && data_type.name().contains(QStringLiteral("html")))
            handleHtmlPage(job->data());
    }
}

void LinkDownloader::start()
{
    KIO::StoredTransferJob *j = KIO::storedGet(url, KIO::Reload, verbose ? KIO::DefaultFlags : KIO::HideProgressInfo);
    connect(j, &KIO::StoredTransferJob::result, this, &LinkDownloader::downloadFinished);
}

bool LinkDownloader::isTorrent(const QByteArray &data) const
{
    bool ret = false;
    try {
        BDecoder decoder(data, false);
        const std::unique_ptr<BNode> node = decoder.decode();
        if (node)
            ret = true;
    } catch (...) {
        ret = false;
    }
    return ret;
}

void LinkDownloader::handleHtmlPage(const QByteArray &data)
{
    QRegExp rx(QLatin1String("href\\s*=\"([^\"]*)\""), Qt::CaseInsensitive);
    QString str(QString::fromUtf8(data));
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        QString href_link = rx.cap(1);
        if (href_link.startsWith(QStringLiteral("magnet:")) && href_link.contains(QStringLiteral("xt=urn:btih:"))) {
            MagnetLinkLoadOptions options;
            options.silently = verbose;
            options.group = group;
            options.location = location;
            options.move_on_completion = move_on_completion;
            core->load(bt::MagnetLink(href_link), options);
            Q_EMIT finished(true);
            deleteLater();
            return;
        } else if (!href_link.startsWith(QStringLiteral("http://")) && !href_link.startsWith(QStringLiteral("https://"))) {
            if (!href_link.startsWith(QStringLiteral("/")))
                href_link = base_url + href_link;
            else
                href_link = url.scheme() + QStringLiteral("://") + url.authority() + href_link;
        }

        link_url = QUrl(href_link);
        if (link_url.isValid())
            links.append(link_url);

        pos += rx.matchedLength();
    }

    tryTorrentLinks();
}

void LinkDownloader::tryTorrentLinks()
{
    // First try links ending with .torrent
    for (const QUrl &u : std::as_const(links)) {
        if (u.path().endsWith(QStringLiteral(".torrent")) || u.path().endsWith(QStringLiteral(".TORRENT"))) {
            Out(SYS_SYN | LOG_DEBUG) << "Trying torrent link: " << u.toDisplayString() << endl;
            link_url = u;
            KIO::StoredTransferJob *j = KIO::storedGet(u, KIO::Reload, verbose ? KIO::DefaultFlags : KIO::HideProgressInfo);
            connect(j, &KIO::StoredTransferJob::result, this, &LinkDownloader::torrentDownloadFinished);
            links.removeAll(u);
            return;
        }
    }

    // Try the next link in the list if there are no torrent links
    tryNextLink();
}

void LinkDownloader::tryNextLink()
{
    if (links.count() == 0) {
        Out(SYS_SYN | LOG_DEBUG) << "Couldn't find a valid link to a torrent on " << url.toDisplayString() << endl;
        if (verbose)
            KMessageBox::error(nullptr, i18n("Could not find a valid link to a torrent on %1", url.toDisplayString()));

        Q_EMIT finished(false);
        deleteLater();
        return;
    }

    link_url = links.front();
    links.pop_front();
    KIO::StoredTransferJob *j = KIO::storedGet(link_url, KIO::Reload, KIO::HideProgressInfo);
    connect(j, &KIO::StoredTransferJob::result, this, &LinkDownloader::torrentDownloadFinished);
    Out(SYS_SYN | LOG_DEBUG) << "Trying " << link_url.toDisplayString() << endl;
}

void LinkDownloader::torrentDownloadFinished(KJob *j)
{
    KIO::StoredTransferJob *job = (KIO::StoredTransferJob *)j;
    if (j->error()) {
        if (links.count() == 0) {
            Out(SYS_SYN | LOG_NOTICE) << "Failed to download torrent: " << job->errorString() << endl;
            if (verbose)
                job->uiDelegate()->showErrorMessage();

            Q_EMIT finished(false);
            deleteLater();
        } else
            tryTorrentLinks();
    } else if (isTorrent(job->data())) {
        bt::TorrentInterface *tc = nullptr;
        if (verbose)
            tc = core->load(job->data(), link_url, group, location);
        else
            tc = core->loadSilently(job->data(), link_url, group, location);

        if (tc && !move_on_completion.isEmpty())
            tc->setMoveWhenCompletedDir(move_on_completion);

        Q_EMIT finished(true);
        deleteLater();
    } else
        tryTorrentLinks();
}
}

#include "moc_linkdownloader.cpp"
