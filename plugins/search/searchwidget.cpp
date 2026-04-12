/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchwidget.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QClipboard>
#include <QFile>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QMenu>
#include <QProgressBar>
#include <QStackedWidget>
#include <QTextStream>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KComboBox>
#include <KCompletion>
#include <KFormat>
#include <KGuiItem>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KMainWindow>
#include <KMessageBox>
#include <KNotification>
#include <KStandardGuiItem>

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
namespace
{
enum ResultColumn {
    NameColumn = 0,
    TrackerColumn,
    SizeColumn,
    SeedsColumn,
    LeechersColumn,
    PublishedColumn,
    ColumnCount,
};

enum ResultRole {
    LinkRole = Qt::UserRole,
    DescriptionRole,
    SizeRole,
    SeedsRole,
    LeechersRole,
    PublishedRole,
};

class TorznabResultItem : public QTreeWidgetItem
{
public:
    using QTreeWidgetItem::QTreeWidgetItem;

    bool operator<(const QTreeWidgetItem &other) const override
    {
        const int column = treeWidget() ? treeWidget()->sortColumn() : 0;
        switch (column) {
        case SizeColumn:
            return data(column, SizeRole).toLongLong() < other.data(column, SizeRole).toLongLong();
        case SeedsColumn:
            return data(column, SeedsRole).toInt() < other.data(column, SeedsRole).toInt();
        case LeechersColumn:
            return data(column, LeechersRole).toInt() < other.data(column, LeechersRole).toInt();
        case PublishedColumn:
            return data(column, PublishedRole).toLongLong() < other.data(column, PublishedRole).toLongLong();
        default:
            return text(column).localeAwareCompare(other.text(column)) < 0;
        }
    }
};
}

SearchWidget::SearchWidget(SearchPlugin *sp)
    : content_stack(nullptr)
    , torznab_page(nullptr)
    , webview(nullptr)
    , sbar(nullptr)
    , sp(sp)
    , prog(nullptr)
    , torznab_summary(nullptr)
    , torznab_results(nullptr)
    , back_action(nullptr)
    , forward_action(nullptr)
    , reload_action(nullptr)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    webview = new WebView(this, sp->getProxy());

    KActionCollection *ac = sp->getSearchActivity()->part()->actionCollection();
    sbar = new KToolBar(this);
    sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    back_action = webview->pageAction(QWebEnginePage::Back);
    back_action->setIcon(QIcon::fromTheme(u"draw-arrow-back"_s));
    sbar->addAction(back_action);

    forward_action = webview->pageAction(QWebEnginePage::Forward);
    forward_action->setIcon(QIcon::fromTheme(u"draw-arrow-forward"_s));
    sbar->addAction(forward_action);

    reload_action = webview->pageAction(QWebEnginePage::Reload);
    reload_action->setIcon(QIcon::fromTheme(u"view-refresh"_s));
    sbar->addAction(reload_action);

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
    connect(search_engine, qOverload<int>(&QComboBox::currentIndexChanged), this, &SearchWidget::currentEngineChanged);

    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFixedHeight(1);

    content_stack = new QStackedWidget(this);
    content_stack->addWidget(webview);

    torznab_page = new QWidget(this);
    auto *torznabLayout = new QVBoxLayout(torznab_page);
    torznabLayout->setContentsMargins(0, 0, 0, 0);
    torznabLayout->setSpacing(6);

    torznab_summary = new QLabel(torznab_page);
    torznab_summary->setWordWrap(true);
    torznabLayout->addWidget(torznab_summary);

    torznab_results = new QTreeWidget(torznab_page);
    torznab_results->setRootIsDecorated(false);
    torznab_results->setAlternatingRowColors(true);
    torznab_results->setSelectionMode(QAbstractItemView::SingleSelection);
    torznab_results->setSortingEnabled(true);
    torznab_results->setContextMenuPolicy(Qt::CustomContextMenu);
    torznab_results->setColumnCount(ColumnCount);
    torznab_results->setHeaderLabels({i18n("Name"), i18n("Tracker"), i18n("Size"), i18n("Seeds"), i18n("Leechers"), i18n("Published")});
    torznab_results->header()->setSectionResizeMode(NameColumn, QHeaderView::Stretch);
    torznab_results->header()->setSectionResizeMode(TrackerColumn, QHeaderView::ResizeToContents);
    torznab_results->header()->setSectionResizeMode(SizeColumn, QHeaderView::ResizeToContents);
    torznab_results->header()->setSectionResizeMode(SeedsColumn, QHeaderView::ResizeToContents);
    torznab_results->header()->setSectionResizeMode(LeechersColumn, QHeaderView::ResizeToContents);
    torznab_results->header()->setSectionResizeMode(PublishedColumn, QHeaderView::ResizeToContents);
    torznab_results->sortItems(SeedsColumn, Qt::DescendingOrder);
    torznabLayout->addWidget(torznab_results);

    content_stack->addWidget(torznab_page);

    layout->addWidget(sbar);
    layout->addWidget(separator);
    layout->addWidget(content_stack);

    connect(webview, &WebView::loadStarted, this, &SearchWidget::loadStarted);
    connect(webview, &WebView::loadFinished, this, &SearchWidget::loadFinished);
    connect(webview, &WebView::loadProgress, this, &SearchWidget::loadProgress);
    connect(webview, &WebView::iconChanged, this, &SearchWidget::iconChanged);
    connect(webview, &WebView::titleChanged, this, &SearchWidget::titleChanged);
    connect(webview, &WebView::torrentFileDownloadRequested, this, &SearchWidget::downloadTorrentFile);
    connect(torznab_results, &QTreeWidget::itemActivated, this, &SearchWidget::torznabResultActivated);
    connect(torznab_results, &QTreeWidget::customContextMenuRequested, this, &SearchWidget::showTorznabResultMenu);

    loadSearchHistory();
    current_url = QUrl(QStringLiteral("about:ktorrent"));
    switchContentMode(ContentMode::Web);
}

