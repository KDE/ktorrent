/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "viewdelegate.h"
#include "core.h"
#include "view.h"
#include "viewmodel.h"

#include <algorithm>

#include <QApplication>
#include <QLocale>
#include <QRect>
#include <QVBoxLayout>

#include <gui/extender.h>

namespace kt
{
//////////////////////////

ExtenderBox::ExtenderBox(QWidget *widget)
    : QWidget(widget)
{
    layout = new QVBoxLayout(this);
}

ExtenderBox::~ExtenderBox()
{
    clear();
}

void ExtenderBox::add(Extender *ext)
{
    layout->addWidget(ext);
    extenders.append(ext);
}

void ExtenderBox::remove(Extender *ext)
{
    layout->removeWidget(ext);
    extenders.removeAll(ext);
    ext->hide();
    ext->deleteLater();
}

void ExtenderBox::removeSimilar(Extender *ext)
{
    for (QList<Extender *>::iterator i = extenders.begin(); i != extenders.end();) {
        if (ext->similar(*i)) {
            (*i)->hide();
            (*i)->deleteLater();
            i = extenders.erase(i);
        } else
            i++;
    }
}

void ExtenderBox::clear()
{
    for (Extender *ext : qAsConst(extenders)) {
        ext->hide();
        ext->deleteLater();
    }

    extenders.clear();
}

//////////////////////////

ViewDelegate::ViewDelegate(Core *core, ViewModel *model, View *parent)
    : QStyledItemDelegate(parent)
    , model(model)
{
    connect(core, &Core::torrentRemoved, this, &ViewDelegate::torrentRemoved);
}

ViewDelegate::~ViewDelegate()
{
    contractAll();
}

void ViewDelegate::extend(bt::TorrentInterface *tc, kt::Extender *widget, bool close_similar)
{
    ExtenderBox *ext = nullptr;
    ExtItr itr = extenders.find(tc);
    if (itr == extenders.end()) {
        QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
        ext = new ExtenderBox(aiv->viewport());
        extenders.insert(tc, ext);
    } else {
        ext = itr.value();
    }

    if (close_similar)
        ext->removeSimilar(widget);

    ext->add(widget);
    widget->setParent(ext);
    widget->show();

    scheduleUpdateViewLayout();
    connect(widget, &Extender::closeRequest, this, &ViewDelegate::closeRequested);
    connect(widget, &Extender::resized, this, &ViewDelegate::resized);
}

void ViewDelegate::closeExtenders(bt::TorrentInterface *tc)
{
    ExtItr itr = extenders.find(tc);
    if (itr != extenders.end()) {
        ExtenderBox *ext = itr.value();
        ext->clear();
        ext->hide();
        ext->deleteLater();
        extenders.erase(itr);
    }

    scheduleUpdateViewLayout();
}

void ViewDelegate::closeExtender(bt::TorrentInterface *tc, Extender *ext)
{
    ExtItr itr = extenders.find(tc);
    if (itr != extenders.end()) {
        ExtenderBox *box = itr.value();
        box->remove(ext);
        if (box->count() == 0) {
            box->hide();
            box->deleteLater();
            extenders.erase(itr);
        }
    }

    scheduleUpdateViewLayout();
}

void ViewDelegate::closeRequested(Extender *ext)
{
    closeExtender(ext->torrent(), ext);
}

void ViewDelegate::torrentRemoved(bt::TorrentInterface *tc)
{
    closeExtenders(tc);
}

void ViewDelegate::resized(Extender *ext)
{
    Q_UNUSED(ext);
    scheduleUpdateViewLayout();
}

QSize ViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize ret;

    if (!extenders.isEmpty())
        ret = maybeExtendedSize(option, index);
    else
        ret = QStyledItemDelegate::sizeHint(option, index);

    return ret;
}

QSize ViewDelegate::maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bt::TorrentInterface *tc = model->torrentFromIndex(index);
    QSize size(QStyledItemDelegate::sizeHint(option, index));
    if (!tc)
        return size;

    ExtCItr itr = extenders.find(tc);
    const QWidget *ext = itr == extenders.end() ? nullptr : itr.value();
    if (!ext)
        return size;

    // add extender height to maximum height of any column in our row
    int item_height = size.height();
    int row = index.row();
    int this_column = index.column();

    // this is quite slow, but Qt is smart about when to call sizeHint().
    for (int column = 0; model->columnCount() < column; column++) {
        if (column == this_column)
            continue;

        QModelIndex neighborIndex(index.sibling(row, column));
        if (neighborIndex.isValid())
            item_height = std::max(item_height, QStyledItemDelegate::sizeHint(option, neighborIndex).height());
    }

    // we only want to reserve vertical space, the horizontal extender layout is our private business.
    size.rheight() = item_height + ext->sizeHint().height();
    return size;
}

void ViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem indicatorOption(option);
    initStyleOption(&indicatorOption, index);
    if (index.column() == 0)
        indicatorOption.viewItemPosition = QStyleOptionViewItem::Beginning;
    else if (index.column() == index.model()->columnCount() - 1)
        indicatorOption.viewItemPosition = QStyleOptionViewItem::End;
    else
        indicatorOption.viewItemPosition = QStyleOptionViewItem::Middle;

    QStyleOptionViewItem itemOption(option);
    initStyleOption(&itemOption, index);
    if (index.column() == 0)
        itemOption.viewItemPosition = QStyleOptionViewItem::Beginning;
    else if (index.column() == index.model()->columnCount() - 1)
        itemOption.viewItemPosition = QStyleOptionViewItem::End;
    else
        itemOption.viewItemPosition = QStyleOptionViewItem::Middle;

    bt::TorrentInterface *tc = model->torrentFromIndex(index);
    if (!tc || !extenders.contains(tc)) {
        normalPaint(painter, itemOption, index);
        return;
    }

    QWidget *extender = extenders[tc];
    int extenderHeight = extender->sizeHint().height();

    // an extender is present - make two rectangles: one to paint the original item, one for the extender
    QStyleOptionViewItem extOption(option);
    initStyleOption(&extOption, index);
    extOption.rect = extenderRect(extender, option, index);
    extender->setGeometry(extOption.rect);
    // if we show it before, it will briefly flash in the wrong location.
    // the downside is, of course, that an api user effectively can't hide it.
    extender->show();

    indicatorOption.rect.setHeight(option.rect.height() - extenderHeight);
    itemOption.rect.setHeight(option.rect.height() - extenderHeight);
    // tricky:make sure that the modified options' rect really has the
    // same height as the unchanged option.rect if no extender is present
    //(seems to work OK)

    normalPaint(painter, itemOption, index);
}

void ViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    bt::TorrentInterface *tc = model->torrentFromIndex(index);
    if (!tc || !extenders.contains(tc)) {
        QStyledItemDelegate::updateEditorGeometry(editor, option, index);
    } else {
        QWidget *extender = extenders[tc];
        int extenderHeight = extender->sizeHint().height();

        QStyleOptionViewItem itemOption(option);
        initStyleOption(&itemOption, index);
        itemOption.rect.setHeight(option.rect.height() - extenderHeight);
        editor->setGeometry(itemOption.rect);
    }
}

QRect ViewDelegate::extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QRect rect(option.rect);
    rect.setTop(rect.bottom() + 1 - extender->sizeHint().height());

    rect.setLeft(0);
    if (QTreeView *tv = qobject_cast<QTreeView *>(parent())) {
        int steps = 0;
        for (QModelIndex idx(index.parent()); idx.isValid(); idx = idx.parent())
            steps++;

        if (tv->rootIsDecorated())
            steps++;

        rect.setLeft(steps * tv->indentation());
    }

    QAbstractScrollArea *container = qobject_cast<QAbstractScrollArea *>(parent());
    rect.setRight(container->viewport()->width() - 1);
    return rect;
}

void ViewDelegate::scheduleUpdateViewLayout()
{
    QAbstractItemView *aiv = qobject_cast<QAbstractItemView *>(parent());
    // prevent crashes during destruction of the view
    if (aiv) {
        // dirty hack to call aiv's protected scheduleDelayedItemsLayout()
        aiv->setRootIndex(aiv->rootIndex());
    }
}

void ViewDelegate::contractAll()
{
    for (ExtCItr i = extenders.cbegin(); i != extenders.cend(); i++) {
        i.value()->clear();
        i.value()->hide();
        i.value()->deleteLater();
    }
    extenders.clear();
}

bool ViewDelegate::extended(bt::TorrentInterface *tc) const
{
    return extenders.contains(tc);
}

void ViewDelegate::hideExtender(bt::TorrentInterface *tc)
{
    ExtItr i = extenders.find(tc);
    if (i != extenders.end())
        i.value()->hide();
}

void ViewDelegate::paintProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // first filling the background as if it was an empty cell
    QStyleOptionViewItem dummyOption = option;
    dummyOption.text = QLatin1String("");
    dummyOption.index = QModelIndex();
    QStyledItemDelegate::paint(painter, dummyOption, QModelIndex());

    int progress = index.data().toInt();

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.palette = option.palette;
    progressBarOption.state = option.state;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.progress = progress;
    progressBarOption.text = QLocale().toString(progress) + QLatin1Char('%');
    progressBarOption.textVisible = true;
    progressBarOption.direction = option.direction;

    // ProgressBars have no concept of being selected (AFAIK), so the color of its text has to be changed manually
    if (option.state & QStyle::State_Selected)
        progressBarOption.palette.setColor(QPalette::Active, QPalette::WindowText, option.palette.highlightedText().color());

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
}

void ViewDelegate::normalPaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == ViewModel::PERCENTAGE)
        paintProgressBar(painter, option, index);
    else
        QStyledItemDelegate::paint(painter, option, index);
}

}
