/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#include "torrentsearchbar.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>

#include <KConfigGroup>
#include <KLocalizedString>

#include "view.h"


namespace kt
{
    TorrentSearchBar::TorrentSearchBar(View* view, QWidget* parent): QWidget(parent)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);

        hide_search_bar = new QToolButton(this);
        hide_search_bar->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
        hide_search_bar->setAutoRaise(true);
        connect(hide_search_bar, &QToolButton::clicked, this, &TorrentSearchBar::hideBar);
        connect(this, SIGNAL(filterBarHidden(QString)), view, SLOT(setFilterString(QString)));

        search_bar = new QLineEdit(this);
        search_bar->setClearButtonEnabled(true);
        search_bar->setPlaceholderText(i18n("Torrent filter"));
        connect(search_bar, SIGNAL(textChanged(QString)), view, SLOT(setFilterString(QString)));
        connect(this, SIGNAL(filterBarShown(QString)), view, SLOT(setFilterString(QString)));

        layout->addWidget(hide_search_bar);
        layout->addWidget(search_bar);

        search_bar->installEventFilter(this);
    }

    TorrentSearchBar::~TorrentSearchBar()
    {
    }

    void TorrentSearchBar::showBar()
    {
        show();
        search_bar->setFocus();
        emit filterBarShown(search_bar->text());
    }

    void TorrentSearchBar::hideBar()
    {
        hide();
        emit filterBarHidden(QString());
    }


    bool TorrentSearchBar::eventFilter(QObject* obj, QEvent* ev)
    {
        if (ev->type() == QEvent::KeyPress && ((QKeyEvent*)ev)->key() == Qt::Key_Escape)
            hideBar();

        return QObject::eventFilter(obj, ev);
    }

    void TorrentSearchBar::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("TorrentSearchBar");
        search_bar->setText(g.readEntry("text", QString()));

        if (g.readEntry("hidden", true))
        {
            setHidden(true);
            emit filterBarHidden(QString());
        }
        else
            setHidden(false);
    }

    void TorrentSearchBar::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("TorrentSearchBar");
        g.writeEntry("hidden", isHidden());
        g.writeEntry("text", search_bar->text());
    }

}

