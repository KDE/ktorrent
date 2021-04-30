/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSEARCHENGINE_H
#define KTSEARCHENGINE_H

#include <QIcon>
#include <QObject>
#include <QUrl>

class KJob;

namespace kt
{
/**
    Keeps track of a search engine
*/
class SearchEngine : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor, sets the data dir
     * @param data_dir Directory where all the information regarding the engine is stored
     */
    SearchEngine(const QString &data_dir);
    ~SearchEngine() override;

    /**
     * Load the engine from an opensearch XML file
     * @param xml_file Local XML file
     * @return true upon success
     */
    bool load(const QString &xml_file);

    /**
     * Fill in search terms into the search url and create the QUrl to use
     * @param terms Tersm to search for
     * @return The url
     */
    QUrl search(const QString &terms);

    /// Get the name of the engine
    QString engineName() const
    {
        return name;
    }

    /// Get the icon
    QIcon engineIcon() const
    {
        return icon;
    }

    /// Get the engine directory
    QString engineDir() const
    {
        return data_dir;
    }

    /// Get the URL
    QString engineUrl() const
    {
        return url;
    }

    /// Get the description
    QString engineDescription() const
    {
        return description;
    }

private Q_SLOTS:
    void iconDownloadFinished(KJob *job);

private:
    QString data_dir;
    QString name;
    QString description;
    QString url;
    QString icon_url;
    QIcon icon;

    friend class OpenSearchHandler;
};

}

#endif
