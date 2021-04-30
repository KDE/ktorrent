/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <QComboBox>

#include "logflagsdelegate.h"
#include <util/log.h>

namespace kt
{
LogFlagsDelegate::LogFlagsDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

LogFlagsDelegate::~LogFlagsDelegate()
{
}

QWidget *LogFlagsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QComboBox *editor = new QComboBox(parent);
    editor->addItem(i18n("All"));
    editor->addItem(i18n("Important"));
    editor->addItem(i18n("Notice"));
    editor->addItem(i18n("Debug"));
    editor->addItem(i18n("None"));
    return editor;
}

void LogFlagsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    bt::Uint32 value = index.model()->data(index, Qt::EditRole).toUInt();
    QComboBox *cb = static_cast<QComboBox *>(editor);
    switch (value) {
    case LOG_DEBUG:
        cb->setCurrentIndex(3);
        break;
    case LOG_NOTICE:
        cb->setCurrentIndex(2);
        break;
    case LOG_IMPORTANT:
        cb->setCurrentIndex(1);
        break;
    case LOG_ALL:
        cb->setCurrentIndex(0);
        break;
    case LOG_NONE:
        cb->setCurrentIndex(4);
        break;
    }
}

void LogFlagsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *cb = static_cast<QComboBox *>(editor);
    switch (cb->currentIndex()) {
    case 0:
        model->setData(index, LOG_ALL, Qt::EditRole);
        break;
    case 1:
        model->setData(index, LOG_IMPORTANT, Qt::EditRole);
        break;
    case 2:
        model->setData(index, LOG_NOTICE, Qt::EditRole);
        break;
    case 3:
        model->setData(index, LOG_DEBUG, Qt::EditRole);
        break;
    case 4:
        model->setData(index, LOG_NONE, Qt::EditRole);
        break;
    }
}

QSize LogFlagsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    QComboBox tmp;
    return QSize(100, tmp.sizeHint().height());
}
}
