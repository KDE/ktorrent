/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KT_SHUTDOWNTORRENTMODEL_H
#define KT_SHUTDOWNTORRENTMODEL_H

#include "shutdownruleset.h"
#include <QAbstractItemModel>
#include <QStyledItemDelegate>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class QueueManager;
class CoreInterface;
class ShutdownRuleSet;

class ShutdownTorrentDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ShutdownTorrentDelegate(QObject *parent = nullptr);
    ~ShutdownTorrentDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ShutdownTorrentModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ShutdownTorrentModel(CoreInterface *core, QObject *parent);
    ~ShutdownTorrentModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
        Apply the current model to the rules
    */
    void applyRules(Action action, ShutdownRuleSet *rules);

    /**
        Add an existing rule to the model
    */
    void addRule(const ShutdownRule &rule);

private Q_SLOTS:
    void torrentAdded(bt::TorrentInterface *tc);
    void torrentRemoved(bt::TorrentInterface *tc);

private:
    struct TriggerItem {
        bt::TorrentInterface *tc;
        bool checked;
        Trigger trigger;
    };

    QueueManager *qman;
    QList<TriggerItem> conds;
};

}

#endif // KT_SHUTDOWNTORRENTMODEL_H
