/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#ifndef KTTORRENTFILETREEMODEL_H
#define KTTORRENTFILETREEMODEL_H

#include "torrentfilemodel.h"
#include <util/bitset.h>

class QSortFilterProxyModel;

namespace bt
{
    class BEncoder;
    class BNode;
}

namespace kt
{

    /**
     * Model for displaying file trees of a torrent
     * @author Joris Guisson
    */
    class KTCORE_EXPORT TorrentFileTreeModel : public TorrentFileModel
    {
        Q_OBJECT
    protected:
        struct KTCORE_EXPORT Node
        {
            Node* parent;
            bt::TorrentFileInterface* file; // file (0 if this is a directory)
            QString name; // name or directory
            QList<Node*> children; // child dirs
            bt::Uint64 size;
            bt::BitSet chunks;
            bool chunks_set;
            float percentage;

            Node(Node* parent, bt::TorrentFileInterface* file, const QString& name,
                 bt::Uint32 total_chunks);
            Node(Node* parent, const QString& name, bt::Uint32 total_chunks);
            ~Node();

            void insert(const QString& path, bt::TorrentFileInterface* file, bt::Uint32 num_chunks);
            int row();
            bt::Uint64 fileSize(const bt::TorrentInterface* tc);
            bt::Uint64 bytesToDownload(const bt::TorrentInterface* tc);
            Qt::CheckState checkState(const bt::TorrentInterface* tc) const;
            QString path();
            void fillChunks();
            void updatePercentage(const bt::BitSet& havechunks);
            void initPercentage(const bt::TorrentInterface* tc, const bt::BitSet& havechunks);

            void saveExpandedState(const QModelIndex& index, QSortFilterProxyModel* pm, QTreeView* tv, bt::BEncoder* enc);
            void loadExpandedState(const QModelIndex& index, QSortFilterProxyModel* pm, QTreeView* tv, bt::BNode* node);
        };
    public:
        TorrentFileTreeModel(bt::TorrentInterface* tc, DeselectMode mode, QObject* parent);
        ~TorrentFileTreeModel();

        void changeTorrent(bt::TorrentInterface* tc) override;
        int rowCount(const QModelIndex& parent) const override;
        int columnCount(const QModelIndex& parent) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
        QVariant data(const QModelIndex& index, int role) const override;
        QModelIndex parent(const QModelIndex& index) const override;
        QModelIndex index(int row, int column, const QModelIndex& parent) const override;
        bool setData(const QModelIndex& index, const QVariant& value, int role) override;
        void checkAll() override;
        void uncheckAll() override;
        void invertCheck() override;
        bt::Uint64 bytesToDownload() override;
        QByteArray saveExpandedState(QSortFilterProxyModel* pm, QTreeView* tv) override;
        void loadExpandedState(QSortFilterProxyModel* pm, QTreeView* tv, const QByteArray& state) override;
        bt::TorrentFileInterface* indexToFile(const QModelIndex& idx) override;
        QString dirPath(const QModelIndex& idx) override;
        void changePriority(const QModelIndexList& indexes, bt::Priority newpriority) override;
        void onCodecChange() override;

    private:
        void constructTree();
        void invertCheck(const QModelIndex& idx);
        bool setCheckState(const QModelIndex& index, Qt::CheckState state);
        bool setName(const QModelIndex& index, const QString& name);
        void modifyPathOfFiles(Node* n, const QString& path);


    protected:
        Node* root;
        bool emit_check_state_change;
    };

}

#endif
