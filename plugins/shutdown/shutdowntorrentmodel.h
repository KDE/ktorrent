/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KT_SHUTDOWNTORRENTMODEL_H
#define KT_SHUTDOWNTORRENTMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include "shutdownruleset.h"

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
		ShutdownTorrentDelegate(QObject *parent = 0);
		virtual ~ShutdownTorrentDelegate();
		
		virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option,const QModelIndex &index) const;
		virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
		virtual void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
		virtual void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index) const;
		virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
	};
	

	class ShutdownTorrentModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		ShutdownTorrentModel(CoreInterface* core,QObject* parent);
		virtual ~ShutdownTorrentModel();
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
		
		/**
			Apply the current model to the rules
		*/
		void applyRules(Action action,ShutdownRuleSet* rules);
		
		/**
			Add an existing rule to the model
		*/
		void addRule(const ShutdownRule & rule);
		
	private slots:
		void torrentAdded(bt::TorrentInterface* tc);
		void torrentRemoved(bt::TorrentInterface* tc);
		
	private:
		
		struct TriggerItem
		{
			bt::TorrentInterface* tc;
			bool checked;
			Trigger trigger;
		};
		
		QueueManager* qman;
		QList<TriggerItem> conds;
	};

}

#endif // KT_SHUTDOWNTORRENTMODEL_H
