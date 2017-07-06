/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
*   joris.guisson@gmail.com                                               *
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

#ifndef KT_SCRIPTDELEGATE_H
#define KT_SCRIPTDELEGATE_H

#include <QAbstractItemView>
#include <QCheckBox>
#include <QPushButton>

#include <KWidgetItemDelegate>

namespace kt
{

    class ScriptDelegate : public KWidgetItemDelegate
    {
        Q_OBJECT
    public:
        ScriptDelegate(QAbstractItemView* parent);
        ~ScriptDelegate();

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
        QList<QWidget*> createItemWidgets(const QModelIndex& index) const override;
        void updateItemWidgets(const QList<QWidget*> widgets, const QStyleOptionViewItem& option, const QPersistentModelIndex& index) const override;

    private:
        QFont titleFont(const QFont& baseFont) const;

    private slots:
        void toggled(bool on);
        void aboutClicked();
        void settingsClicked();

    private:
        QCheckBox* check_box;
        QPushButton* push_button;
    };

}

#endif // KT_SCRIPTDELEGATE_H