SearchWidget::~SearchWidget()
{
    removeProgressBar();
}

void SearchWidget::iconChanged()
{
    if (content_mode == ContentMode::Web) {
        Q_EMIT changeIcon(this, webview->icon());
    }
}

void SearchWidget::titleChanged(const QString &text)
{
    if (content_mode != ContentMode::Web) {
        return;
    }

    if (!text.isEmpty()) {
        Q_EMIT changeTitle(this, text);
    } else { // no empty tab titles allowed
        Q_EMIT changeTitle(this, webview->url().toString());
    }
}

QUrl SearchWidget::getCurrentUrl() const
{
    return current_url;
}

QString SearchWidget::getSearchBarText() const
{
    const QString value = search_text->lineEdit()->text();
    return value.isEmpty() ? current_search_text : value;
}

int SearchWidget::getSearchBarEngine() const
{
    return search_engine->currentIndex();
}

void SearchWidget::restore(const QUrl &url, const QString &text, const QString &sb_text, int engine)
{
    const QString restoreText = sb_text.isEmpty() ? text : sb_text;
    search_text->lineEdit()->setText(restoreText);
    search_engine->setCurrentIndex(engine);
    current_search_text = text;

    if (url.scheme() == QLatin1String("torznab")) {
        search(restoreText, engine);
        return;
    }

    if (url == QUrl(QStringLiteral("about:ktorrent")) || url.scheme() == QLatin1String("home")) {
        home();
        return;
    }

    switchContentMode(ContentMode::Web);
    current_url = url;
    webview->openUrl(url);
}

void SearchWidget::search(const QString &text, int engine)
{
    if (search_text->lineEdit()->text() != text) {
        search_text->lineEdit()->setText(text);
    }

    if (search_engine->currentIndex() != engine) {
        search_engine->setCurrentIndex(engine);
    }

    SearchEngine *engineObject = sp->getSearchEngineList()->engine(search_engine->currentIndex());
    if (!engineObject) {
        return;
    }

    current_search_text = text;

    KCompletion *comp = search_text->completionObject();
    if (!text.isEmpty() && !search_text->contains(text)) {
        comp->addItem(text);
        search_text->addItem(text);
    }
    search_text->lineEdit()->clear();
    saveSearchHistory();

    Q_EMIT changeTitle(this, current_search_text.isEmpty() ? engineObject->engineName() : current_search_text);
    Q_EMIT changeIcon(this, engineObject->engineIcon());

    if (engineObject->isTorznab()) {
        searchTorznab(text, *engineObject);
    } else {
        searchWeb(text, search_engine->currentIndex());
    }
}

SearchEngine *SearchWidget::selectedEngine() const
{
    return sp->getSearchEngineList()->engine(search_engine->currentIndex());
}

void SearchWidget::searchWeb(const QString &text, int engine)
{
    clearTorznabResults();
    current_url = sp->getSearchEngineList()->search(engine, text);
    switchContentMode(ContentMode::Web);
    webview->openUrl(current_url);
}

