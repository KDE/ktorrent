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

#include <QModelIndex>
#include <QSpinBox>

#include <KLocalizedString>

#include "spinboxdelegate.h"

namespace kt
{

    SpinBoxDelegate::SpinBoxDelegate(QObject* parent) : QItemDelegate(parent)
    {}


    SpinBoxDelegate::~SpinBoxDelegate()
    {}

    QWidget* SpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
    {
        QSpinBox* editor = new QSpinBox(parent);
        editor->setSuffix(i18n(" KiB/s"));
        if (index.column() < 3)
            editor->setSpecialValueText(i18n("No limit"));
        else
            editor->setSpecialValueText(i18n("No assured speed"));
        editor->setMinimum(0);
        editor->setMaximum(10000000);
        return editor;
    }

    void SpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void SpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
    {
        QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }

    void SpinBoxDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex&) const
    {
        QRect r = option.rect;
        if (option.rect.height() < editor->sizeHint().height())
            r.setHeight(editor->sizeHint().height());
        editor->setGeometry(r);
    }

    QSize SpinBoxDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        return QSpinBox().sizeHint();
    }
}

