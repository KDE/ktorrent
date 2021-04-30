/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDRETRIEVER_H
#define KTFEEDRETRIEVER_H

#include <QFile>
#include <Syndication/DataRetriever>

class KJob;

namespace kt
{
/**
    Class which downloads a feed and also saves a backup copy.
*/
class FeedRetriever : public Syndication::DataRetriever
{
public:
    /// Constructor, does not save a backup copy
    FeedRetriever();

    /// Constructor, does save a backup copy
    FeedRetriever(const QString &file_name);

    ~FeedRetriever();

    /// Set the authentication cookie
    void setAuthenticationCookie(const QString &cookie);

    void abort() override;
    int errorCode() const override;
    void retrieveData(const QUrl &url) override;

    void finished(KJob *j);

private:
    QString backup_file;
    KJob *job;
    int err;
    QString cookie;
};

}

#endif