void SearchWidget::searchTorznab(const QString &text, const SearchEngine &engine)
{
    clearTorznabResults();
    torznab_engine_name = engine.engineName();
    torznab_tracker_first = engine.torznabConfig().trackerFirst;
    current_url = engine.search(text);
    switchContentMode(ContentMode::Torznab);
    torznab_summary->setText(i18n("Searching %1…", torznab_engine_name));
    ensureProgressBar(0);

    torznab_job = new TorznabSearchJob(engine.torznabConfig(), text, sp->getProxy(), this);
    connect(torznab_job, &TorznabSearchJob::resultFound, this, &SearchWidget::torznabResultFound);
    connect(torznab_job, &TorznabSearchJob::fatalError, this, &SearchWidget::torznabSearchFailed);
    connect(torznab_job, &TorznabSearchJob::progressChanged, this, &SearchWidget::torznabSearchProgress);
    connect(torznab_job, &TorznabSearchJob::finished, this, &SearchWidget::torznabSearchFinished);
    torznab_job->start();
}

void SearchWidget::setSearchBarEngine(int engine)
{
    search_engine->setCurrentIndex(engine);
}

void SearchWidget::currentEngineChanged(int index)
{
    Q_UNUSED(index)

    if (current_url != QUrl(QStringLiteral("about:ktorrent"))) {
        return;
    }

    SearchEngine *engineObject = selectedEngine();
    if (engineObject && engineObject->isTorznab()) {
        showTorznabHome();
    } else {
        home();
    }
}

void SearchWidget::ensureProgressBar(int maximum)
{
    if (!prog) {
        prog = sp->getGUI()->getStatusBar()->createProgressBar();
        if (!prog) {
            return;
        }
    }

    if (maximum <= 0) {
        prog->setRange(0, 0);
    } else if (prog->maximum() != maximum || prog->minimum() != 0) {
        prog->setRange(0, maximum);
    }
}

void SearchWidget::removeProgressBar()
{
    if (prog) {
        sp->getGUI()->getStatusBar()->removeProgressBar(prog);
        prog = nullptr;
    }
}

void SearchWidget::loadProgress(int perc)
{
    if (content_mode != ContentMode::Web) {
        return;
    }

    ensureProgressBar(100);
    if (prog) {
        prog->setValue(perc);
    }
}

void SearchWidget::loadStarted()
{
    if (content_mode != ContentMode::Web) {
        return;
    }

    ensureProgressBar(100);
}

void SearchWidget::loadFinished(bool ok)
{
    Q_UNUSED(ok);

    if (content_mode != ContentMode::Web) {
        return;
    }

    if (!(current_url.scheme() == QLatin1String("about") && current_url.path() == QLatin1String("ktorrent"))) {
        current_url = webview->url();
    }
    removeProgressBar();
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
    current_search_text.clear();
    current_url = QUrl(QStringLiteral("about:ktorrent"));

    SearchEngine *engineObject = selectedEngine();
    if (engineObject && engineObject->isTorznab()) {
        showTorznabHome();
        return;
    }

    clearTorznabResults();
    switchContentMode(ContentMode::Web);
    webview->home();
    Q_EMIT changeTitle(this, i18n("Home"));
}

bool SearchWidget::backAvailable() const
{
    return content_mode == ContentMode::Web && webview->pageAction(QWebEnginePage::Back)->isEnabled();
}

void SearchWidget::switchContentMode(ContentMode mode)
{
    content_mode = mode;
    content_stack->setCurrentWidget(mode == ContentMode::Web ? static_cast<QWidget *>(webview) : torznab_page);

    if (mode == ContentMode::Web) {
        back_action->setEnabled(webview->pageAction(QWebEnginePage::Back)->isEnabled());
        forward_action->setEnabled(webview->pageAction(QWebEnginePage::Forward)->isEnabled());
        reload_action->setEnabled(webview->pageAction(QWebEnginePage::Reload)->isEnabled());
    } else {
        back_action->setEnabled(false);
        forward_action->setEnabled(false);
        reload_action->setEnabled(false);
    }
}

void SearchWidget::clearTorznabResults()
{
    if (torznab_job) {
        torznab_job->deleteLater();
        torznab_job = nullptr;
    }

    torznab_result_count = 0;
    torznab_engine_name.clear();
    torznab_results->clear();
    torznab_summary->clear();
    removeProgressBar();
}

void SearchWidget::showTorznabHome()
{
    clearTorznabResults();
    switchContentMode(ContentMode::Torznab);

    SearchEngine *engineObject = selectedEngine();
    torznab_engine_name = engineObject ? engineObject->engineName() : QString();
    torznab_summary->setText(engineObject ? i18n("Enter a search term and press Search to query %1.", torznab_engine_name)
                                          : i18n("Enter a search term and press Search."));
    Q_EMIT changeTitle(this, i18n("Home"));
}

void SearchWidget::torznabResultActivated(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    torznab_results->setCurrentItem(item);
    downloadSelectedTorznabResult();
}

