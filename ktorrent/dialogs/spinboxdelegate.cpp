/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QModelIndex>
#include <QSpinBox>

#include <KLocalizedString>

#include "spinboxdelegate.h"

namespace kt
{
SpinBoxDelegate::SpinBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

SpinBoxDelegate::~SpinBoxDelegate()
{
}

QWidget *SpinBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    QSpinBox *editor = new QSpinBox(parent);
    editor->setSuffix(i18n(" KiB/s"));
    if (index.column() < 3)
        editor->setSpecialValueText(i18n("No limit"));
    else
        editor->setSpecialValueText(i18n("No assured speed"));
    editor->setMinimum(0);
    editor->setMaximum(10000000);
    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    int value = index.model()->data(index, Qt::EditRole).toInt();
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QSpinBox *spinBox = static_cast<QSpinBox *>(editor);
    spinBox->interpretText();
    int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    QRect r = option.rect;
    if (option.rect.height() < editor->sizeHint().height())
        r.setHeight(editor->sizeHint().height());
    editor->setGeometry(r);
}

QSize SpinBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSpinBox().sizeHint();
}
}
