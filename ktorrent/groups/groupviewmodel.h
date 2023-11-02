/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_GROUPVIEWMODEL_H
#define KT_GROUPVIEWMODEL_H

#include <QAbstractItemModel>
#include <QStringList>

namespace kt
{

class Group;
class GroupManager;
class TorrentGroup;

/**
 * Model for the GroupView
 **/
class GroupViewModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum ColumnRoles { PathRole = Qt::UserRole + 1 };

    GroupViewModel(GroupManager *gman, QObject *parent);
    ~GroupViewModel() override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QStringList mimeTypes() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    /// Get the group given an index
    Group *groupForIndex(const QModelIndex &index) const;

    /// Update the group count
    void updateGroupCount(const QModelIndex &idx);

Q_SIGNALS:
    void addTorrentSelectionToGroup(TorrentGroup *g);

private Q_SLOTS:
    void groupAdded(Group *g);
    void groupRemoved(Group *g);

private:
    struct Item {
        Item(const QString &name, Item *parent, int row, GroupViewModel *model);

        void insert(const QString &name, const QString &p, const QModelIndex &idx);
        void insert(Group *g, const QModelIndex &idx);
        void remove(Group *g, const QModelIndex &idx);
        bool operator==(const QString &n) const;
        QVariant displayData();
        QVariant decoration();
        QString path() const;
        void dump();
        QModelIndex findGroup(Group *g, const QModelIndex &idx);

        QString name;
        QString display_name;
        Item *parent;
        int row;
        Group *group;
        QList<Item> children;
        GroupViewModel *model;
    };

    QModelIndex findGroup(Group *g);

private:
    Item root;
    GroupManager *gman;
};

}

#endif // KT_GROUPVIEWMODEL_H
