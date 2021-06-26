/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTMEDIAVIEW_H
#define KTMEDIAVIEW_H

#include <KSharedConfig>
#include <QCheckBox>
#include <QListView>
#include <QSortFilterProxyModel>

#include "mediafile.h"

class QLineEdit;
class KToolBar;

namespace kt
{
class MediaModel;

/**
 * QSortFilterProxyModel to filter out incomplete files
 */
class MediaViewFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    MediaViewFilter(QObject *parent = nullptr);
    ~MediaViewFilter() override;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    /// Enable or disable showing of incomplete files
    void setShowIncomplete(bool on);

public Q_SLOTS:
    void refresh();

private:
    bool show_incomplete;
};

/**
    @author
*/
class MediaView : public QWidget
{
    Q_OBJECT
public:
    MediaView(MediaModel *model, QWidget *parent);
    ~MediaView() override;

    void saveState(KSharedConfig::Ptr cfg);
    void loadState(KSharedConfig::Ptr cfg);

Q_SIGNALS:
    void doubleClicked(const MediaFileRef &mf);

private Q_SLOTS:
    void onDoubleClicked(const QModelIndex &index);
    void showIncompleteChanged(bool on);

private:
    MediaModel *model;
    QListView *media_tree;
    QLineEdit *search_box;
    MediaViewFilter *filter;
    KToolBar *tool_bar;
    QAction *show_incomplete;
    QAction *refresh;
};

}

#endif
