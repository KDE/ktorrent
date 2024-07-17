/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2005-2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchwidget.h"

#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressBar>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KComboBox>
#include <KIO/Job>
#include <KLocalizedString>
#include <KMainWindow>
#include <KMessageBox>
#include <KNotification>
#include <KStandardAction>

#include "searchactivity.h"
#include "searchenginelist.h"
#include "searchplugin.h"
#include "webview.h"
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <magnet/magnetlink.h>
#include <torrent/globals.h>
#include <util/log.h>

using namespace bt;
using namespace Qt::Literals::StringLiterals;

namespace kt
{
SearchWidget::SearchWidget(SearchPlugin *sp)
    : webview(nullptr)
    , sp(sp)
    , prog(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    webview = new WebView(this, sp->getProxy());

    KActionCollection *ac = sp->getSearchActivity()->part()->actionCollection();
    sbar = new KToolBar(this);
    sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    auto backAction = webview->pageAction(QWebEnginePage::Back);
    backAction->setIcon(QIcon::fromTheme(u"draw-arrow-back"_s));
    sbar->addAction(backAction);

    auto forwardAction = webview->pageAction(QWebEnginePage::Forward);
    forwardAction->setIcon(QIcon::fromTheme(u"draw-arrow-forward"_s));
    sbar->addAction(forwardAction);

    auto reloadAction = webview->pageAction(QWebEnginePage::Reload);
    reloadAction->setIcon(QIcon::fromTheme(u"view-refresh"_s));
    sbar->addAction(reloadAction);

    sbar->addAction(ac->action(QStringLiteral("search_home")));
    search_text = new KComboBox(nullptr);
    search_text->setEditable(true);
    search_text->setMaxCount(20);
    search_text->setInsertPolicy(QComboBox::NoInsert);
    search_text->setMinimumWidth(150);
    search_text->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    sbar->addWidget(search_text);
    sbar->addAction(ac->action(QStringLiteral("search_tab_search")));
    sbar->addWidget(new QLabel(i18n(" Engine: "))); // same i18n string as in SearchToolBar()
    search_engine = new KComboBox(sbar);
    search_engine->setModel(sp->getSearchEngineList());
    sbar->addWidget(search_engine);

    connect(search_text->lineEdit(), &QLineEdit::returnPressed, this, qOverload<>(&SearchWidget::search));

    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);

    layout->addWidget(sbar);
    layout->addWidget(separator);
    layout->addWidget(webview);

    connect(webview, &WebView::loadStarted, this, &SearchWidget::loadStarted);
    connect(webview, &WebView::loadFinished, this, &SearchWidget::loadFinished);
    connect(webview, &WebView::loadProgress, this, &SearchWidget::loadProgress);
    connect(webview, &WebView::iconChanged, this, &SearchWidget::iconChanged);
    connect(webview, &WebView::titleChanged, this, &SearchWidget::titleChanged);
    connect(webview, &WebView::torrentFileDownloadRequested, this, &SearchWidget::downloadTorrentFile);

    loadSearchHistory();
}

SearchWidget::~SearchWidget()
{
    if (prog) {
        sp->getGUI()->getStatusBar()->removeProgressBar(prog);
        prog = nullptr;
    }
}

void SearchWidget::iconChanged()
{
    Q_EMIT changeIcon(this, webview->icon());
}

void SearchWidget::titleChanged(const QString &text)
{
    if (!text.isEmpty()) {
        Q_EMIT changeTitle(this, text);
    } else { // no empty tab titles allowed
        Q_EMIT changeTitle(this, webview->url().toString());
    }
}

QUrl SearchWidget::getCurrentUrl() const
{
    return webview->url();
}

QString SearchWidget::getSearchBarText() const
{
    return search_text->lineEdit()->text();
}

int SearchWidget::getSearchBarEngine() const
{
    return search_engine->currentIndex();
}

void SearchWidget::restore(const QUrl &url, const QString &text, const QString &sb_text, int engine)
{
    Q_UNUSED(text);

    if (url.scheme() == QLatin1String("home"))
        webview->home();
    else
        webview->openUrl(url);

    search_text->lineEdit()->setText(sb_text);

    search_engine->setCurrentIndex(engine);
}

