/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSEARCHENGINELIST_H
#define KTSEARCHENGINELIST_H

#include "proxy_helper.h"
#include "searchengine.h"
#include <QAbstractListModel>
#include <QList>
#include <util/constants.h>

namespace kt
{
class OpenSearchDownloadJob;

/**
    @author Joris Guisson <joris.guisson@gmail.com>
*/
class SearchEngineList : public QAbstractListModel
{
    Q_OBJECT

    QList<SearchEngine *> engines;
    QList<QUrl> default_opensearch_urls;
    QList<QUrl> default_urls;
    ProxyHelper *m_proxy;
    QString data_dir;

public:
    SearchEngineList(ProxyHelper *proxy, const QString &data_dir);
    ~SearchEngineList() override;

    /// Load all engines
    void loadEngines();

    /// Search with an engine
    QUrl search(bt::Uint32 engine, const QString &terms);

    /// Get an engine by index
    SearchEngine *engine(bt::Uint32 index) const;

    /// Get the name of an engine
    QString getEngineName(bt::Uint32 engine) const;

    /// Get the number of engines
    bt::Uint32 getNumEngines() const
    {
        return engines.count();
    }

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;

    /**
     * Remove all engines in a list
     * @param sel The list
     */
    void removeEngines(const QModelIndexList &sel);

    /**
     * Remove all engines
     */
    void removeAllEngines();

    /**
     * Add all defaults engines (if they are not added yet)
     */
    void addDefaults();

    /**
     * Add an engine from an OpenSearchDownloadJob
     * @param j The OpenSearchDownloadJob
     */
    void addEngine(OpenSearchDownloadJob *j);

    /**
     * Add an engine from a search URL
     * @param dir The directory to use
     * @param url The url
     */
    void addEngine(const QString &dir, const QString &url);

    /**
     * Add a Jackett/Torznab engine from a configuration object
     * @param dir The directory to use
     * @param config Engine configuration
     * @param error_message Receives a user-visible error on failure
     * @return true if the engine was added
     */
    bool addTorznabEngine(const QString &dir, const TorznabEngineConfig &config, QString *error_message = nullptr);

    /**
     * Update an existing Jackett/Torznab engine
     * @param index Model index of the engine
     * @param config Replacement configuration
     * @param error_message Receives a user-visible error on failure
     * @return true if the engine was updated
     */
    bool updateTorznabEngine(const QModelIndex &index, const TorznabEngineConfig &config, QString *error_message = nullptr);

private:
    void convertSearchEnginesFile();
    void loadDefault(bool removed_to);
    bool alreadyLoaded(const QString &user_dir);
    void loadEngine(const QString &global_dir, const QString &user_dir, bool load_removed);

private Q_SLOTS:
    void openSearchDownloadJobFinished(KJob *j);
};

}

#endif
