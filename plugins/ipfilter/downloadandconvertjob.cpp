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

// [Fonic] Porting from KMimeType to QMimeType; reference: plugin syndication, file linkdownloader.cpp
//#include <kmimetype.h>
#include <QMimeType>
#include <QMimeDatabase>

#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <kzip.h>
#include <kio/jobuidelegate.h>
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
        : url(url), unzip(false), convert_dlg(0), mode(mode)
    {
    }


    DownloadAndConvertJob::~DownloadAndConvertJob()
    {
    }

    void DownloadAndConvertJob::start()
    {
        // [Fonic] url.fileName() doesn't work for the default URL (will return ""),
        // so we hard-code this for now...
        //QString temp = kt::DataDir() + "tmp-" + url.fileName();
        QString temp = kt::DataDir() + "temp-ipfilter-download.gz";
        if (bt::Exists(temp))
            bt::Delete(temp, true);

        // [Fonic] Changed API:
        // FileCopyJob *KIO::file_copy(const QUrl &src, const QUrl &dest, int permissions, JobFlags flags)
        // -> QString needs to be converted to QUrl:
        // QUrl(const QString &url, ParsingMode parsingMode = TolerantMode)
        // QUrl QUrl::fromLocalFile(const QString &localFile)
        //active_job = KIO::file_copy(url, temp, -1, KIO::Overwrite);
        active_job = KIO::file_copy(url, QUrl::fromLocalFile(temp), -1, KIO::Overwrite);
        connect(active_job, SIGNAL(result(KJob*)), this, SLOT(downloadFileFinished(KJob*)));
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
        active_job = 0;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                ((KIO::Job*)j)->ui()->showErrorMessage();
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

    void DownloadAndConvertJob::downloadFileFinished(KJob* j)
    {
        active_job = 0;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                ((KIO::Job*)j)->ui()->showErrorMessage();
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

        //
        // [Fonic]
        // - Porting from KMimeType to QMimeType; reference: plugin syndication, file linkdownloader.cpp
        // - KIO::file_* operations require QUrl as argument for both source and destination
        //
        
        // [Fonic] Same as in DownloadAndConvertJob::start()
        //QString temp = kt::DataDir() + "tmp-" + url.fileName();
        QString temp = kt::DataDir() + "temp-ipfilter-download.gz";

        // [Fonic]
        /*//now determine if it's ZIP or TXT file
        KMimeType::Ptr ptr = KMimeType::findByFileContent(temp);
        Out(SYS_IPF|LOG_NOTICE) << "Mimetype: " << ptr->name() << endl;
        if(ptr->name() == "application/zip")
        {
            active_job = KIO::file_move(temp, QString(kt::DataDir() + QLatin1String("level1.zip")), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(extract(KJob*)));
        }
        else if(ptr->name() == "application/x-7z-compressed")
        {
            QString msg = i18n("7z files are not supported", url.prettyUrl());
            if (mode == Verbose)
                KMessageBox::error(0, msg);
            else
                notification(msg);
            
            setError(UNZIP_FAILED);
            emitResult();
        }
        else if(ptr->name() == "application/gzip" || ptr->name() == "application/x-bzip")
        {
            active_job = new bt::DecompressFileJob(temp, QString(kt::DataDir() + "level1.txt"));
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(convert(KJob*)));
            active_job->start();
        }
        else if(!KMimeType::isBinaryData(temp) || ptr->name() == "text/plain")
        {
            active_job = KIO::file_move(temp, QString(kt::DataDir() + "level1.txt"), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(convert(KJob*)));
        }
        else
        {
            QString msg = i18n("Cannot determine file type of <b>%1</b>", url.prettyUrl());
            if (mode == Verbose)
                KMessageBox::error(0, msg);
            else
                notification(msg);

            setError(UNZIP_FAILED);
            emitResult();
        }*/
        
        // [Fonic]
        //now determine if it's ZIP or TXT file
        QMimeType file_type = QMimeDatabase().mimeTypeForFile(temp);
        Out(SYS_IPF|LOG_NOTICE) << "Mimetype: " << file_type.name() << endl;
        if (file_type.name() == "application/zip")
        {
            // [Fonic] Changed API (similar to KIO::file_copy):
            // FileCopyJob *KIO::file_move(const QUrl &src, const QUrl &dest, int permissions, JobFlags flags)
            //active_job = KIO::file_move(temp, QString(kt::DataDir() + QLatin1String("level1.zip")), -1, KIO::HideProgressInfo | KIO::Overwrite);
            active_job = KIO::file_move(QUrl::fromLocalFile(temp), QUrl::fromLocalFile(QString(kt::DataDir() + QLatin1String("level1.zip"))), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(extract(KJob*)));
        }
        else if (file_type.name() == "application/x-7z-compressed")
        {
            // [Fonic] QUrl.toString defaults to formatting 'PrettyDecoded'
            //QString msg = i18n("7z files are not supported", url.prettyUrl());
            QString msg = i18n("7z files are not supported", url.toString());
            if (mode == Verbose)
                KMessageBox::error(0, msg);
            else
                notification(msg);
            setError(UNZIP_FAILED);
            emitResult();
        }
        else if (file_type.name() == "application/gzip" || file_type.name() == "application/x-bzip")
        {
            active_job = new bt::DecompressFileJob(temp, QString(kt::DataDir() + "level1.txt"));
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(convert(KJob*)));
            active_job->start();
        }
        // [Fonic] TODO: find out if check for binary data is supported by QMimeType
        //else if(!KMimeType::isBinaryData(temp) || file_type.name() == "text/plain")
        else if (file_type.name() == "text/plain")
        {
            // [Fonic] Same as above
            //active_job = KIO::file_move(temp, QString(kt::DataDir() + "level1.txt"), -1, KIO::HideProgressInfo | KIO::Overwrite);
            active_job = KIO::file_move(QUrl::fromLocalFile(temp), QUrl::fromLocalFile(QString(kt::DataDir() + "level1.txt")), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(convert(KJob*)));
        }
        else
        {
            // [Fonic] Same as above
            //QString msg = i18n("Cannot determine file type of <b>%1</b>", url.prettyUrl());
            QString msg = i18n("Cannot determine file type of <b>%1</b>", url.toString());
            if (mode == Verbose)
                KMessageBox::error(0, msg);
            else
                notification(msg);
            setError(UNZIP_FAILED);
            emitResult();
        }
        
    }

    void DownloadAndConvertJob::extract(KJob* j)
    {
        active_job = 0;
        if (j->error())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: " << j->errorString() << endl;
            if (mode == Verbose)
            {
                ((KIO::Job*)j)->ui()->showErrorMessage();
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

        QString zipfile = kt::DataDir() + "level1.zip";
        KZip* zip = new KZip(zipfile);
        if (!zip->open(QIODevice::ReadOnly) || !zip->directory())
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: cannot open zip file " << zipfile << endl;
            if (mode == Verbose)
            {
                KMessageBox::error(0, i18n("Cannot open zip file %1.", zipfile));
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

        QString destination = kt::DataDir() + "level1.txt";
        QStringList entries = zip->directory()->entries();
        if(entries.count() >= 1)
        {
            active_job = new bt::ExtractFileJob(zip, entries.front(), destination);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(convert(KJob*)));
            unzip = true;
            active_job->start();
        }
        else
        {
            Out(SYS_IPF | LOG_NOTICE) << "IP filter update failed: no blocklist found in zipfile " << zipfile << endl;
            if (mode == Verbose)
            {
                KMessageBox::error(0, i18n("Cannot find blocklist in zip file %1.", zipfile));
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
                ((KIO::Job*)j)->ui()->showErrorMessage();
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
            convert_dlg = new ConvertDialog(0);
            if (mode == Verbose)
                convert_dlg->show();
            connect(convert_dlg, SIGNAL(accepted()), this, SLOT(convertAccepted()));
            connect(convert_dlg, SIGNAL(rejected()), this, SLOT(convertRejected()));
        }
    }

    void DownloadAndConvertJob::convertAccepted()
    {
        convert_dlg->deleteLater();
        convert_dlg = 0;
        cleanUpFiles();
        setError(0);
        emitResult();
    }

    void DownloadAndConvertJob::convertRejected()
    {
        convert_dlg->deleteLater();
        convert_dlg = 0;
        // shit happened move back backup stuff
        QString dat_file = kt::DataDir() + "level1.dat";
        QString tmp_file = kt::DataDir() + "level1.dat.tmp";

        if (bt::Exists(tmp_file))
        {
            // [Fonic] Same as above
            //active_job = KIO::file_copy(tmp_file, dat_file, -1, KIO::HideProgressInfo | KIO::Overwrite);
            active_job = KIO::file_copy(QUrl::fromLocalFile(tmp_file), QUrl::fromLocalFile(dat_file), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(active_job, SIGNAL(result(KJob*)), this, SLOT(revertBackupFinished(KJob*)));
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
        if (bt::Exists(kt::DataDir() + "level1.dat"))
        {
            // make backup of data file, if stuff fails we can always go back
            QString dat_file = kt::DataDir() + "level1.dat";
            QString tmp_file = kt::DataDir() + "level1.dat.tmp";

            // [Fonic] Same as above
            //KIO::Job* job = KIO::file_copy(dat_file, tmp_file, -1, KIO::HideProgressInfo | KIO::Overwrite);
            KIO::Job* job = KIO::file_copy(QUrl::fromLocalFile(dat_file), QUrl::fromLocalFile(tmp_file), -1, KIO::HideProgressInfo | KIO::Overwrite);
            connect(job, SIGNAL(result(KJob*)), this, SLOT(makeBackupFinished(KJob*)));
        }
        else
            makeBackupFinished(0);
    }

    void DownloadAndConvertJob::cleanUpFiles()
    {
        // cleanup temp files
        cleanUp(kt::DataDir() + "level1.zip");
        cleanUp(kt::DataDir() + "level1.txt");
        cleanUp(kt::DataDir() + "level1.tmp");
        cleanUp(kt::DataDir() + "level1.dat.tmp");
    }

    void DownloadAndConvertJob::cleanUp(const QString& path)
    {
        if (bt::Exists(path))
            bt::Delete(path, true);
    }
}
