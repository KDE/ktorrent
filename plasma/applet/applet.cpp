/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include "applet.h"

#include <KConfigDialog>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QFile>
#include <QGraphicsLinearLayout>
#include <KLocalizedString>
#include <KRun>
#include <KWindowSystem>

#if (PLASMA_VERSION_MAJOR < 3)
#include <Plasma/Icon>
#else
#include <Plasma/IconWidget>
#endif
#include <Plasma/Label>
#include <util/functions.h>
#include "chunkbar.h"
#include "fadingnavigationwidget.h"


using namespace bt;

namespace ktplasma
{

    Applet::Applet(QObject* parent, const QVariantList& args) : Plasma::PopupApplet(parent, args), icon(0)
    {
        engine = 0;
        root_layout = 0;
        desktop_widget = 0;
        connected_to_app = false;

        // drop data (dragged from ktorrent)
        if (!args.isEmpty())
        {
            QFile f(args[0].toString());
            if (f.open(QIODevice::ReadOnly))
            {
                QDataStream s(&f);
                s >> current_source;
            }
        }

        KGlobal::locale()->insertCatalog("ktorrent");
        setHasConfigurationInterface(true);
        setAspectRatioMode(Plasma::IgnoreAspectRatio);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        setPopupIcon("ktorrent");
    }

    Applet::~Applet()
    {
    }

    void Applet::init()
    {
        // note: counterintuitively, this method may be called more than once
        // (such as when moving the applet from panel to desktop)

        desktop_widget = graphicsWidget();
        clearData();
        if (!engine)
        {
            engine = dataEngine("ktorrent");
            connect(engine, &Plasma::DataEngine::sourceAdded, this, &Applet::sourceAdded);
            connect(engine, &Plasma::DataEngine::sourceRemoved, this, &Applet::sourceRemoved);
            engine->connectSource("core", this);
        }
    }

    QGraphicsWidget* Applet::graphicsWidget()
    {
        if (desktop_widget)
            return desktop_widget;

        root_layout = new QGraphicsLinearLayout(Qt::Vertical);
        root_layout->setOrientation(Qt::Vertical);

        QGraphicsLinearLayout* line = new QGraphicsLinearLayout(0);

#if (PLASMA_VERSION_MAJOR < 3)
        icon = new Plasma::Icon(QIcon::fromTheme("ktorrent"), QString(), this);
#else
        icon = new Plasma::IconWidget(QIcon::fromTheme("ktorrent"), QString(), this);
#endif
        int icon_size = IconSize(KIconLoader::Desktop);
        icon->setMaximumSize(icon_size, icon_size);
        icon->setMinimumSize(icon_size, icon_size);
        icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(icon, &Plasma::IconWidget::clicked, this, &Applet::iconClicked);

        title = new Plasma::Label(this);
        title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        title->setAcceptedMouseButtons(0); // enable moving applet by dragging it
        line->addItem(icon);
        line->addItem(title);
        root_layout->addItem(line);

        chunk_bar = new ChunkBar(this);
        root_layout->addItem(chunk_bar);

        misc = new Plasma::Label(this);
        misc->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        misc->setAcceptedMouseButtons(0); // enable moving applet by dragging it
        misc->setMinimumWidth(330); // to prevent table from breaking up
        misc->setPreferredHeight(80);
        root_layout->addItem(misc);

        desktop_widget = new QGraphicsWidget(this);
        desktop_widget->setLayout(root_layout);
        desktop_widget->adjustSize(); // necessary for arrow location calculation

        // parent is widget (not applet), prevents show/hide flickering
        navigation = new FadingNavigationWidget(desktop_widget);
        connect(navigation, &FadingNavigationWidget::prevClicked, this, &Applet::selectPrev);
        connect(navigation, &FadingNavigationWidget::nextClicked, this, &Applet::selectNext);

        return desktop_widget;
    }

    void Applet::selectPrev()
    {
        if (sources.empty())
        {
            clearData();
        }
        else
        {
            int i = sources.indexOf(current_source) - 1 + sources.count();
            setSource(sources[i % sources.count()]);
        }
    }

    void Applet::selectNext()
    {
        if (sources.empty())
        {
            clearData();
        }
        else
        {
            int i = sources.indexOf(current_source) + 1;
            setSource(sources[i % sources.count()]);
        }
    }

