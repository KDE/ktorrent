/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAction>
#include <QClipboard>
#include <QDomDocument>
#include <QMenu>
#include <QMenuBar>
#include <QPushButton>
#include <QScreen>
#include <QTimer>

#include <KActionCollection>
#include <KConfig>
#include <KEditToolBar>
#include <KFileWidget>
#include <KGlobalAccel>
#include <KIO/Job>
#include <KIO/JobUiDelegate>
#include <KMessageBox>
#include <KNotifyConfigWidget>
#include <KParts/PartManager>
#include <KRecentDirs>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToggleAction>
#include <KXMLGUIFactory>

#include <dht/dhtbase.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <interfaces/torrentinterface.h>
#include <plugin/pluginmanager.h>
#include <settings.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <util/error.h>
#include <util/functions.h>
#include <util/log.h>
#include <util/timer.h>

#include "core.h"
#include "dbus/dbus.h"
#include "dialogs/importdialog.h"
#include "dialogs/pastedialog.h"
#include "dialogs/torrentcreatordlg.h"
#include "groups/groupview.h"
#include "gui.h"
#include "ipfilterwidget.h"
#include "pref/prefdialog.h"
#include "statusbar.h"
#include "tools/queuemanagerwidget.h"
#include "trayicon.h"
#include "view/view.h"
#include <gui/centralwidget.h>

#include "torrentactivity.h"

#include <interfaces/functions.h>

