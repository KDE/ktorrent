/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDOWNLOADANDCONVERTJOB_H
#define KTDOWNLOADANDCONVERTJOB_H

#include <KIO/Job>

namespace kt
{
class ConvertDialog;

/**
    Job to download and convert a filter file
*/
class DownloadAndConvertJob : public KIO::Job
{
    Q_OBJECT
public:
    enum Mode {
        Verbose,
        Quietly,
    };
    DownloadAndConvertJob(const QUrl &url, Mode mode);
    ~DownloadAndConvertJob() override;

    enum ErrorCode {
        CANCELED = 100,
        DOWNLOAD_FAILED,
        UNZIP_FAILED,
        MOVE_FAILED,
        BACKUP_FAILED,
    };

    void kill(KJob::KillVerbosity v);
    void start() override;

    bool isAutoUpdate() const
    {
        return mode == Quietly;
    }

Q_SIGNALS:
    /// Emitted when the job needs to show a notification
    void notification(const QString &msg);

private Q_SLOTS:
    void downloadFileFinished(KJob *);
    void convert(KJob *);
    void extract(KJob *);
    void makeBackupFinished(KJob *);
    void revertBackupFinished(KJob *);
    void convertAccepted();
    void convertRejected();

private:
    void convert();
    void cleanUp(const QString &path);
    void cleanUpFiles();

private:
    QUrl url;
    KJob *active_job;
    bool unzip;
    ConvertDialog *convert_dlg;
    Mode mode;
};

}

#endif
