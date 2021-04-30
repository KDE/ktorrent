/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ScriptDelegate(QAbstractItemView *parent);
    ~ScriptDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QList<QWidget *> createItemWidgets(const QModelIndex &index) const override;
    void updateItemWidgets(const QList<QWidget *> widgets, const QStyleOptionViewItem &option, const QPersistentModelIndex &index) const override;

private:
    QFont titleFont(const QFont &baseFont) const;

private Q_SLOTS:
    void toggled(bool on);
    void aboutClicked();
    void settingsClicked();

private:
    QCheckBox *check_box;
    QPushButton *push_button;
};

}

#endif // KT_SCRIPTDELEGATE_H
