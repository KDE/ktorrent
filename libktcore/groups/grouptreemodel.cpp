/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "grouptreemodel.h"

#include <algorithm>
#include <iterator>

#include <KLocalizedString>
#include <QIcon>

#include <groups/group.h>
#include <groups/groupmanager.h>
#include <groups/torrentgroup.h>
#include <torrent/queuemanager.h>
#include <util/log.h>
using namespace bt;

namespace kt
{
GroupTreeModel::GroupTreeModel(kt::GroupManager *gman, QObject *parent)
    : QAbstractItemModel(parent)
    , root(QStringLiteral("all"), nullptr, 0, this)
    , gman(gman)
{
    for (GroupManager::CItr i = gman->begin(); i != gman->end(); i++)
        root.insert(i->second, index(0, 0));

    root.insert(i18n("Custom Groups"), QStringLiteral("/all/custom"), index(0, 0));
    // root.dump();

    connect(gman, &GroupManager::groupRemoved, this, &GroupTreeModel::groupRemoved);
    connect(gman, &GroupManager::groupAdded, this, &GroupTreeModel::groupAdded);
}

GroupTreeModel::~GroupTreeModel()
{
}

QVariant GroupTreeModel::data(const QModelIndex &index, int role) const
{
    Item *item = (Item *)index.internalPointer();
    if (!item)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
        return item->displayData();
    case Qt::DecorationRole:
        return item->decoration();
    case PathRole:
        return item->path();
    }

    return QVariant();
}

bool GroupTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    Item *item = (Item *)index.internalPointer();
    if (!item)
        return false;

    Group *group = item->group;
    QString new_name = value.toString();
    if (new_name.isEmpty() || gman->find(new_name))
        return false;

    item->name = new_name;
    gman->renameGroup(group->groupName(), new_name);
    Q_EMIT dataChanged(index, index);
    return true;
}

int GroupTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int GroupTreeModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 1;

    Item *item = (Item *)parent.internalPointer();
    if (!item)
        return 0;
    else
        return item->children.size();
}

QModelIndex GroupTreeModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) {
        return QModelIndex();
    }

    Item *item = (Item *)child.internalPointer();
    if (!item || !item->parent)
        return QModelIndex();
    else
        return createIndex(item->parent->row, 0, (void *)item->parent);
}

QModelIndex GroupTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid())
        return createIndex(row, column, (void *)&root);

    Item *item = (Item *)parent.internalPointer();
    if (!item || row < 0 || row >= static_cast<int>(item->children.size()))
        return QModelIndex();

    auto childItr = std::find_if(item->children.begin(), item->children.end(), [=](const auto &child) {
        return child.row == row;
    });

    if (childItr == item->children.end()) {
        return QModelIndex{};
    }

    return createIndex(row, column, (void *)&*childItr);
}

bool GroupTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(data);
    Q_UNUSED(action);
    if (row != -1 || column != -1)
        return false;

    TorrentGroup *g = dynamic_cast<TorrentGroup *>(groupForIndex(parent));
    if (!g)
        return false;

    Q_EMIT addTorrentSelectionToGroup(g);
    return true;
}

Qt::DropActions GroupTreeModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

QStringList GroupTreeModel::mimeTypes() const
{
    QStringList sl;
    sl << QStringLiteral("application/x-ktorrent-drag-object");
    return sl;
}

Qt::ItemFlags GroupTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Item *item = (Item *)index.internalPointer();
    if (item && item->group && !item->group->isStandardGroup())
        return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    else
        return Qt::ItemIsEnabled;
}

void GroupTreeModel::groupAdded(Group *g)
{
    root.insert(g, index(0, 0));
}

void GroupTreeModel::groupRemoved(Group *g)
{
    //        QModelIndex idx = findGroup(g).parent();
    root.remove(g, index(0, 0));
    // root.dump();
}

Group *GroupTreeModel::groupForIndex(const QModelIndex &index) const
{
    Item *item = (Item *)index.internalPointer();
    return item ? item->group : nullptr;
}

QModelIndex GroupTreeModel::findGroup(Group *g)
{
    QModelIndex idx = index(0, 0);
    return root.findGroup(g, idx);
}

void GroupTreeModel::updateGroupCount(const QModelIndex &idx)
{
    int row = 0;
    QModelIndex child = this->index(row, 0, idx);
    while (child.isValid()) {
        updateGroupCount(child);
        row++;
        child = this->index(row, 0, idx);
    }

    Q_EMIT dataChanged(idx, idx);
}

bool GroupTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Item *item = (Item *)parent.internalPointer();
    if (!item)
        return false;

    const int lastIndex = row + count - 1;
    if (row < 0 || lastIndex >= static_cast<int>(item->children.size()) || count < 1) {
        return false;
    }

    const auto firstIt = std::next(item->children.cbegin(), row);
    const auto lastIt = std::next(firstIt, count);
    beginRemoveRows(parent, row, lastIndex);
    item->children.erase(firstIt, lastIt);
    int row_index = 0;
    for (Item &i : item->children)
        i.row = row_index++;
    endRemoveRows();
    return true;
}

