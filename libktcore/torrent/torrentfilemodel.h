/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KTTORRENTFILEMODEL_HH

#define KTTORRENTFILEMODEL_HH

#include <QAbstractItemModel>
#include <QByteArray>

#include <ktcore_export.h>
#include <util/constants.h>

class QTreeView;
class QSortFilterProxyModel;

namespace bt
{
class TorrentInterface;
class TorrentFileInterface;
}

namespace kt
{
class KTCORE_EXPORT TorrentFileModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum DeselectMode {
        KEEP_FILES,
        DELETE_FILES,
    };
    TorrentFileModel(bt::TorrentInterface *tc, DeselectMode mode, QObject *parent);
    ~TorrentFileModel() override;

    /**
     * Change the current torrent
     */
    virtual void changeTorrent(bt::TorrentInterface *tc) = 0;

    /**
     * Check all the files in the torrent.
     */
    virtual void checkAll() = 0;

    /**
     * Uncheck all files in the torrent.
     */
    virtual void uncheckAll() = 0;

    /**
     * Invert the check of each file of the torrent
     */
    virtual void invertCheck() = 0;

    /**
     * Calculate the number of bytes to download
     * @return Bytes to download
     */
    virtual bt::Uint64 bytesToDownload() = 0;

    /**
     * Save which items are expanded.
     * @param pm Proxy model of the view
     * @param tv The QTreeView
     * @return The expanded state encoded in a byte array
     */
    virtual QByteArray saveExpandedState(QSortFilterProxyModel *pm, QTreeView *tv);

    /**
     * Retore the expanded state of the tree.in a QTreeView
     * @param pm Proxy model of the view
     * @param tv The QTreeView
     * @param state The encoded expanded state
     */
    virtual void loadExpandedState(QSortFilterProxyModel *pm, QTreeView *tv, const QByteArray &state);

    /**
     * Convert a model index to a file.
     * @param idx The model index
     * @return The file index or 0 for a directory
     **/
    virtual bt::TorrentFileInterface *indexToFile(const QModelIndex &idx) = 0;

    /**
     * Get the path of a directory (root directory not included)
     * @param idx The model index
     * @return The path
     */
    virtual QString dirPath(const QModelIndex &idx) = 0;

    /**
     * Change the priority of a bunch of items.
     * @param indexes The list of items
     * @param newpriority The new priority
     */
    virtual void changePriority(const QModelIndexList &indexes, bt::Priority newpriority) = 0;

    /**
     * Missing files have been marked DND, update the preview and selection information.
     */
    virtual void missingFilesMarkedDND();

    /**
     * Update gui if necessary
     */
    virtual void update();

    /**
     * Codec has changed, so update the model.
     */
    virtual void onCodecChange();

    /// Set the file names editable
    void setFileNamesEditable(bool on)
    {
        file_names_editable = on;
    }

    /// Are the file names editable
    bool fileNamesEditable() const
    {
        return file_names_editable;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    virtual void filePercentageChanged(bt::TorrentFileInterface *file, float percentage);
    virtual void filePreviewChanged(bt::TorrentFileInterface *file, bool preview);
Q_SIGNALS:
    /**
     * Emitted whenever one or more items changes check state
     */
    void checkStateChanged();

protected:
    bt::TorrentInterface *tc;
    DeselectMode mode;
    bool file_names_editable;
};
}

#endif
