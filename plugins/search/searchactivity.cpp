/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
#include <KStandardActions>

#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <interfaces/functions.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/indexofcompare.h>

#include "searchplugin.h"
#include "searchwidget.h"
#include <searchpluginsettings.h>

namespace kt
{
SearchActivity::SearchActivity(SearchPlugin *sp, QWidget *parent)
    : Activity(i18nc("plugin name", "Search"), QStringLiteral("edit-find"), 10, parent)
    , sp(sp)
{
    setXMLGUIFile(QStringLiteral("ktorrent_searchui.rc"));
    setupActions();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    tabs = new QTabWidget(this);
    tabs->setDocumentMode(true);
    tabs->setMovable(true);
    connect(tabs, &QTabWidget::tabCloseRequested, this, &SearchActivity::closeTab);
    layout->addWidget(tabs);

    auto newTabButton = new QToolButton(tabs);
    newTabButton->setIcon(QIcon::fromTheme(QStringLiteral("tab-new")));
    connect(newTabButton, &QToolButton::clicked, this, &SearchActivity::openTab);
    tabs->setCornerWidget(newTabButton, Qt::TopRightCorner);
}

SearchActivity::~SearchActivity()
{
}

void SearchActivity::setupActions()
{
    KActionCollection *ac = part()->actionCollection();

    search_action = new QAction(QIcon::fromTheme(QStringLiteral("edit-find")), i18n("Search"), this);
    connect(search_action, &QAction::triggered, this, qOverload<>(&SearchActivity::search));
    ac->addAction(QStringLiteral("search_tab_search"), search_action);

    find_action = KStandardActions::find(this, &SearchActivity::find, this);
    ac->addAction(QStringLiteral("search_tab_find"), find_action);

    home_action = KStandardActions::home(this, &SearchActivity::home, this);
    ac->addAction(QStringLiteral("search_home"), home_action);
}

void SearchActivity::search(const QString &text, int engine)
{
    for (SearchWidget *s : std::as_const(searches)) {
        if (s->getCurrentUrl() == QUrl(QStringLiteral("about:ktorrent"))) {
            s->search(text, engine);
            tabs->setCurrentWidget(s);
            return;
        }
    }

    SearchWidget *sw = newSearchWidget(text);
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
    for (SearchWidget *w : std::as_const(searches)) {
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
    if (!SearchPluginSettings::restorePreviousSession()) {
        SearchWidget *search = newSearchWidget(QString());
        search->home();
        return;
    }

    QFile fptr(kt::DataDir() + QLatin1String("current_searches"));
    if (!fptr.open(QIODevice::ReadOnly)) {
        SearchWidget *search = newSearchWidget(QString());
        search->home();
        return;
    }

    QByteArray data = fptr.readAll();
    bt::BDecoder dec(data, false, 0);
    try {
        const std::unique_ptr<bt::BListNode> search_list = dec.decodeList();
        if (!search_list)
            throw bt::Error(QStringLiteral("Invalid current searches"));

        for (bt::Uint32 i = 0; i < search_list->getNumChildren(); i++) {
            bt::BDictNode *dict = search_list->getDict(i);
            if (!dict)
                continue;

            QString text = dict->getString("TEXT");
            QString sbtext = dict->getString("SBTEXT");
            int engine = dict->getInt("ENGINE");
            QUrl url = QUrl(dict->getString("URL"));

            SearchWidget *search = newSearchWidget(text);
            search->restore(url, text, sbtext, engine);
        }
    } catch (...) {
    }

    if (searches.count() == 0) {
        SearchWidget *search = newSearchWidget(QString());
        search->home();
    }
}

void SearchActivity::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group(QStringLiteral("SearchActivity"));
    g.writeEntry("current_search", tabs->currentIndex());
}

void SearchActivity::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group(QStringLiteral("SearchActivity"));
    int idx = g.readEntry("current_search", 0);
    tabs->setCurrentIndex(idx);
}

void SearchActivity::find()
{
    QWidget *w = tabs->currentWidget();
    for (SearchWidget *s : std::as_const(searches)) {
        if (w == s) {
            //              s->find();
            break;
        }
    }
}

void SearchActivity::search()
{
    QWidget *w = tabs->currentWidget();
    for (SearchWidget *s : std::as_const(searches)) {
        if (w == s) {
            s->search();
            break;
        }
    }
}

/*
void SearchActivity::copy()
{
    QWidget* w = tabs->currentWidget();
    for (SearchWidget* s: std::as_const(searches))
    {
        if (w == s)
        {
            s->copy();
            break;
        }
    }
}
*/

SearchWidget *SearchActivity::newSearchWidget(const QString &text)
{
    SearchWidget *search = new SearchWidget(sp);
    int idx = tabs->addTab(search, QIcon::fromTheme(QLatin1String("edit-find")), text);
    if (!text.isEmpty())
        tabs->setTabToolTip(idx, i18n("Search for %1", text));

    connect(search, &SearchWidget::openNewTab, this, &SearchActivity::openNewTab);
    connect(search, &SearchWidget::changeTitle, this, &SearchActivity::setTabTitle);
    connect(search, &SearchWidget::changeIcon, this, &SearchActivity::setTabIcon);
    searches.append(search);
    return search;
}

SearchWidget *SearchActivity::newTab()
{
    return newSearchWidget(QString());
}

void SearchActivity::openNewTab(const QUrl &url)
{
    QString text = url.host();
    SearchWidget *search = newSearchWidget(text);
    search->restore(url, text, QString(), 1);
    tabs->setCurrentWidget(search);
}

void SearchActivity::home()
{
    QWidget *w = tabs->currentWidget();
    for (SearchWidget *s : std::as_const(searches)) {
        if (w == s) {
            s->home();
            break;
        }
    }
}

void SearchActivity::closeTab(int index)
{
    if (searches.count() == 1)
        return;

    auto searchWidget = searches[index];
    tabs->removeTab(index);
    searches.remove(index);
    delete searchWidget;

    tabs->setTabsClosable(searches.count() > 1);
}

void SearchActivity::openTab()
{
    SearchWidget *search = newSearchWidget(QString());
    search->home();
    tabs->setCurrentWidget(search);

    tabs->setTabsClosable(true);
}

void SearchActivity::setTabTitle(SearchWidget *sw, const QString &title)
{
    int idx = tabs->indexOf(sw);
    if (idx >= 0)
        tabs->setTabText(idx, title);
}

void SearchActivity::setTabIcon(SearchWidget *sw, const QIcon &icon)
{
    int idx = tabs->indexOf(sw);
    if (idx >= 0)
        tabs->setTabIcon(idx, icon);
}

void SearchActivity::clearSearchHistory()
{
    bt::Delete(kt::DataDir() + QLatin1String("search_history"), true);
    for (SearchWidget *s : std::as_const(searches)) {
        s->clearHistory();
    }
}
}

#include "moc_searchactivity.cpp"