/////////////////////////////////////////::

GroupTreeModel::Item::Item(const QString &name, kt::GroupTreeModel::Item *parent, int row, kt::GroupTreeModel *model)
    : name(name)
    , display_name(name)
    , parent(parent)
    , row(row)
    , group(nullptr)
    , model(model)
{
}

void GroupTreeModel::Item::insert(Group *g, const QModelIndex &idx)
{
    QString group_path = g->groupPath();
    QString item_path = path();
    if (!group_path.startsWith(item_path))
        return;

    QString remainder = group_path.remove(0, item_path.size());
    if (remainder.isEmpty()) {
        group = g;
    } else {
        QString child_name;
        if (remainder.indexOf(QLatin1Char('/')) == -1)
            child_name = remainder;
        else
            child_name = remainder.section(QLatin1Char('/'), 1, 1);

        auto i = std::find(children.begin(), children.end(), child_name);
        if (i == children.end()) {
            int npos = children.size();
            model->beginInsertRows(idx, npos, npos);
            children.emplace_back(child_name, this, npos, model);
            children.back().insert(g, model->index(npos, 0, idx));
            model->endInsertRows();
        } else
            i->insert(g, model->index(i->row, 0, idx));
    }
}

void GroupTreeModel::Item::insert(const QString &name, const QString &p, const QModelIndex &idx)
{
    QString item_path = path();
    if (!p.startsWith(item_path))
        return;

    QString tmp = p;
    QString remainder = tmp.remove(0, item_path.size());
    if (remainder.isEmpty()) {
        display_name = name;
    } else {
        QString child_name;
        if (remainder.indexOf(QLatin1Char('/')) == -1)
            child_name = remainder;
        else
            child_name = remainder.section(QLatin1Char('/'), 1, 1);

        auto i = std::find(children.begin(), children.end(), child_name);
        if (i == children.end()) {
            int npos = children.size();
            model->beginInsertRows(idx, npos, npos);
            children.emplace_back(child_name, this, npos, model);
            children.back().insert(name, p, model->index(npos, 0, idx));
            model->endInsertRows();
        } else
            i->insert(name, p, model->index(i->row, 0, idx));
    }
}

void GroupTreeModel::Item::remove(kt::Group *g, const QModelIndex &idx)
{
    QString group_path = g->groupPath();
    QString item_path = path();
    if (!group_path.startsWith(item_path))
        return;

    QString remainder = group_path.remove(0, item_path.size());
    if (remainder.count(QLatin1Char('/')) == 1) {
        auto i = std::find(children.begin(), children.end(), remainder.mid(1));
        if (i != children.end()) {
            model->removeRows(i->row, 1, idx);
        }
    } else {
        QString child_name = remainder.section(QLatin1Char('/'), 1, 1);
        auto i = std::find(children.begin(), children.end(), child_name);
        if (i != children.end())
            i->remove(g, model->index(i->row, 0, idx));
    }
}

bool GroupTreeModel::Item::operator==(const QString &n) const
{
    return name == n;
}

QVariant GroupTreeModel::Item::displayData()
{
    if (group)
        return QStringLiteral("%1 (%2/%3)").arg(group->groupName()).arg(group->runningTorrents()).arg(group->totalTorrents());
    else
        return display_name;
}

QVariant GroupTreeModel::Item::decoration()
{
    if (group)
        return group->groupIcon();
    else
        return QIcon::fromTheme(QStringLiteral("folder"));
}

QString GroupTreeModel::Item::path() const
{
    if (!parent)
        return QLatin1Char('/') + name;
    else
        return parent->path() + QLatin1Char('/') + name;
}

QModelIndex GroupTreeModel::Item::findGroup(Group *g, const QModelIndex &idx)
{
    if (group == g)
        return idx;

    int row = 0;
    for (Item &i : children) {
        QModelIndex ret = i.findGroup(g, model->index(row, 0, idx));
        row++;
        if (ret.isValid())
            return ret;
    }

    return QModelIndex();
}

void GroupTreeModel::Item::dump()
{
    QString p = path();
    int indentation = p.count(QLatin1Char('/')) - 1;
    QString indent = QStringLiteral("\t").repeated(indentation);
    Out(SYS_GEN | LOG_DEBUG) << indent << path() << endl;
    if (group)
        Out(SYS_GEN | LOG_DEBUG) << indent << group->groupName() << endl;
    else
        Out(SYS_GEN | LOG_DEBUG) << indent << name << endl;

    for (Item &i : children) {
        Out(SYS_GEN | LOG_DEBUG) << indent << "child " << i.row << endl;
        i.dump();
    }
}

}

#include "moc_grouptreemodel.cpp"