    void Applet::constraintsEvent(Plasma::Constraints constraints)
    {
        Plasma::PopupApplet::constraintsEvent(constraints);
        if (constraints & Plasma::SizeConstraint)
        {
            // reposition arrows relative to widget (not applet)
            QPointF pos((desktop_widget->size().width() - navigation->frame()->size().width()) / 2,
                        desktop_widget->contentsRect().bottom() - navigation->frame()->size().height() - 5);
            navigation->frame()->setPos(pos);
        }
    }

    void Applet::updateNavigation()
    {
        navigation->setEnabled(connected_to_app && !sources.empty()
                               && (sources.count() > 1 || !sources.contains(current_source)));
    }

    void Applet::updateConnection(bool connected)
    {
        connected_to_app = connected;
        clearData();
        updateNavigation();
        if (connected)
        {
            if (current_source.isEmpty()) // don't override dragged item
                current_source = config().readEntry("current_source", QString());
            initSource();
            // addSource will be called for each sorce if reconnected
        }
    }

    void Applet::updateSources()
    {
        sources = engine->sources();
        sources.removeOne("core");
    }

    void Applet::setSource(QString source)
    {
        if (!current_source.isEmpty())
            engine->disconnectSource(current_source, this);
        clearData();
        current_source = source;
        engine->connectSource(current_source, this, 1000);
        config().writeEntry("current_source", current_source);
        config().sync();
        updateNavigation();
    }

    void Applet::initSource()
    {
        updateSources();
        if (sources.contains(current_source))
            setSource(current_source);
        else if (!sources.empty())
            setSource(sources[0]);
        else
            clearData();
    }

    void Applet::saveState(KConfigGroup& config) const
    {
        Q_UNUSED(config);
    }

