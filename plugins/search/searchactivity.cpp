/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#include "searchactivity.h"

#include <algorithm>

#include <QFile>
#include <QIcon>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardAction>

#include <interfaces/functions.h>
#include <util/indexofcompare.h>
#include <util/error.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>

#include "searchwidget.h"
#include "searchplugin.h"
#include <searchpluginsettings.h>
#include "searchtoolbar.h"


namespace kt
{
    SearchActivity::SearchActivity(SearchPlugin* sp, QWidget* parent)
        : Activity(i18nc("plugin name", "Search"), QStringLiteral("edit-find"), 10, parent), sp(sp)
    {
        setXMLGUIFile(QStringLiteral("ktorrent_searchui.rc"));
        setupActions();
        toolbar = new SearchToolBar(part()->actionCollection(), sp->getSearchEngineList(), this);
        connect(toolbar, &SearchToolBar::search, sp, &SearchPlugin::search);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);
        tabs = new QTabWidget(this);
        tabs->setMovable(true);
        layout->addWidget(tabs);
        connect(tabs, &QTabWidget::currentChanged, this, &SearchActivity::currentTabChanged);

        QToolButton* lc = new QToolButton(tabs);
        tabs->setCornerWidget(lc, Qt::TopLeftCorner);
        QToolButton* rc = new QToolButton(tabs);
        tabs->setCornerWidget(rc, Qt::TopRightCorner);
        lc->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
        connect(lc, &QToolButton::clicked, this, &SearchActivity::openTab);
        rc->setIcon(QIcon::fromTheme(QStringLiteral("tab-close")));
        connect(rc, &QToolButton::clicked, this, &SearchActivity::closeTab);
    }

    SearchActivity::~SearchActivity()
    {
    }

    void SearchActivity::setupActions()
    {
        KActionCollection* ac = part()->actionCollection();

        search_action = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Search"), this);
        connect(search_action, &QAction::triggered, this, static_cast<void (SearchActivity::*)()>(&SearchActivity::search));
        ac->addAction(QStringLiteral("search_tab_search"), search_action);

        find_action = KStandardAction::find(this, SLOT(find()), this);
        ac->addAction(QStringLiteral("search_tab_find"), find_action);

        home_action = KStandardAction::home(this, SLOT(home()), this);
        ac->addAction(QStringLiteral("search_home"), home_action);
    }

    void SearchActivity::search(const QString& text, int engine)
    {
        for (SearchWidget* s : qAsConst(searches))
        {
            if (s->getCurrentUrl() == QUrl(QStringLiteral("about:ktorrent")))
            {
                s->search(text, engine);
                tabs->setCurrentWidget(s);
                return;
            }
        }

        SearchWidget* sw = newSearchWidget(text);
        sw->search(text, engine);
        tabs->setCurrentWidget(sw);
    }

    void SearchActivity::saveCurrentSearches()
    {
        QFile fptr(kt::DataDir() + QStringLiteral("current_searches"));
        if (!fptr.open(QIODevice::WriteOnly))
            return;

        // Sort by order in tab widget so that they are restored in the proper order
        std::sort(searches.begin(), searches.end(), IndexOfCompare<QTabWidget, SearchWidget>(tabs));
        bt::BEncoder enc(&fptr);
        enc.beginList();
        for (SearchWidget* w : qAsConst(searches))
        {
            enc.beginDict();
            enc.write("TEXT", w->getSearchText().toUtf8());
            enc.write("URL", w->getCurrentUrl().toDisplayString().toUtf8());
            enc.write("SBTEXT", w->getSearchBarText().toUtf8());
            enc.write("ENGINE", (bt::Uint32)w->getSearchBarEngine());
            enc.end();
        }
        enc.end();
    }

    void SearchActivity::loadCurrentSearches()
    {
        if (!SearchPluginSettings::restorePreviousSession())
        {
            SearchWidget* search = newSearchWidget(QString());
            search->home();
            return;
        }

        QFile fptr(kt::DataDir() + QLatin1String("current_searches"));
        if (!fptr.open(QIODevice::ReadOnly))
        {
            SearchWidget* search = newSearchWidget(QString());
            search->home();
            return;
        }

        QByteArray data = fptr.readAll();
        bt::BDecoder dec(data, false, 0);
        bt::BListNode* search_list = 0;
        try
        {
            search_list = dec.decodeList();
            if (!search_list)
                throw bt::Error(QStringLiteral("Invalid current searches"));

            for (bt::Uint32 i = 0; i < search_list->getNumChildren(); i++)
            {
                bt::BDictNode* dict = search_list->getDict(i);
                if (!dict)
                    continue;

                QString text = dict->getString("TEXT", 0);
                QString sbtext = dict->getString("SBTEXT", 0);
                int engine = dict->getInt("ENGINE");
                QUrl url = QUrl(dict->getString("URL", 0));

                SearchWidget* search = newSearchWidget(text);
                search->restore(url, text, sbtext, engine);
            }

            delete search_list;
        }
        catch (...)
        {
            delete search_list;
        }

        if (searches.count() == 0)
        {
            SearchWidget* search = newSearchWidget(QString());
            search->home();
        }
    }

    void SearchActivity::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("SearchActivity");
        g.writeEntry("current_search", tabs->currentIndex());
        toolbar->saveSettings();
    }

    void SearchActivity::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("SearchActivity");
        int idx = g.readEntry("current_search", 0);
        tabs->setCurrentIndex(idx);
    }

    void SearchActivity::find()
    {
        QWidget* w = tabs->currentWidget();
        foreach (SearchWidget* s, searches)
        {
            if (w == s)
            {
//              s->find();
                break;
            }
        }
    }

    void SearchActivity::search()
    {
        QWidget* w = tabs->currentWidget();
        foreach (SearchWidget* s, searches)
        {
            if (w == s)
            {
                s->search();
                break;
            }
        }
    }

    /*
    void SearchActivity::copy()
    {
        QWidget* w = tabs->currentWidget();
        foreach (SearchWidget* s,searches)
        {
            if (w == s)
            {
                s->copy();
                break;
            }
        }
    }
    */

    SearchWidget* SearchActivity::newSearchWidget(const QString& text)
    {
        SearchWidget* search = new SearchWidget(sp);
        int idx = tabs->addTab(search, QIcon::fromTheme(QLatin1String("edit-find")), text);
        if (!text.isEmpty())
            tabs->setTabToolTip(idx, i18n("Search for %1", text));

        connect(search, &SearchWidget::openNewTab, this, &SearchActivity::openNewTab);
        connect(search, &SearchWidget::changeTitle, this, &SearchActivity::setTabTitle);
        connect(search, &SearchWidget::changeIcon, this, &SearchActivity::setTabIcon);
        searches.append(search);
        search->setSearchBarEngine(toolbar->currentSearchEngine());
        return search;
    }

    SearchWidget* SearchActivity::newTab()
    {
        return newSearchWidget(QString());
    }


    void SearchActivity::openNewTab(const QUrl &url)
    {
        QString text = url.host();
        SearchWidget* search = newSearchWidget(text);
        search->restore(url, text, QString(), toolbar->currentSearchEngine());
        tabs->setCurrentWidget(search);
    }

    void SearchActivity::currentTabChanged(int idx)
    {
        Q_UNUSED(idx);
        tabs->cornerWidget(Qt::TopRightCorner)->setEnabled(searches.count() > 1);
    }

    void SearchActivity::home()
    {
        QWidget* w = tabs->currentWidget();
        foreach (SearchWidget* s, searches)
        {
            if (w == s)
            {
                s->home();
                break;
            }
        }
    }

    void SearchActivity::closeTab()
    {
        if (searches.count() == 1)
            return;

        foreach (SearchWidget* s, searches)
        {
            if (s == tabs->currentWidget())
            {
                tabs->removeTab(tabs->currentIndex());
                searches.removeAll(s);
                delete s;
                break;
            }
        }

        tabs->cornerWidget(Qt::TopRightCorner)->setEnabled(searches.count() > 1);
    }

    void SearchActivity::openTab()
    {
        SearchWidget* search = newSearchWidget(QString());
        search->home();
        tabs->setCurrentWidget(search);
    }

    void SearchActivity::setTabTitle(SearchWidget* sw, const QString& title)
    {
        int idx = tabs->indexOf(sw);
        if (idx >= 0)
            tabs->setTabText(idx, title);
    }

    void SearchActivity::setTabIcon(SearchWidget* sw, const QIcon& icon)
    {
        int idx = tabs->indexOf(sw);
        if (idx >= 0)
            tabs->setTabIcon(idx, icon);
    }


    void SearchActivity::clearSearchHistory()
    {
        toolbar->clearHistory();
    }

}
