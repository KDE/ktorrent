/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#include "groupviewmodel.h"
#include "groupview.h"

#include <QIcon>
#include <KLocalizedString>

#include <groups/groupmanager.h>
#include <groups/group.h>
#include <groups/torrentgroup.h>
#include <torrent/queuemanager.h>
#include <view/view.h>
#include <util/log.h>

using namespace bt;


namespace kt
{
    GroupViewModel::GroupViewModel(kt::GroupManager* gman, View* view, QObject* parent) :
        QAbstractItemModel(parent),
        root(QStringLiteral("all"), nullptr, 0, this),
        gman(gman),
        view(view)
    {
        for (GroupManager::Itr i = gman->begin(); i != gman->end(); i++)
            root.insert(i->second, index(0, 0));

        root.insert(i18n("Custom Groups"), QStringLiteral("/all/custom"), index(0, 0));
        //root.dump();

        connect(gman, &GroupManager::groupRemoved, this, &GroupViewModel::groupRemoved);
        connect(gman, &GroupManager::groupAdded, this, &GroupViewModel::groupAdded);
    }

    GroupViewModel::~GroupViewModel()
    {
    }

    QVariant GroupViewModel::data(const QModelIndex& index, int role) const
    {
        Item* item = (Item*)index.internalPointer();
        if (!item)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
            return item->displayData();
        case Qt::DecorationRole:
            return item->decoration();
        }