    void Applet::createConfigurationInterface(KConfigDialog* parent)
    {
        QWidget* widget = new QWidget();
        ui.setupUi(widget);
        updateTorrentCombo(); // must come before addPage for size to be correct
        parent->setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Apply);
        parent->addPage(widget, i18n("Applet"), "ktorrent");
        connect(parent, SIGNAL(applyClicked()), this, SLOT(configUpdated()));
        connect(parent, SIGNAL(okClicked()), this, SLOT(configUpdated()));
    }

    void Applet::updateTorrentCombo()
    {
        updateSources();
        ui.torrent_to_display->clear();
        ui.torrent_to_display->setEnabled(!sources.empty());
        if (sources.empty())
            return;
        QStringList names;
        foreach (const QString& s, sources)
            names << engine->query(s).value("name").toString();
        ui.torrent_to_display->addItems(names);
        if (current_source.isEmpty())
            initSource();
    }

    void Applet::configUpdated()
    {
        QString name = ui.torrent_to_display->currentText();
        foreach (const QString& s, sources)
        {
            if (engine->query(s).value("name").toString() == name)
            {
                setSource(s);
                break;
            }
        }
    }

    void Applet::dataUpdated(const QString& name, const Plasma::DataEngine::Data& data)
    {
        if (name == "core")
        {
            bool connected = data.value("connected").toBool();
            if (connected_to_app != connected)
                updateConnection(connected);
        }
        else if (name == current_source)
        {
            updateCurrent(data);
        }
    }

    void Applet::updateCurrent(const Plasma::DataEngine::Data& data)
    {
        double ds = data.value("download_rate").toDouble();
        double us = data.value("upload_rate").toDouble();
        qlonglong uploaded = data.value("bytes_uploaded").toLongLong();
        qlonglong downloaded = data.value("bytes_downloaded").toLongLong();
        qlonglong size = data.value("total_bytes_to_download").toLongLong();
        int st = data.value("seeders_total").toInt();
        int sc = data.value("seeders_connected_to").toInt();
        int lt = data.value("leechers_total").toInt();
        int lc = data.value("leechers_connected_to").toInt();
        int cd = data.value("num_chunks_downloaded").toInt();
        int ct = data.value("total_chunks").toInt();
        KLocale* loc = KGlobal::locale();
        float share_ratio = (downloaded == 0) ? 0 : (float)uploaded / downloaded;
        float percent = 100.0 * cd / ct;
        misc->setText(
            i18n(
                "<table>\
				<tr><td>Download Speed:</td><td>%5 </td><td>Seeders: </td><td>%1 (%2)</td></tr>\
				<tr><td>Upload Speed:</td><td>%6 </td><td>Leechers: </td><td>%3 (%4)</td></tr>",
                sc, st, lc, lt, BytesPerSecToString(ds), BytesPerSecToString(us)) +
            i18n(
                "<tr><td>Downloaded:</td><td>%1 </td><td>Size: </td><td>%2</td></tr>\
				<tr><td>Uploaded:</td><td>%3 </td><td>Complete: </td><td>%4 %</td></tr>\
				</table>",
                BytesToString(downloaded), BytesToString(size), BytesToString(uploaded),
                loc->formatNumber(percent, 2)));
        title->setText(
            i18n("<b>%1</b><br/>%2 (Share Ratio: <font color=\"%4\">%3</font>)",
                 data.value("name").toString(),
                 data.value("status").toString(),
                 loc->formatNumber(share_ratio, 2),
                 share_ratio <= 0.8 ? "#ff0000" : "#1c9a1c"));
        chunk_bar->updateBitSets(
            data.value("total_chunks").toInt(),
            data.value("downloaded_chunks").toByteArray(),
            data.value("excluded_chunks").toByteArray());
    }

    void Applet::sourceAdded(const QString& s)
    {
        // note: we get this event for each source also when app reconnects
        if (!sources.contains(s))
            sources.append(s);
        if (current_source.isEmpty() || current_source == s)
            initSource();
        else if (!sources.contains(current_source))
            clearData();
        updateNavigation();
    }

    void Applet::sourceRemoved(const QString& s)
    {
        // note: we get this event for each source also when app disconnects
        sources.removeOne(s);
        if (current_source == s)
            clearData();
        updateNavigation();
    }

    void Applet::iconClicked()
    {
        QDBusConnection session_bus = QDBusConnection::sessionBus();
        QDBusConnectionInterface* dbus_service = session_bus.interface();
        if (!session_bus.isConnected() || !dbus_service || !dbus_service->isServiceRegistered("org.ktorrent.ktorrent"))
        {
            // can't find the window, try launching it
            KUrl::List empty;
            KRun::run("ktorrent", empty, 0);
        }
        else
        {
            QDBusMessage msg = QDBusMessage::createMethodCall("org.ktorrent.ktorrent", "/ktorrent/MainWindow_1", "org.kde.KMainWindow", "winId");
            QDBusPendingCall call = session_bus.asyncCall(msg, 5000);
            QDBusPendingCallWatcher* watcher = new QDBusPendingCallWatcher(call , this);
            connect(watcher, &QDBusPendingCallWatcher::finished, this, &Applet::dbusCallFinished);
        }
    }

    void Applet::dbusCallFinished(QDBusPendingCallWatcher* self)
    {
        if (self->isError())
        {
            // call failed, try launching it
            KUrl::List empty;
            KRun::run("ktorrent", empty, 0);
        }
        else
        {
            QDBusPendingReply<qlonglong> reply = *self;
            KWindowSystem::activateWindow(reply.value());
        }
        self->deleteLater();
    }


    void Applet::clearData()
    {
        KLocale* loc = KGlobal::locale();
        misc->setText(
            i18n(
                "<table>\
				<tr><td>Download Speed:</td><td>%5 </td><td>Seeders: </td><td>%1 (%2)</td></tr>\
				<tr><td>Upload Speed:</td><td>%6 </td><td>Leechers: </td><td>%3 (%4)</td></tr>",
                0, 0, 0, 0, BytesPerSecToString(0), BytesPerSecToString(0)) +
            i18n(
                "<tr><td>Downloaded:</td><td>%1 </td><td>Size: </td><td>%2</td></tr>\
				<tr><td>Uploaded:</td><td>%3 </td><td>Complete: </td><td>%4 %</td></tr>\
				</table>",
                BytesToString(0), BytesToString(0), BytesToString(0),
                loc->formatNumber(0, 2)));
        if (!connected_to_app)
            title->setText(i18n("KTorrent is not running."));
        else if (sources.empty())
            title->setText(i18n("No torrents loaded."));
        else if (!sources.contains(current_source))
            title->setText(i18n("Selected torrent is unavailable."));
        else
            title->setText(QString());
        chunk_bar->updateBitSets(1, QByteArray(1, 0), QByteArray(1, 0)); // no chunks
    }
}

K_EXPORT_PLASMA_APPLET(ktorrent, ktplasma::Applet)
