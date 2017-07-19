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

#include <QMimeDatabase>
#include <QMimeType>

#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <KZip>

#include <util/log.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/decompressfilejob.h>
#include <util/extractfilejob.h>
#include <interfaces/functions.h>
#include "convertdialog.h"
#include "downloadandconvertjob.h"

using namespace bt;


namespace kt
{

    DownloadAndConvertJob::DownloadAndConvertJob(const QUrl& url, Mode mode)
        : url(url), unzip(false), convert_dlg(nullptr), mode(mode)
    {
    }


    DownloadAndConvertJob::~DownloadAndConvertJob()
    {
    }

    void DownloadAndConvertJob::start()
    {
        QString temp = kt::DataDir() + QStringLiteral("tmp-") + url.fileName();
        if (bt::Exists(temp))
            bt::Delete(temp, true);

        active_job = KIO::file_copy(url, QUrl::fromLocalFile(temp), -1, KIO::Overwrite);
        connect(active_job, &KJob::result, this, &DownloadAndConvertJob::downloadFileFinished);
    }

    void DownloadAndConvertJob::kill(KJob::KillVerbosity)
    {
        if (active_job)
            active_job->kill(KJob::EmitResult);
        else if (convert_dlg)
            convert_dlg->reject();
    }

    void DownloadAndConvertJob::convert(KJob* j)
    {
        active_job = nullptr;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                j->uiDelegate()->showErrorMessage();
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: %1", j->errorString());
                notification(msg);
            }
            setError(unzip ? UNZIP_FAILED : MOVE_FAILED);
            emitResult();
        }
        else
            convert();
    }

    static bool isBinaryData(const QString &fileName)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;    // err, whatever
        }
        // Check the first 32 bytes (see shared-mime spec)
        const QByteArray data = file.read(32);
        const char *p = data.data();
        for (int i = 0; i < data.size(); ++i) {
            if ((unsigned char)(p[i]) < 32 && p[i] != 9 && p[i] != 10 && p[i] != 13) { // ASCII control character
                return true;
            }
        }
        return false;
    }

    void DownloadAndConvertJob::downloadFileFinished(KJob* j)
    {
        active_job = nullptr;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                j->uiDelegate()->showErrorMessage();
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: %1", j->errorString());
                notification(msg);
            }

            setError(DOWNLOAD_FAILED);
            emitResult();
            return;
        }

        QString temp = kt::DataDir() + QStringLiteral("tmp-") + url.fileName();

        //now determine if it's ZIP or TXT file
        QMimeDatabase db;
        QMimeType ptr = db.mimeTypeForFile(temp, QMimeDatabase::MatchContent);
        Out(SYS_IPF|LOG_NOTICE) << "Mimetype: " << ptr.name() << endl;
        if(ptr.name() == QStringLiteral("application/zip"))
        {
            active_job = KIO::file_move(QUrl::fromLocalFile(temp), QUrl::fromLocalFile(QString(kt::DataDir() + QLatin1String("level1.zip"))), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, &KJob::result, this, &DownloadAndConvertJob::extract);
        }
        else if(ptr.name() == QStringLiteral("application/x-7z-compressed"))
        {
            QString msg = i18n("7z files are not supported", url.toDisplayString());
            if (mode == Verbose)
                KMessageBox::error(nullptr, msg);
            else
                notification(msg);
            
            setError(UNZIP_FAILED);
            emitResult();
        }
        else if(ptr.name() == QStringLiteral("application/gzip") || ptr.name() == QStringLiteral("application/x-bzip"))
        {
            active_job = new bt::DecompressFileJob(temp, kt::DataDir() + QStringLiteral("level1.txt"));
            connect(active_job, &KJob::result, this, static_cast<void (DownloadAndConvertJob::*)(KJob*)>(&DownloadAndConvertJob::convert));
            active_job->start();
        }
        else if(!isBinaryData(temp) || ptr.name() == QStringLiteral("text/plain"))
        {
            active_job = KIO::file_move(QUrl::fromLocalFile(temp), QUrl::fromLocalFile(kt::DataDir() + QStringLiteral("level1.txt")), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, &KJob::result, this, static_cast<void (DownloadAndConvertJob::*)(KJob*)>(&DownloadAndConvertJob::convert));
        }
        else
        {
            QString msg = i18n("Cannot determine file type of <b>%1</b>", url.toDisplayString());
            if (mode == Verbose)
                KMessageBox::error(nullptr, msg);
            else
                notification(msg);

            setError(UNZIP_FAILED);
            emitResult();
        }
    }

    void DownloadAndConvertJob::extract(KJob* j)
    {
        active_job = nullptr;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                j->uiDelegate()->showErrorMessage();
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: %1", j->errorString());
                notification(msg);
            }
            setError(MOVE_FAILED);
            emitResult();
            return;
        }

        QString zipfile = kt::DataDir() + QStringLiteral("level1.zip");
        KZip* zip = new KZip(zipfile);
        if (!zip->open(QIODevice::ReadOnly) || !zip->directory())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: cannot open zip file " << zipfile << endl;
            if (mode == Verbose)
            {
                KMessageBox::error(nullptr, i18n("Cannot open zip file %1.", zipfile));
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: cannot open zip file %1", zipfile);
                notification(msg);
            }

            setError(UNZIP_FAILED);
            emitResult();
            delete zip;
            return;
        }

        QString destination = kt::DataDir() + QStringLiteral("level1.txt");
        QStringList entries = zip->directory()->entries();
        if(entries.count() >= 1)
        {
            active_job = new bt::ExtractFileJob(zip, entries.front(), destination);
            connect(active_job, &KJob::result, this, static_cast<void (DownloadAndConvertJob::*)(KJob*)>(&DownloadAndConvertJob::convert));
            unzip = true;
            active_job->start();
        }
        else
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: no blocklist found in zipfile " << zipfile << endl;
            if (mode == Verbose)
            {
                KMessageBox::error(nullptr, i18n("Cannot find blocklist in zip file %1.", zipfile));
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: cannot find blocklist in zip file %1", zipfile);
                notification(msg);
            }

            setError(UNZIP_FAILED);
            emitResult();
            delete zip;
        }
    }

    void DownloadAndConvertJob::revertBackupFinished(KJob*)
    {
        active_job = 0;
        cleanUpFiles();
        setError(CANCELED);
        emitResult();
    }

    void DownloadAndConvertJob::makeBackupFinished(KJob* j)
    {
        if (j && j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                j->uiDelegate()->showErrorMessage();
            }
            else
            {
                QString msg = i18n("Automatic update of IP filter failed: %1", j->errorString());
                notification(msg);
            }
            setError(BACKUP_FAILED);
            emitResult();
        }
        else
        {
            convert_dlg = new ConvertDialog(nullptr);
            if (mode == Verbose)
                convert_dlg->show();
            connect(convert_dlg, &ConvertDialog::accepted, this, &DownloadAndConvertJob::convertAccepted);
            connect(convert_dlg, &ConvertDialog::rejected, this, &DownloadAndConvertJob::convertRejected);
        }
    }

    void DownloadAndConvertJob::convertAccepted()
    {
        convert_dlg->deleteLater();
        convert_dlg = nullptr;
        cleanUpFiles();
        setError(0);
        emitResult();
    }

    void DownloadAndConvertJob::convertRejected()
    {
        convert_dlg->deleteLater();
        convert_dlg = nullptr;
        // shit happened move back backup stuff
        QString dat_file = kt::DataDir() + QStringLiteral("level1.dat");
        QString tmp_file = kt::DataDir() + QStringLiteral("level1.dat.tmp");

        if (bt::Exists(tmp_file))
        {
            active_job = KIO::file_copy(QUrl::fromLocalFile(tmp_file), QUrl::fromLocalFile(dat_file), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, &KJob::result, this, &DownloadAndConvertJob::revertBackupFinished);
        }
        else
        {
            cleanUpFiles();
            setError(CANCELED);
            emitResult();
        }
    }

    void DownloadAndConvertJob::convert()
    {
        if (bt::Exists(kt::DataDir() + QStringLiteral("level1.dat")))
        {
            // make backup of data file, if stuff fails we can always go back
            QString dat_file = kt::DataDir() + QStringLiteral("level1.dat");
            QString tmp_file = kt::DataDir() + QStringLiteral("level1.dat.tmp");


            KIO::Job* job = KIO::file_copy(QUrl::fromLocalFile(dat_file), QUrl::fromLocalFile(tmp_file), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, &KIO::Job::result, this, &DownloadAndConvertJob::makeBackupFinished);
        }
        else
            makeBackupFinished(nullptr);
    }

    void DownloadAndConvertJob::cleanUpFiles()
    {
        // cleanup temp files
        cleanUp(kt::DataDir() + QStringLiteral("level1.zip"));
        cleanUp(kt::DataDir() + QStringLiteral("level1.txt"));
        cleanUp(kt::DataDir() + QStringLiteral("level1.tmp"));
        cleanUp(kt::DataDir() + QStringLiteral("level1.dat.tmp"));
    }

    void DownloadAndConvertJob::cleanUp(const QString& path)
    {
        if (bt::Exists(path))
            bt::Delete(path, true);
    }
}
