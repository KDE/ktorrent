/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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

#include <QComboBox>
#include <KLocalizedString>

#include <util/log.h>
#include "logflagsdelegate.h"

namespace kt
{

    LogFlagsDelegate::LogFlagsDelegate(QObject* parent)
        : QItemDelegate(parent)
    {
    }


    LogFlagsDelegate::~LogFlagsDelegate()
    {
    }

    QWidget* LogFlagsDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
    {
        Q_UNUSED(index);
        QComboBox* editor = new QComboBox(parent);
        editor->addItem(i18n("All"));
        editor->addItem(i18n("Important"));
        editor->addItem(i18n("Notice"));
        editor->addItem(i18n("Debug"));
        editor->addItem(i18n("None"));
        return editor;
    }

    void LogFlagsDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        bt::Uint32 value = index.model()->data(index, Qt::EditRole).toUInt();
        QComboBox* cb = static_cast<QComboBox*>(editor);
        switch (value)
        {
        case LOG_DEBUG: cb->setCurrentIndex(3); break;
        case LOG_NOTICE: cb->setCurrentIndex(2); break;
        case LOG_IMPORTANT: cb->setCurrentIndex(1); break;
        case LOG_ALL: cb->setCurrentIndex(0); break;
        case LOG_NONE: cb->setCurrentIndex(4); break;
        }
    }

    void LogFlagsDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        QComboBox* cb = static_cast<QComboBox*>(editor);
        switch (cb->currentIndex())
        {
        case 0: model->setData(index, LOG_ALL, Qt::EditRole); break;
        case 1: model->setData(index, LOG_IMPORTANT, Qt::EditRole); break;
        case 2: model->setData(index, LOG_NOTICE, Qt::EditRole); break;
        case 3: model->setData(index, LOG_DEBUG, Qt::EditRole); break;
        case 4: model->setData(index, LOG_NONE, Qt::EditRole); break;
        }
    }

    QSize LogFlagsDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);

        QComboBox tmp;
        return QSize(100, tmp.sizeHint().height());
    }
}
