/*
    SPDX-FileCopyrightText: 2011 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "torrentsearchbar.h"

#include <QHBoxLayout>
#include <QIcon>
#include <QKeyEvent>

#include <KConfigGroup>
#include <KLocalizedString>

#include "view.h"

namespace kt
{
TorrentSearchBar::TorrentSearchBar(View *view, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    hide_search_bar = new QToolButton(this);
    hide_search_bar->setIcon(QIcon::fromTheme(QStringLiteral("window-close")));
    hide_search_bar->setAutoRaise(true);
    connect(hide_search_bar, &QToolButton::clicked, this, &TorrentSearchBar::hideBar);
    connect(this, &TorrentSearchBar::filterBarHidden, view, &View::setFilterString);

    search_bar = new QLineEdit(this);
    search_bar->setClearButtonEnabled(true);
    search_bar->setPlaceholderText(i18n("Filter..."));
    connect(search_bar, &QLineEdit::textChanged, view, &View::setFilterString);
    connect(this, &TorrentSearchBar::filterBarShown, view, &View::setFilterString);

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
    Q_EMIT filterBarShown(search_bar->text());
}

void TorrentSearchBar::hideBar()
{
    hide();
    Q_EMIT filterBarHidden(QString());
}

bool TorrentSearchBar::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress && ((QKeyEvent *)ev)->key() == Qt::Key_Escape)
        hideBar();

    return QObject::eventFilter(obj, ev);
}

void TorrentSearchBar::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TorrentSearchBar");
    search_bar->setText(g.readEntry("text", QString()));

    if (g.readEntry("hidden", true)) {
        setHidden(true);
        Q_EMIT filterBarHidden(QString());
    } else
        setHidden(false);
}

void TorrentSearchBar::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("TorrentSearchBar");
    g.writeEntry("hidden", isHidden());
    g.writeEntry("text", search_bar->text());
}

}