void SearchWidget::showTorznabResultMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = torznab_results->itemAt(pos);
    if (!item) {
        return;
    }

    torznab_results->setCurrentItem(item);
    QMenu menu(this);
    QAction *downloadAction = menu.addAction(i18n("Download"));
    QAction *detailsAction = nullptr;

    if (!item->data(NameColumn, DescriptionRole).toString().isEmpty()) {
        detailsAction = menu.addAction(i18n("Open Description Page"));
    }

    QAction *selectedAction = menu.exec(torznab_results->viewport()->mapToGlobal(pos));
    if (selectedAction == downloadAction) {
        downloadSelectedTorznabResult();
    } else if (selectedAction == detailsAction) {
        openSelectedTorznabDescription();
    }
}

void SearchWidget::downloadSelectedTorznabResult()
{
    QTreeWidgetItem *item = torznab_results->currentItem();
    if (!item) {
        return;
    }

    const QString link = item->data(NameColumn, LinkRole).toString();
    if (link.startsWith(QLatin1String("magnet:?"))) {
        magnetUrl(QUrl(link));
    } else {
        sp->getCore()->load(QUrl(link), QString());
    }
}

void SearchWidget::openSelectedTorznabDescription()
{
    QTreeWidgetItem *item = torznab_results->currentItem();
    if (!item) {
        return;
    }

    const QString descriptionUrl = item->data(NameColumn, DescriptionRole).toString();
    if (descriptionUrl.isEmpty()) {
        return;
    }

    auto *job = new KIO::OpenUrlJob(QUrl(descriptionUrl), QApplication::activeWindow());
    job->start();
}

void SearchWidget::torznabResultFound(const kt::TorznabSearchResult &result)
{
    auto *item = new TorznabResultItem(torznab_results);
    const QString displayName = torznab_tracker_first && !result.tracker.isEmpty() ? i18n("[%1] %2", result.tracker, result.title) : result.title;

    item->setText(NameColumn, displayName);
    item->setText(TrackerColumn, result.tracker);
    item->setText(SizeColumn, result.size >= 0 ? KFormat().formatByteSize(result.size) : QStringLiteral("?"));
    item->setText(SeedsColumn, result.seeders >= 0 ? QString::number(result.seeders) : QStringLiteral("?"));
    item->setText(LeechersColumn, result.leechers >= 0 ? QString::number(result.leechers) : QStringLiteral("?"));
    item->setText(PublishedColumn, result.publishedAt.isValid() ? QLocale().toString(result.publishedAt, QLocale::ShortFormat) : QString());

    item->setData(NameColumn, LinkRole, result.link);
    item->setData(NameColumn, DescriptionRole, result.descriptionUrl);
    item->setData(SizeColumn, SizeRole, result.size);
    item->setData(SeedsColumn, SeedsRole, result.seeders);
    item->setData(LeechersColumn, LeechersRole, result.leechers);
    item->setData(PublishedColumn, PublishedRole, result.publishedAt.isValid() ? result.publishedAt.toSecsSinceEpoch() : 0);

    torznab_result_count++;
    torznab_summary->setText(i18np("1 result", "%1 results", torznab_result_count));
}

void SearchWidget::torznabSearchFinished(int totalIndexers, int failedIndexers)
{
    removeProgressBar();
    torznab_job = nullptr;

    if (totalIndexers <= 0 && torznab_result_count == 0 && !torznab_summary->text().isEmpty()) {
        return;
    }

    QString summary = i18np("1 result from %2", "%1 results from %2", torznab_result_count, torznab_engine_name);
    if (failedIndexers > 0) {
        summary += i18np(" (%1 indexer failed)", " (%1 indexers failed)", failedIndexers);
    }
    torznab_summary->setText(summary);

    if (!current_search_text.isEmpty()) {
        Q_EMIT changeTitle(this, i18n("%1 (%2)", current_search_text, torznab_result_count));
    }
}

void SearchWidget::torznabSearchFailed(const QString &message)
{
    torznab_summary->setText(message);
}

void SearchWidget::torznabSearchProgress(int completed, int total)
{
    ensureProgressBar(total > 0 ? total : 0);
    if (prog && total > 0) {
        prog->setValue(completed);
    }
}

QUrl SearchWidget::searchUrl(const QString &search_text)
{
    return sp->getSearchEngineList()->search(search_engine->currentIndex(), search_text);
}

void SearchWidget::loadSearchHistory()
{
    QFile fptr(kt::DataDir() + QLatin1String("search_history"));
    if (!fptr.open(QIODevice::ReadOnly)) {
        return;
    }

    KCompletion *comp = search_text->completionObject();

    Uint32 cnt = 0;
    QTextStream in(&fptr);
    while (!in.atEnd() && cnt < 50) {
        QString line = in.readLine();
        if (line.isEmpty()) {
            break;
        }

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
    if (!fptr.open(QIODevice::WriteOnly)) {
        return;
    }

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
