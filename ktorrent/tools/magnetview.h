/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_MAGNETVIEW_H
#define KT_MAGNETVIEW_H

#include <KSharedConfig>
#include <QWidget>

class QTreeView;
class QMenu;

namespace kt
{
class MagnetManager;
class MagnetModel;

/**
    View which displays a list of magnet links being downloaded
*/
class MagnetView : public QWidget
{
    Q_OBJECT
public:
    MagnetView(MagnetManager *magnetManager, QWidget *parent = nullptr);
    ~MagnetView() override;

    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

    void keyPressEvent(QKeyEvent *event) override;

private Q_SLOTS:
    void showContextMenu(QPoint p);
    void removeMagnetDownload();
    void startMagnetDownload();
    void stopMagnetDownload();
    void copyMagnetUrl();

private:
    MagnetManager *mman;
    MagnetModel *model;
    QTreeView *view;
    QMenu *menu;

    QAction *start;
    QAction *stop;
    QAction *copy_url;
    QAction *remove;
};

}

#endif // KT_MAGNETVIEW_H