void SearchWidget::search(const QString &text, int engine)
{
    if (search_text->lineEdit()->text() != text)
        search_text->lineEdit()->setText(text);

    if (search_engine->currentIndex() != engine)
        search_engine->setCurrentIndex(engine);

    QUrl url = sp->getSearchEngineList()->search(engine, text);

    KCompletion *comp = search_text->completionObject();
    if (!search_text->contains(text)) {
        comp->addItem(text);
        search_text->addItem(text);
    }
    search_text->lineEdit()->clear();
    saveSearchHistory();

    webview->openUrl(url);
}

QUrl SearchWidget::searchUrl(const QString &search_text)
{
    return sp->getSearchEngineList()->search(search_engine->currentIndex(), search_text);
}

void SearchWidget::setSearchBarEngine(int engine)
{
    search_engine->setCurrentIndex(engine);
}

void SearchWidget::loadProgress(int perc)
{
    if (!prog)
        prog = sp->getGUI()->getStatusBar()->createProgressBar();

    if (prog)
        prog->setValue(perc);
}

void SearchWidget::loadStarted()
{
    if (!prog) {
        prog = sp->getGUI()->getStatusBar()->createProgressBar();

        if (prog)
            prog->setValue(0);
    }
}

void SearchWidget::loadFinished(bool ok)
{
    Q_UNUSED(ok);

    if (prog) {
        sp->getGUI()->getStatusBar()->removeProgressBar(prog);
        prog = nullptr;
    }
}

void SearchWidget::magnetUrl(const QUrl &magnet_url)
{
    MagnetLinkLoadOptions options;
    options.silently = false;
    sp->getCore()->load(bt::MagnetLink(magnet_url.toString()), options);
    QString msg = i18n("Downloading:<br/><b>%1</b>", magnet_url.toString());
    KNotification::event(QStringLiteral("MagnetLinkDownloadStarted"), msg, QPixmap());
}

void SearchWidget::downloadTorrentFile(QWebEngineDownloadRequest *download)
{
    int ret = KMessageBox::questionTwoActionsCancel(nullptr,

                                                    i18n("Do you want to download or save the torrent?"),
                                                    i18n("Download Torrent"),
                                                    KGuiItem(i18n("Download"), QStringLiteral("ktorrent")),
                                                    KStandardGuiItem::save(),
                                                    KStandardGuiItem::cancel(),
                                                    QStringLiteral(":TorrentDownloadFinishedQuestion"));

    if (ret == KMessageBox::PrimaryAction) {
        sp->getCore()->load(download->url(), QString());
    } else if (ret == KMessageBox::SecondaryAction) {
        webview->downloadFile(download);
    }
}

void SearchWidget::search()
{
    search(search_text->lineEdit()->text(), search_engine->currentIndex());
}

QWebEngineView *SearchWidget::newTab()
{
    return sp->getSearchActivity()->newTab()->webview;
}

void SearchWidget::home()
{
    webview->home();
}

bool SearchWidget::backAvailable() const
{
    return webview->pageAction(QWebEnginePage::Back)->isEnabled();
}

void SearchWidget::loadSearchHistory()
{
    QFile fptr(kt::DataDir() + QLatin1String("search_history"));
    if (!fptr.open(QIODevice::ReadOnly))
        return;

    KCompletion *comp = search_text->completionObject();

    Uint32 cnt = 0;
    QTextStream in(&fptr);
    while (!in.atEnd() && cnt < 50) {
        QString line = in.readLine();
        if (line.isEmpty())
            break;

        if (!search_text->contains(line)) {
            comp->addItem(line);
            search_text->addItem(line);
        }
        cnt++;
    }

    search_text->lineEdit()->clear();
}

void SearchWidget::saveSearchHistory()
{
    QFile fptr(kt::DataDir() + QLatin1String("search_history"));
    if (!fptr.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&fptr);
    KCompletion *comp = search_text->completionObject();
    const QStringList items = comp->items();
    for (const QString &s : items) {
        out << s << Qt::endl;
    }
}

void SearchWidget::clearHistory()
{
    KCompletion *comp = search_text->completionObject();
    search_text->clear();
    comp->clear();
}
}

#include "moc_searchwidget.cpp"
