/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_TORRENTSEARCHBAR_H
#define KT_TORRENTSEARCHBAR_H

#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

#include <KSharedConfig>

namespace kt
{
class View;

/**
 * Search bar widget for torrents.
 */
class TorrentSearchBar : public QWidget
{
    Q_OBJECT
public:
    TorrentSearchBar(View *view, QWidget *parent);
    ~TorrentSearchBar() override;

    void loadState(KSharedConfigPtr cfg);
    void saveState(KSharedConfigPtr cfg);

public Q_SLOTS:
    void showBar();
    void hideBar();

Q_SIGNALS:
    void filterBarHidden(QString str);
    void filterBarShown(QString str);

protected:
    bool eventFilter(QObject *obj, QEvent *ev) override;

private:
    QToolButton *hide_search_bar;
    QLineEdit *search_bar;
};
}

#endif // KT_TORRENTSEARCHBAR_H
