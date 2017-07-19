/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <QUrl>

#include <KGuiItem>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <util/log.h>
#include <util/error.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include <torrent/torrent.h>
#include <diskio/chunkmanager.h>
#include <interfaces/coreinterface.h>
#include <datachecker/datacheckerthread.h>
#include <datachecker/singledatachecker.h>
#include <datachecker/multidatachecker.h>
#include <interfaces/functions.h>
#include <settings.h>
#include "importdialog.h"


using namespace bt;

namespace kt
{
    ImportDialog::ImportDialog(CoreInterface* core, QWidget* parent)
        : QDialog(parent), core(core), dc(nullptr), dc_thread(nullptr), canceled(false)
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setupUi(this);
        KUrlRequester* r = m_torrent_url;
        r->setMode(KFile::File | KFile::LocalOnly);
        r->setFilter(kt::TorrentFileFilter(true));

        r = m_data_url;
        r->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);

        connect(m_import_btn, &QPushButton::clicked, this, &ImportDialog::onImport);
        connect(m_cancel_btn, &QPushButton::clicked, this, &ImportDialog::cancelImport);
        m_progress->setEnabled(false);
        m_progress->setValue(0);
        KGuiItem::assign(m_cancel_btn, KStandardGuiItem::cancel());
        m_import_btn->setIcon(QIcon::fromTheme(QStringLiteral("document-import")));
    }

    ImportDialog::~ImportDialog()
    {}

    void ImportDialog::progress(quint32 num, quint32 total)
    {
        m_progress->setMaximum(total);
        m_progress->setValue(num);
    }

    void ImportDialog::finished()
    {
        QString data_dir = m_data_url->url().toLocalFile();
        QUrl tor_url = m_torrent_url->url();
        if (canceled || !dc_thread->getError().isEmpty())
        {
            if (!canceled)
                KMessageBox::error(this, dc_thread->getError());
            dc_thread->deleteLater();
            dc_thread = nullptr;
            reject();
            return;
        }

        // find a new torrent dir and make it if necessary
        QString tor_dir = core->findNewTorrentDir();
        if (!tor_dir.endsWith(bt::DirSeparator()))
            tor_dir += bt::DirSeparator();

        try
        {
            if (!bt::Exists(tor_dir))
                bt::MakeDir(tor_dir);

            // write the index file
            writeIndex(tor_dir + QStringLiteral("index"), dc->getResult());

            // copy the torrent file
            bt::CopyFile(tor_url.url(), tor_dir + QStringLiteral("torrent"));

            Uint64 imported = calcImportedBytes(dc->getResult(), tor);

            // make the cache
            if (tor.isMultiFile())
            {
                QList<Uint32> dnd_files;

                // first make tor_dir/dnd
                QString dnd_dir = tor_dir + QStringLiteral("dnd") + bt::DirSeparator();
                if (!bt::Exists(dnd_dir))
                    MakeDir(dnd_dir);

                if (!data_dir.endsWith(bt::DirSeparator()))
                    data_dir += bt::DirSeparator();

                for (Uint32 i = 0; i < tor.getNumFiles(); i++)
                {
                    TorrentFile& tf = tor.getFile(i);
                    makeDirs(dnd_dir, data_dir, tf.getPath());
                    tf.setPathOnDisk(data_dir + tf.getPath());
                }

                saveFileMap(tor, tor_dir);

                QString durl = data_dir;
                //if (durl.endsWith(bt::DirSeparator()))
                durl.chop(1);
                int ds = durl.lastIndexOf(bt::DirSeparator());
                if (durl.midRef(ds + 1) == tor.getNameSuggestion())
                {
                    durl.truncate(ds);
                    saveStats(tor_dir + QStringLiteral("stats"), durl, imported, false);
                }
                else
                {
                    saveStats(tor_dir + QStringLiteral("stats"), durl, imported, true);
                }
                saveFileInfo(tor_dir + QStringLiteral("file_info"), dnd_files);
            }
            else
            {
                // single file, just symlink the data_url to tor_dir/cache
                QString durl = data_dir;
                int ds = durl.lastIndexOf(bt::DirSeparator());
                durl.truncate(ds);
                saveStats(tor_dir + QStringLiteral("stats"), durl, imported, false);
                saveFileMap(tor_dir, data_dir);
            }

            // everything went OK, so load the whole shabang and start downloading
            core->loadExistingTorrent(tor_dir);
        }
        catch (Error& e)
        {
            // delete tor_dir
            bt::Delete(tor_dir, true);
            KMessageBox::error(this, e.toString());
            dc_thread->deleteLater();
            dc_thread = nullptr;
            reject();
            return;
        }

        dc_thread->deleteLater();
        dc_thread = nullptr;
        accept();
    }

    void ImportDialog::import()
    {
        // get the urls
        QUrl tor_url = m_torrent_url->url();
        QUrl data_url = m_data_url->url();

        // now we need to check the data
        if (tor.isMultiFile())
        {
            dc = new MultiDataChecker(0, tor.getNumChunks());
            QString path = data_url.toLocalFile();
            if (!path.endsWith(bt::DirSeparator()))
                path += bt::DirSeparator();

            for (Uint32 i = 0; i < tor.getNumFiles(); i++)
            {
                bt::TorrentFile& tf = tor.getFile(i);
                tf.setPathOnDisk(path + tf.getPath());
            }
        }
        else
            dc = new SingleDataChecker(0, tor.getNumChunks());

        connect(dc, &bt::DataChecker::progress, this, &ImportDialog::progress, Qt::QueuedConnection);

        BitSet bs(tor.getNumChunks());
        bs.setAll(false);
        dc_thread = new DataCheckerThread(dc, bs, data_url.toLocalFile(), tor, QString::null);
        connect(dc_thread, &bt::DataCheckerThread::finished, this, &ImportDialog::finished, Qt::QueuedConnection);
        dc_thread->start();
    }

    void ImportDialog::onTorrentGetReult(KJob* j)
    {
        if (j->error())
        {
            j->uiDelegate()->showErrorMessage();
            reject();
        }
        else
        {
            // try to load the torrent
            try
            {
                KIO::StoredTransferJob* stj = (KIO::StoredTransferJob*)j;
                tor.load(stj->data(), false);
            }
            catch (Error& e)
            {
                KMessageBox::error(this, i18n("Cannot load the torrent file: %1", e.toString()));
                reject();
                return;
            }
            import();
        }
    }

    void ImportDialog::onImport()
    {
        m_progress->setEnabled(true);
        m_import_btn->setEnabled(false);
        m_cancel_btn->setEnabled(false);
        m_torrent_url->setEnabled(false);
        m_data_url->setEnabled(false);

        QUrl tor_url = m_torrent_url->url();
        if (!tor_url.isLocalFile())
        {
            // download the torrent file
            KIO::StoredTransferJob* j = KIO::storedGet(tor_url);
            connect(j, &KIO::StoredTransferJob::result, this, &ImportDialog::onTorrentGetReult);
        }
        else
        {
            // try to load the torrent
            try
            {
                tor.load(bt::LoadFile(tor_url.toLocalFile()), false);
            }
            catch (Error& e)
            {
                KMessageBox::error(this, i18n("Cannot load the torrent file: %1", e.toString()));
                reject();
                return;
            }
            import();
        }
    }

    void ImportDialog::cancelImport()
    {
        if (dc_thread)
        {
            canceled = true;
            dc->stop();
            dc_thread->wait();
            dc_thread->deleteLater();
            dc_thread = nullptr;
        }

        reject();
    }


    void ImportDialog::writeIndex(const QString& file, const BitSet& chunks)
    {
        // first try to open it
        File fptr;
        if (!fptr.open(file, QStringLiteral("wb")))
            throw Error(i18n("Cannot open %1: %2", file, fptr.errorString()));

        // write all chunks to the file
        for (Uint32 i = 0; i < chunks.getNumBits(); i++)
        {
            if (!chunks.get(i))
                continue;

            // we have the chunk so write a NewChunkHeader struct to the file
            NewChunkHeader hdr;
            hdr.index = i;
            hdr.deprecated = 0;
            fptr.write(&hdr, sizeof(NewChunkHeader));
        }
    }

    void ImportDialog::makeDirs(const QString& dnd_dir, const QString &data_url, const QString& fpath)
    {
        QStringList sl = fpath.split(bt::DirSeparator());

        // create all necessary subdirs
        QString otmp = data_url;
        //if (!otmp.endsWith(bt::DirSeparator()))
        //    otmp += bt::DirSeparator();

        QString dtmp = dnd_dir;
        for (int i = 0; i < sl.count() - 1; i++)
        {
            otmp += sl[i];
            dtmp += sl[i];
            if (!bt::Exists(otmp))
                MakeDir(otmp);
            if (!bt::Exists(dtmp))
                MakeDir(dtmp);
            otmp += bt::DirSeparator();
            dtmp += bt::DirSeparator();
        }
    }

    void ImportDialog::saveStats(const QString& stats_file, const QString &data_dir, Uint64 imported, bool custom_output_name)
    {
        QFile fptr(stats_file);
        if (!fptr.open(QIODevice::WriteOnly))
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Warning : can't create stats file" << endl;
            return;
        }

        QTextStream out(&fptr);
        out << "OUTPUTDIR=" << data_dir << ::endl;
        out << "UPLOADED=0" << ::endl;
        out << "RUNNING_TIME_DL=0" << ::endl;
        out << "RUNNING_TIME_UL=0" << ::endl;
        out << "PRIORITY=0" << ::endl;
        out << "AUTOSTART=1" << ::endl;
        if (Settings::maxRatio() > 0)
            out << QStringLiteral("MAX_RATIO=%1").arg(Settings::maxRatio(), 0, 'f', 2) << ::endl;
        out << QStringLiteral("IMPORTED=%1").arg(imported) << ::endl;
        if (custom_output_name)
            out << "CUSTOM_OUTPUT_NAME=1" << endl;
    }

    Uint64 ImportDialog::calcImportedBytes(const bt::BitSet& chunks, const Torrent& tor)
    {
        Uint64 nb = 0;
        Uint64 ls = tor.getLastChunkSize();

        for (Uint32 i = 0; i < chunks.getNumBits(); i++)
        {
            if (!chunks.get(i))
                continue;

            if (i == chunks.getNumBits() - 1)
                nb += ls;
            else
                nb += tor.getChunkSize();
        }
        return nb;
    }

    void ImportDialog::saveFileInfo(const QString& file_info_file, QList<Uint32> & dnd)
    {
        // saves which TorrentFile's do not need to be downloaded
        File fptr;
        if (!fptr.open(file_info_file, QStringLiteral("wb")))
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Warning : Can't save chunk_info file : " << fptr.errorString() << endl;
            return;
        }

        ;

        // first write the number of excluded ones
        Uint32 tmp = dnd.count();
        fptr.write(&tmp, sizeof(Uint32));
        // then all the excluded ones
        for (int i = 0; i < dnd.count(); i++)
        {
            tmp = dnd[i];
            fptr.write(&tmp, sizeof(Uint32));
        }
        fptr.flush();
    }

    void ImportDialog::saveFileMap(const Torrent& tor, const QString& tor_dir)
    {
        QString file_map = tor_dir + QLatin1String("file_map");
        QFile fptr(file_map);
        if (!fptr.open(QIODevice::WriteOnly))
            throw Error(i18n("Failed to create %1: %2", file_map, fptr.errorString()));

        QTextStream out(&fptr);

        Uint32 num = tor.getNumFiles();
        for (Uint32 i = 0; i < num; i++)
        {
            const TorrentFile& tf = tor.getFile(i);
            out << tf.getPathOnDisk() << ::endl;
        }
    }

    void ImportDialog::saveFileMap(const QString& tor_dir, const QString& ddir)
    {
        QString file_map = tor_dir + QLatin1String("file_map");
        QFile fptr(file_map);
        if (!fptr.open(QIODevice::WriteOnly))
            throw Error(i18n("Failed to create %1: %2", file_map, fptr.errorString()));

        QTextStream out(&fptr);
        out << ddir << ::endl;
    }

}


