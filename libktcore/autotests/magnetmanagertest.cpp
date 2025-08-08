/*
 *   SPDX-FileCopyrightText: 2025 (c) Jack Hill <jackhill3103@gmail.com>
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <torrent/magnetmanager.h>

#include <QSignalSpy>
#include <QTest>

using namespace Qt::Literals::StringLiterals;

namespace kt
{

class MagnetManagerTests : public QObject
{
    Q_OBJECT

private:
    const MagnetLinkLoadOptions default_options{
        .silently = false,
        .group = ""_L1,
        .location = ""_L1,
        .move_on_completion = ""_L1,
    };

    static constexpr QLatin1StringView magnet1 =
        "magnet:?xt=urn:btih:dd8255ecdc7ca55fb0bbf81323d87062db1f6d1c&dn=Big+Buck+Bunny&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fbig-buck-bunny.torrent"_L1;

    static constexpr QLatin1StringView magnet2 =
        "magnet:?xt=urn:btih:c9e15763f722f23e98a29decdfae341b98d53056&dn=Cosmos+Laundromat&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fcosmos-laundromat.torrent"_L1;

    static constexpr QLatin1StringView magnet3 =
        "magnet:?xt=urn:btih:08ada5a7a6183aae1e09d831df6748d566095a10&dn=Sintel&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fsintel.torrent"_L1;

    static constexpr QLatin1StringView magnet4 =
        "magnet:?xt=urn:btih:209c8226b299b308beaf2b9cd3fb49212dbd13ec&dn=Tears+of+Steel&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Ftears-of-steel.torrent"_L1;

    static constexpr QLatin1StringView magnet5 =
        "magnet:?xt=urn:btih:a88fda5954e89178c372716a6a78b8180ed4dad3&dn=The+WIRED+CD+-+Rip.+Sample.+Mash.+Share&tr=udp%3A%2F%2Fexplodie.org%3A6969&tr=udp%3A%2F%2Ftracker.coppersurfer.tk%3A6969&tr=udp%3A%2F%2Ftracker.empire-js.us%3A1337&tr=udp%3A%2F%2Ftracker.leechers-paradise.org%3A6969&tr=udp%3A%2F%2Ftracker.opentrackr.org%3A1337&tr=wss%3A%2F%2Ftracker.btorrent.xyz&tr=wss%3A%2F%2Ftracker.fastcast.nz&tr=wss%3A%2F%2Ftracker.openwebtorrent.com&ws=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2F&xs=https%3A%2F%2Fwebtorrent.io%2Ftorrents%2Fwired-cd.torrent"_L1;

    const bt::MagnetLink mlink1{magnet1};

    const bt::MagnetLink mlink2{magnet2};

    const bt::MagnetLink mlink3{magnet3};

    const bt::MagnetLink mlink4{magnet4};

    const bt::MagnetLink mlink5{magnet5};

private Q_SLOTS:

    void testUpdateWithNoMagnets()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};

        mman.update();
        QVERIFY(update_queue_spy.isEmpty());
    }

    void testAddAndRemoveOneMagnetUnfinished()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};

        mman.addMagnet(mlink1, default_options, false);
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);

        mman.update();
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);

        mman.removeMagnets(0, 1);
        QCOMPARE(update_queue_spy.count(), 2);
        QCOMPARE(mman.count(), 0);
    }

    void testAddSameMagnetTwice()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};

        mman.addMagnet(mlink1, default_options, false);
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);

        mman.update();
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);

        mman.addMagnet(mlink1, default_options, false);
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);

        mman.removeMagnets(0, 1);
        QCOMPARE(update_queue_spy.count(), 2);
        QCOMPARE(mman.count(), 0);
    }

    void testAddAndRemoveTwoMagnets()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};

        mman.addMagnet(mlink1, default_options, false);
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);

        mman.update();
        QCOMPARE(update_queue_spy.count(), 1);
        QCOMPARE(mman.count(), 1);

        mman.addMagnet(mlink2, default_options, false);
        QCOMPARE(update_queue_spy.count(), 2);
        QCOMPARE(mman.count(), 2);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);
        QCOMPARE(mman.getMagnetDownloader(1)->magnetLink(), mlink2);

        mman.removeMagnets(0, 1);
        QCOMPARE(update_queue_spy.count(), 3);
        QCOMPARE(mman.count(), 1);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink2);

        mman.removeMagnets(0, 1);
        QCOMPARE(update_queue_spy.count(), 4);
        QCOMPARE(mman.count(), 0);
    }

    void testRemoveMultipleMagnetsAtOnce()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};

        mman.addMagnet(mlink1, default_options, false);
        mman.addMagnet(mlink2, default_options, false);
        mman.addMagnet(mlink3, default_options, false);
        mman.addMagnet(mlink4, default_options, false);
        mman.addMagnet(mlink5, default_options, false);
        QCOMPARE(update_queue_spy.count(), 5);
        QCOMPARE(mman.count(), 5);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);
        QCOMPARE(mman.getMagnetDownloader(1)->magnetLink(), mlink2);
        QCOMPARE(mman.getMagnetDownloader(2)->magnetLink(), mlink3);
        QCOMPARE(mman.getMagnetDownloader(3)->magnetLink(), mlink4);
        QCOMPARE(mman.getMagnetDownloader(4)->magnetLink(), mlink5);

        mman.update();
        QCOMPARE(update_queue_spy.count(), 5);
        QCOMPARE(mman.count(), 5);

        mman.removeMagnets(2, 2);
        QCOMPARE(update_queue_spy.count(), 6);
        QCOMPARE(mman.count(), 3);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);
        QCOMPARE(mman.getMagnetDownloader(1)->magnetLink(), mlink2);
        QCOMPARE(mman.getMagnetDownloader(2)->magnetLink(), mlink5);

        mman.removeMagnets(0, 3);
        QCOMPARE(update_queue_spy.count(), 7);
        QCOMPARE(mman.count(), 0);
    }

    void testMetadataDownloaded()
    {
        MagnetManager mman;
        QSignalSpy update_queue_spy{&mman, &MagnetManager::updateQueue};
        QSignalSpy metadata_downloaded_spy{&mman, &MagnetManager::metadataDownloaded};

        mman.addMagnet(mlink1, default_options, false);
        mman.addMagnet(mlink2, default_options, false);
        mman.addMagnet(mlink3, default_options, false);
        mman.addMagnet(mlink4, default_options, false);
        mman.addMagnet(mlink5, default_options, false);
        QCOMPARE(update_queue_spy.count(), 5);
        QCOMPARE(mman.count(), 5);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);
        QCOMPARE(mman.getMagnetDownloader(1)->magnetLink(), mlink2);
        QCOMPARE(mman.getMagnetDownloader(2)->magnetLink(), mlink3);
        QCOMPARE(mman.getMagnetDownloader(3)->magnetLink(), mlink4);
        QCOMPARE(mman.getMagnetDownloader(4)->magnetLink(), mlink5);

        mman.update();
        QCOMPARE(update_queue_spy.count(), 5);
        QCOMPARE(mman.count(), 5);
        QCOMPARE(metadata_downloaded_spy.count(), 0);

        const QByteArray metadata = "abcde"_ba;
        auto md = const_cast<kt::MagnetDownloader *>(mman.getMagnetDownloader(2));
        Q_EMIT md->foundMetadata(md, metadata);

        QCOMPARE(metadata_downloaded_spy.count(), 1);
        QCOMPARE(metadata_downloaded_spy.at(0).count(), 3);
        QCOMPARE(metadata_downloaded_spy.at(0).at(0).value<bt::MagnetLink>(), mlink3);
        QCOMPARE(metadata_downloaded_spy.at(0).at(1).toByteArray(), metadata);
        const auto opts = metadata_downloaded_spy.at(0).at(2).value<kt::MagnetLinkLoadOptions>();
        QCOMPARE(opts.silently, default_options.silently);
        QCOMPARE(opts.group, default_options.group);
        QCOMPARE(opts.location, default_options.location);
        QCOMPARE(opts.move_on_completion, default_options.move_on_completion);

        QCOMPARE(update_queue_spy.count(), 6);
        QCOMPARE(mman.count(), 4);
        QCOMPARE(mman.getMagnetDownloader(0)->magnetLink(), mlink1);
        QCOMPARE(mman.getMagnetDownloader(1)->magnetLink(), mlink2);
        QCOMPARE(mman.getMagnetDownloader(2)->magnetLink(), mlink4);
        QCOMPARE(mman.getMagnetDownloader(3)->magnetLink(), mlink5);
    }
};
}

QTEST_GUILESS_MAIN(kt::MagnetManagerTests)

#include "magnetmanagertest.moc"