namespace kt
{
GUI::GUI()
    : core(nullptr)
    , pref_dlg(nullptr)
{
    // Marker markk("GUI::GUI()");
    part_manager = new KParts::PartManager(this);
    connect(part_manager, &KParts::PartManager::activePartChanged, this, &GUI::activePartChanged);
    core = new Core(this);
    core->loadTorrents();

    tray_icon = new TrayIcon(core, this);

    central = new CentralWidget(this);
    setCentralWidget(central);
    connect(central, &CentralWidget::changeActivity, this, &GUI::setCurrentActivity);
    torrent_activity = new TorrentActivity(core, this, nullptr);

    status_bar = new kt::StatusBar(this);
    setStatusBar(status_bar);

    setupActions();
    setupGUI(Default, QStringLiteral("ktorrentui.rc"));

    addActivity(torrent_activity);

    // mark.update();
    connect(&timer, &QTimer::timeout, this, &GUI::update);
    timer.start(Settings::guiUpdateInterval());

    applySettings();
    connect(core, &Core::settingsChanged, this, &GUI::applySettings);

    if (Settings::showSystemTrayIcon()) {
        tray_icon->updateMaxRateMenus();
        tray_icon->show();
    } else
        tray_icon->hide();

    dbus_iface = new DBus(this, core, this);
    core->loadPlugins();
    loadState(KSharedConfig::openConfig());

    IPFilterWidget::registerFilterList();

    // markk.update();
    updateActions();
    core->startUpdateTimer();

    tray_icon->setAssociatedWindow(windowHandle());
}

GUI::~GUI()
{
    delete core;
}

bool GUI::event(QEvent *e)
{
    if (e->type() == QEvent::DeferredDelete) {
        // HACK to prevent ktorrent from crashing on logout/shotdown (when launched e.g. via alt+f2)
        delete core;
        core = nullptr;
        return true;
    }

    return KParts::MainWindow::event(e);
}

QSize GUI::sizeHint() const
{
    QSize desktop_size = QGuiApplication::primaryScreen()->availableSize();
    return KParts::MainWindow::sizeHint().expandedTo(desktop_size);
}

void GUI::addActivity(Activity *act)
{
    unplugActionList(QStringLiteral("activities_list"));
    central->addActivity(act);
    if (act->part())
        part_manager->addPart(act->part(), false);
    plugActionList(QStringLiteral("activities_list"), central->activitySwitchingActions());
}

void GUI::removeActivity(Activity *act)
{
    unplugActionList(QStringLiteral("activities_list"));
    central->removeActivity(act);
    if (act->part())
        part_manager->removePart(act->part());
    plugActionList(QStringLiteral("activities_list"), central->activitySwitchingActions());
}

void GUI::setCurrentActivity(Activity *act)
{
    central->setCurrentActivity(act);
    part_manager->setActivePart(act ? act->part() : nullptr);
}

void GUI::activePartChanged(KParts::Part *p)
{
    unplugActionList(QStringLiteral("activities_list"));
    createGUI(p);
    plugActionList(QStringLiteral("activities_list"), central->activitySwitchingActions());
}

void GUI::addPrefPage(PrefPageInterface *page)
{
    if (!pref_dlg) {
        pref_dlg = new PrefDialog(this, core);
        pref_dlg->loadState(KSharedConfig::openConfig());
    }

    pref_dlg->addPrefPage(page);
}

void GUI::removePrefPage(PrefPageInterface *page)
{
    if (pref_dlg)
        pref_dlg->removePrefPage(page);
}

StatusBarInterface *GUI::getStatusBar()
{
    return status_bar;
}

void GUI::mergePluginGui(Plugin *p)
{
    if (p->parentPart() == QStringLiteral("ktorrent")) {
        guiFactory()->addClient(p);
    } else {
        const QList<KParts::Part *> parts = part_manager->parts();
        for (KParts::Part *part : parts) {
            if (part->domDocument().documentElement().attribute(QStringLiteral("name")) == p->parentPart()) {
                part->insertChildClient(p);
                break;
            }
        }
    }
}

void GUI::removePluginGui(Plugin *p)
{
    if (p->factory()) {
        p->factory()->removeClient(p);
    }

    if (p->parentPart() != QStringLiteral("ktorrent")) {
        const QList<KParts::Part *> parts = part_manager->parts();
        for (KParts::Part *part : parts) {
            if (part->domDocument().documentElement().attribute(QStringLiteral("name")) == p->parentPart()) {
                part->removeChildClient(p);
                break;
            }
        }
    }
}

void GUI::errorMsg(const QString &err)
{
    KMessageBox::error(this, err);
}

void GUI::errorMsg(KIO::Job *j)
{
    if (j->error())
        j->uiDelegate()->showErrorMessage();
}

void GUI::infoMsg(const QString &info)
{
    KMessageBox::information(this, info);
}

void GUI::load(const QUrl &url)
{
    core->load(url, QString());
}

void GUI::loadSilently(const QUrl &url)
{
    core->loadSilently(url, QString());
}

void GUI::createTorrent()
{
    TorrentCreatorDlg *dlg = new TorrentCreatorDlg(core, this, this);
    dlg->show();
}

void GUI::openTorrent(bool silently)
{
    QString recentDirClass;
    QUrl defaultUrl = KFileWidget::getStartUrl(QUrl(QStringLiteral("kfiledialog:///openTorrent")), recentDirClass);
    if (!QDir(defaultUrl.toLocalFile()).exists())
        defaultUrl = QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    const QList<QUrl> urls = QFileDialog::getOpenFileUrls(this, i18n("Open Location"), defaultUrl, kt::TorrentFileFilter(true));

    if (urls.isEmpty())
        return;

    if (!recentDirClass.isEmpty() && defaultUrl.toLocalFile() != urls.first().toLocalFile())
        KRecentDirs::add(recentDirClass, QFileInfo(urls.first().toLocalFile()).absolutePath());

    if (urls.count() == 1 && !silently) {
        QUrl url = urls.front();
        if (url.isValid())
            load(url);
    } else {
        // load multiple torrents silently
        for (const QUrl &url : urls) {
            if (url.isValid()) {
                if (silently || Settings::openMultipleTorrentsSilently())
                    loadSilently(url);
                else
                    load(url);
            }
        }
    }
}

void GUI::pasteURL()
{
    PasteDialog dlg(core, this);
    dlg.loadState(KSharedConfig::openConfig());
    dlg.exec();
    dlg.saveState(KSharedConfig::openConfig());
}

void GUI::paste()
{
    if (!paste_action->isEnabled())
        return;

    QClipboard *cb = QApplication::clipboard();
    QString text = cb->text(QClipboard::Clipboard);
    if (text.length() == 0)
        return;

    QUrl url = QFile::exists(text) ? QUrl::fromLocalFile(text) : QUrl(text);

    if (url.isValid())
        load(url);
    else
        KMessageBox::error(this, i18n("Invalid URL: %1", url.toDisplayString()));
}

void GUI::showPrefDialog()
{
    if (!pref_dlg)
        pref_dlg = new PrefDialog(this, core);

    pref_dlg->updateWidgetsAndShow();
}

void GUI::showIPFilter()
{
    IPFilterWidget *dlg = new IPFilterWidget(this);
    dlg->show();
}

void GUI::configureKeys()
{
    KShortcutsDialog::showDialog(actionCollection());
}

void GUI::configureToolbars()
{
    auto main_window_group = KSharedConfig::openConfig()->group(QStringLiteral("MainWindow"));
    saveMainWindowSettings(main_window_group);
    KEditToolBar dlg(factory());
    connect(&dlg, &KEditToolBar::newToolBarConfig, this, &GUI::newToolBarConfig);
    dlg.exec();

    // Replug action list
    unplugActionList(QStringLiteral("activities_list"));
    plugActionList(QStringLiteral("activities_list"), central->activitySwitchingActions());
}

void GUI::newToolBarConfig() // This is called when OK, Apply or Defaults is clicked
{
    applyMainWindowSettings(KSharedConfig::openConfig()->group(QStringLiteral("MainWindow")));
}

void GUI::import()
{
    ImportDialog *dlg = new ImportDialog(core, this);
    dlg->show();
}

void GUI::setupActions()
{
    KActionCollection *ac = actionCollection();
    QAction *new_action = KStandardActions::openNew(this, &GUI::createTorrent, ac);
    new_action->setToolTip(i18n("Create a new torrent"));
    QAction *open_action = KStandardActions::open(this, &GUI::openTorrent, ac);
    open_action->setToolTip(i18n("Open a torrent"));
    paste_action = KStandardActions::paste(this, &GUI::paste, ac);

    open_silently_action = new QAction(open_action->icon(), i18n("Open Silently"), this);
    open_silently_action->setToolTip(i18n("Open a torrent without asking any questions"));
    connect(open_silently_action, &QAction::triggered, this, &GUI::openTorrentSilently);
    ac->addAction(QStringLiteral("file_open_silently"), open_silently_action);

    KStandardActions::quit(this, &GUI::quit, ac);

    show_status_bar_action = KStandardAction::showStatusbar(statusBar(), &GUI::setVisible, ac);
    show_status_bar_action->setIcon(QIcon::fromTheme(QStringLiteral("kt-show-statusbar")));

    show_menu_bar_action = KStandardAction::showMenubar(menuBar(), &GUI::setVisible, ac);
    KStandardActions::preferences(this, &GUI::showPrefDialog, ac);
    KStandardActions::keyBindings(this, &GUI::configureKeys, ac);
    KStandardActions::configureToolbars(this, &GUI::configureToolbars, ac);
    KStandardActions::configureNotifications(this, &GUI::configureNotifications, ac);

    paste_url_action = new QAction(QIcon::fromTheme(QStringLiteral("document-open-remote")), i18n("Open URL"), this);
    paste_url_action->setToolTip(i18n("Open a URL which points to a torrent, magnet links are supported"));
    connect(paste_url_action, &QAction::triggered, this, &GUI::pasteURL);
    ac->addAction(QStringLiteral("paste_url"), paste_url_action);
    ac->setDefaultShortcut(paste_url_action, QKeySequence(Qt::CTRL | Qt::Key_P));

    ipfilter_action = new QAction(QIcon::fromTheme(QStringLiteral("view-filter")), i18n("IP Filter"), this);
    ipfilter_action->setToolTip(i18n("Show the list of blocked IP addresses"));
    connect(ipfilter_action, &QAction::triggered, this, &GUI::showIPFilter);
    ac->addAction(QStringLiteral("ipfilter_action"), ipfilter_action);
    ac->setDefaultShortcut(ipfilter_action, QKeySequence(Qt::CTRL | Qt::Key_I));

    import_action = new QAction(QIcon::fromTheme(QStringLiteral("document-import")), i18n("Import Torrent"), this);
    import_action->setToolTip(i18n("Import a torrent"));
    connect(import_action, &QAction::triggered, this, &GUI::import);
    ac->addAction(QStringLiteral("import"), import_action);
    ac->setDefaultShortcut(import_action, QKeySequence(Qt::SHIFT | Qt::Key_I));

    show_kt_action = new QAction(QIcon::fromTheme(QStringLiteral("kt-show-hide")), i18n("Show/Hide KTorrent"), this);
    connect(show_kt_action, &QAction::triggered, this, &GUI::showOrHide);
    ac->addAction(QStringLiteral("show_kt"), show_kt_action);
    KGlobalAccel::setGlobalShortcut(show_kt_action, QKeySequence(Qt::ALT | Qt::SHIFT | Qt::Key_T));

    setStandardToolBarMenuEnabled(true);
}

void GUI::update()
{
    try {
        CurrentStats stats = core->getStats();
        status_bar->updateSpeed(stats.upload_speed, stats.download_speed);
        status_bar->updateTransfer(stats.bytes_uploaded, stats.bytes_downloaded);
        status_bar->updateDHTStatus(Globals::instance().getDHT().isRunning(), Globals::instance().getDHT().getStats());

        // All speed to Window status bar
        if (Settings::showTotalSpeedInTitle()) {
            QString down_up_speed = i18n("D: %1 | U: %2", BytesPerSecToString((double)stats.download_speed), BytesPerSecToString((double)stats.upload_speed));
            setCaption(down_up_speed);
        } else
            setCaption(core->getGroupManager()->allGroup()->groupName());

        tray_icon->updateStats(stats);
        core->updateGuiPlugins();
        torrent_activity->update();
    } catch (bt::Error &err) {
        Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.toString() << endl;
    }
}

void GUI::applySettings()
{
    // Apply GUI update interval
    timer.setInterval(Settings::guiUpdateInterval());
    if (Settings::showSystemTrayIcon()) {
        tray_icon->updateMaxRateMenus();
        tray_icon->show();
    } else
        tray_icon->hide();
}

void GUI::loadState(KSharedConfigPtr cfg)
{
    setAutoSaveSettings(QStringLiteral("MainWindow"), true);
    central->loadState(cfg);
    torrent_activity->loadState(cfg);

    KConfigGroup g = cfg->group(QStringLiteral("MainWindow"));
    bool statusbar_hidden = g.readEntry("statusbar_hidden", false);
    status_bar->setHidden(statusbar_hidden);
    show_status_bar_action->setChecked(!statusbar_hidden);

    bool menubar_hidden = g.readEntry("menubar_hidden", false);
    menuBar()->setHidden(menubar_hidden);
    show_menu_bar_action->setChecked(!menubar_hidden);

    bool minimize_to_system_tray = Settings::alwaysMinimizeToSystemTray();

    if (Settings::showSystemTrayIcon() && minimize_to_system_tray) {
        Out(SYS_GEN | LOG_DEBUG) << "Starting minimized" << endl;
        hide();
    } else {
        show();
    }

    setCurrentActivity(central->currentActivity());
}

void GUI::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group(QStringLiteral("MainWindow"));
    saveMainWindowSettings(g);
    g.writeEntry("statusbar_hidden", status_bar->isHidden());
    g.writeEntry("menubar_hidden", menuBar()->isHidden());
    torrent_activity->saveState(cfg);
    central->saveState(cfg);
    if (pref_dlg)
        pref_dlg->saveState(cfg);
    cfg->sync();
}

bool GUI::queryClose()
{
    if (isVisible()) {
        saveState(KSharedConfig::openConfig());
    }

    if (Settings::showSystemTrayIcon() && !qApp->isSavingSession()) {
        hide();
        return false;
    } else {
        timer.stop();
        QTimer::singleShot(500, qApp, &QCoreApplication::quit);
        return true;
    }
}

void GUI::quit()
{
    qApp->quit();
}

void GUI::updateActions()
{
    torrent_activity->updateActions();
}

void GUI::showOrHide()
{
    setVisible(!isVisible());
}

void GUI::configureNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void GUI::setPasteDisabled(bool on)
{
    paste_action->setEnabled(!on);
}

QWidget *GUI::container(const QString &name)
{
    return guiFactory()->container(name, this);
}

TorrentActivityInterface *GUI::getTorrentActivity()
{
    return torrent_activity;
}

}

#include "moc_gui.cpp"