        return QVariant();
    }

    bool GroupViewModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (role != Qt::EditRole)
            return false;

        Item* item = (Item*)index.internalPointer();
        if (!item)
            return false;

        Group* group = item->group;
        QString new_name = value.toString();
        if (new_name.isEmpty() || gman->find(new_name))
            return false;

        item->name = new_name;
        gman->renameGroup(group->groupName(), new_name);
        dataChanged(index, index);
        return true;
    }


    int GroupViewModel::columnCount(const QModelIndex& parent) const
    {
        Q_UNUSED(parent);
        return 1;
    }

    int GroupViewModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return 1;

        Item* item = (Item*)parent.internalPointer();
        if (!item)
            return 0;
        else
            return item->children.size();
    }

    QModelIndex GroupViewModel::parent(const QModelIndex& child) const
    {
        Item* item = (Item*)child.internalPointer();
        if (!item || !item->parent)
            return QModelIndex();
        else
            return createIndex(item->parent->row, 0, (void*)item->parent);
    }

    QModelIndex GroupViewModel::index(int row, int column, const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return createIndex(row, column, (void*)&root);

        Item* item = (Item*)parent.internalPointer();
        if (!item || row < 0 || row >= item->children.count())
            return QModelIndex();

        return createIndex(row, column, (void*)&item->children.at(row));
    }

    bool GroupViewModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
    {
        Q_UNUSED(data);
        Q_UNUSED(action);
        if (row != -1 || column != -1)
            return false;

        TorrentGroup* g = dynamic_cast<TorrentGroup*>(groupForIndex(parent));
        if (!g)
            return false;

        QList<TorrentInterface*> sel;
        view->getSelection(sel);
        foreach (TorrentInterface* ti, sel)
        {
            g->addTorrent(ti, false);
        }
        gman->saveGroups();
        return true;
    }

    Qt::DropActions GroupViewModel::supportedDropActions() const
    {
        return Qt::CopyAction;
    }

    QStringList GroupViewModel::mimeTypes() const
    {
        QStringList sl;
        sl << QStringLiteral("application/x-ktorrent-drag-object");
        return sl;
    }


    Qt::ItemFlags GroupViewModel::flags(const QModelIndex& index) const
    {
        Item* item = (Item*)index.internalPointer();
        if (item && item->group && !item->group->isStandardGroup())
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
        else
            return Qt::ItemIsEnabled;
    }

    void GroupViewModel::groupAdded(Group* g)
    {
        root.insert(g, index(0, 0));
    }

    void GroupViewModel::groupRemoved(Group* g)
    {
        QModelIndex idx = findGroup(g).parent();
        root.remove(g, index(0, 0));
        //root.dump();
        view->onGroupRemoved(g);
    }

    Group* GroupViewModel::groupForIndex(const QModelIndex& index) const
    {
        Item* item = (Item*)index.internalPointer();
        return item ? item->group : 0;
    }

    QModelIndex GroupViewModel::findGroup(Group* g)
    {
        QModelIndex idx = index(0, 0);
        return root.findGroup(g, idx);
    }

    QStringList GroupViewModel::expandedGroups(GroupView* gview)
    {
        QStringList ret;
        QModelIndex idx = createIndex(0, 0, &root);
        root.expandedGroups(gview, ret, idx);
        return ret;
    }

    void GroupViewModel::expandGroups(GroupView* gview, const QStringList& groups)
    {
        QModelIndex idx = createIndex(0, 0, &root);
        root.expandGroups(gview, groups, idx);
    }

    void GroupViewModel::updateGroupCount(const QModelIndex& idx)
    {
        int row = 0;
        QModelIndex child = idx.child(row, 0);
        while (child.isValid())
        {
            updateGroupCount(child);
            row++;
            child = idx.child(row, 0);
        }

        dataChanged(idx, idx);
    }

    bool GroupViewModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Item* item = (Item*)parent.internalPointer();
        if (!item)
            return false;

        beginRemoveRows(parent, row, row + count);
        for (int i = 0; i < count; i++)
            item->children.removeAt(row);
        int row_index = 0;
        for (QList<Item>::iterator j = item->children.begin(); j != item->children.end(); j++)
            j->row = row_index++;
        endRemoveRows();
        return true;
    }


    /////////////////////////////////////////::

    GroupViewModel::Item::Item(const QString& name, kt::GroupViewModel::Item* parent, int row, kt::GroupViewModel* model) :
        name(name),
        display_name(name),
        parent(parent),
        row(row),
        group(nullptr),
        model(model)
    {

    }

    void GroupViewModel::Item::insert(Group* g, const QModelIndex& idx)
    {
        QString group_path = g->groupPath();
        QString item_path = path();
        if (!group_path.startsWith(item_path))
            return;

        QString remainder = group_path.remove(0, item_path.size());
        if (remainder.isEmpty())
        {
            group = g;
        }
        else
        {
            QString child_name;
            if (remainder.indexOf(QLatin1Char('/')) == -1)
                child_name = remainder;
            else
                child_name = remainder.section(QLatin1Char('/'), 1, 1);

            QList<Item>::iterator i = qFind(children.begin(), children.end(), child_name);
            if (i == children.end())
            {
                int npos = children.count();
                model->beginInsertRows(idx, npos, npos);
                children.append(Item(child_name, this, npos, model));
                children.last().insert(g, idx.child(npos, 0));
                model->endInsertRows();
            }
            else
                i->insert(g, idx.child(i->row, 0));
        }
    }

    void GroupViewModel::Item::insert(const QString& name, const QString& p, const QModelIndex& idx)
    {
        QString item_path = path();
        if (!p.startsWith(item_path))
            return;

        QString tmp = p;
        QString remainder = tmp.remove(0, item_path.size());
        if (remainder.isEmpty())
        {
            display_name = name;
        }
        else
        {
            QString child_name;
            if (remainder.indexOf(QLatin1Char('/')) == -1)
                child_name = remainder;
            else
                child_name = remainder.section(QLatin1Char('/'), 1, 1);

            QList<Item>::iterator i = qFind(children.begin(), children.end(), child_name);
            if (i == children.end())
            {
                int npos = children.count();
                model->beginInsertRows(idx, npos, npos);
                children.append(Item(child_name, this, npos, model));
                children.last().insert(name, p, idx.child(npos, 0));
                model->endInsertRows();
            }
            else
                i->insert(name, p, idx.child(i->row, 0));
        }
    }

    void GroupViewModel::Item::remove(kt::Group* g, const QModelIndex& idx)
    {
        QString group_path = g->groupPath();
        QString item_path = path();
        if (!group_path.startsWith(item_path))
            return;

        QString remainder = group_path.remove(0, item_path.size());
        if (remainder.count(QLatin1Char('/')) == 1)
        {
            QList<Item>::iterator i = qFind(children.begin(), children.end(), remainder.mid(1));
            if (i != children.end())
            {
                model->removeRows(i->row, 1, idx);
            }
        }
        else
        {
            QString child_name = remainder.section(QLatin1Char('/'), 1, 1);
            QList<Item>::iterator i = qFind(children.begin(), children.end(), child_name);
            if (i != children.end())
                i->remove(g, idx.child(i->row, 0));
        }
    }


    bool GroupViewModel::Item::operator==(const QString& n) const
    {
        return name == n;
    }

    QVariant GroupViewModel::Item::displayData()
    {
        if (group)
            return QStringLiteral("%1 (%2/%3)").arg(group->groupName()).arg(group->runningTorrents()).arg(group->totalTorrents());
        else
            return display_name;
    }

    QVariant GroupViewModel::Item::decoration()
    {
        if (group)
            return group->groupIcon();
        else
            return QIcon::fromTheme(QStringLiteral("folder"));
    }

    QString GroupViewModel::Item::path() const
    {
        if (!parent)
            return QLatin1Char('/') + name;
        else
            return parent->path() + QLatin1Char('/') + name;
    }


    void GroupViewModel::Item::expandedGroups(GroupView* gview, QStringList& groups, const QModelIndex& idx) const
    {
        if (children.empty())
            return;

        if (gview->isExpanded(idx))
            groups << path();

        int row = 0;
        foreach (const Item& child, children)
        {
            child.expandedGroups(gview, groups, idx.child(row, 0));
            row++;
        }
    }

    void GroupViewModel::Item::expandGroups(kt::GroupView* gview, const QStringList& groups, const QModelIndex& idx)
    {
        if (children.empty())
            return;

        if (groups.contains(path()))
            gview->expand(idx);

        int row = 0;
        for (QList<Item>::iterator i = children.begin(); i != children.end(); i++)
        {
            i->expandGroups(gview, groups, idx.child(row, 0));
            row++;
        }
    }

    QModelIndex GroupViewModel::Item::findGroup(Group* g, const QModelIndex& idx)
    {
        if (group == g)
            return idx;

        int row = 0;
        for (QList<Item>::iterator i = children.begin(); i != children.end(); i++)
        {
            QModelIndex ret = i->findGroup(g, idx.child(row, 0));
            row++;
            if (ret.isValid())
                return ret;
        }

        return QModelIndex();
    }

    void GroupViewModel::Item::dump()
    {
        QString p = path();
        int indentation = p.count(QLatin1Char('/')) - 1;
        QString indent = QStringLiteral("\t").repeated(indentation);
        Out(SYS_GEN | LOG_DEBUG) << indent <<  path() << endl;
        if (group)
            Out(SYS_GEN | LOG_DEBUG) << indent << group->groupName()  << endl;
        else
            Out(SYS_GEN | LOG_DEBUG) << indent << name << endl;

        for (QList<Item>::iterator i = children.begin(); i != children.end(); i++)
        {
            Out(SYS_GEN | LOG_DEBUG) << indent << "child " << i->row << endl;
            i->dump();
        }
    }


}

