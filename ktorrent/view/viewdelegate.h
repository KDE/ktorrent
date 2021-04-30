/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_VIEWDELEGATE_H
#define KT_VIEWDELEGATE_H

#include <QMap>
#include <QStyledItemDelegate>

class QVBoxLayout;
namespace bt
{
class TorrentInterface;
}

namespace kt
{
class ViewModel;
class View;
class Core;
class Extender;

/**
    Box which contains all the extenders of a widget
*/
class ExtenderBox : public QWidget
{
public:
    ExtenderBox(QWidget *widget);
    ~ExtenderBox() override;

    /// Add an Extender
    void add(Extender *ext);

    /// Remove an Extender
    void remove(Extender *ext);

    /// Remove extenders similar to ext
    void removeSimilar(Extender *ext);

    /// Clear all extenders
    void clear();

    /// Get the number of extenders
    int count() const
    {
        return extenders.count();
    }

private:
    QVBoxLayout *layout;
    QList<Extender *> extenders;
};

/**
    Item delegate which keeps track of of ScanExtenders
*/
class ViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ViewDelegate(Core *core, ViewModel *model, View *parent);
    ~ViewDelegate() override;

    /**
        Extend a torrent with a widget
    */
    void extend(bt::TorrentInterface *tc, Extender *widget, bool close_similar);

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * Close all extenders and delete all extender widgets.
     */
    void contractAll();

    /// Is an extender being shown for a torrent
    bool extended(bt::TorrentInterface *tc) const;

    /// Hide the extender for a torrent
    void hideExtender(bt::TorrentInterface *tc);

    /// Does the delegate have extenders
    bool hasExtenders() const
    {
        return !extenders.isEmpty();
    }

public Q_SLOTS:
    /// Close all the extenders of a torrent
    void closeExtenders(bt::TorrentInterface *tc);
    void closeExtender(bt::TorrentInterface *tc, Extender *ext);

private Q_SLOTS:
    void torrentRemoved(bt::TorrentInterface *tc);
    void closeRequested(Extender *ext);
    void resized(Extender *ext);

private:
    QSize maybeExtendedSize(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QRect extenderRect(QWidget *extender, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void scheduleUpdateViewLayout();
    void paintProgressBar(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void normalPaint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    ViewModel *model;
    QMap<bt::TorrentInterface *, ExtenderBox *> extenders;

    typedef QMap<bt::TorrentInterface *, ExtenderBox *>::iterator ExtItr;
    typedef QMap<bt::TorrentInterface *, ExtenderBox *>::const_iterator ExtCItr;
};

}

#endif // KT_VIEWDELEGATE_H
