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
enum class SearchEngineType {
    OpenSearch,
    Torznab,
};

struct TorznabEngineConfig {
    QString name;
    QString description;
    QUrl serviceUrl;
    QString apiKey;
    bool trackerFirst = false;
    int threadCount = 20;
};

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

    static QString torznabConfigFileName();

    /**
     * Load the engine from a descriptor file
     * @param descriptor_file Either an OpenSearch XML file or a Torznab JSON file
     * @return true upon success
     */
    bool load(const QString &descriptor_file);

    /**
     * Fill in search terms into the search url and create the QUrl to use
     * @param terms Tersm to search for
     * @return The url
     */
    QUrl search(const QString &terms) const;

    SearchEngineType engineType() const
    {
        return type;
    }

    bool isTorznab() const
    {
        return type == SearchEngineType::Torznab;
    }

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

    const TorznabEngineConfig &torznabConfig() const
    {
        return torznab_config;
    }

    static bool writeTorznabConfig(const QString &config_path, const TorznabEngineConfig &config, QString *error_message = nullptr);

private Q_SLOTS:
    void iconDownloadFinished(KJob *job);

private:
    bool loadOpenSearch(const QString &xml_file);
    bool loadTorznab(const QString &json_file);
    static TorznabEngineConfig normalizedTorznabConfig(const TorznabEngineConfig &config);

    QString data_dir;
    QString name;
    QString description;
    QString url;
    QString icon_url;
    QIcon icon;
    SearchEngineType type = SearchEngineType::OpenSearch;
    TorznabEngineConfig torznab_config;

    friend class OpenSearchHandler;
};

}

#endif
