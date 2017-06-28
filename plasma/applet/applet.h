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

#ifndef KTPLASMAAPPLET_H
#define KTPLASMAAPPLET_H

#include <plasma/popupapplet.h>
#include <plasma/dataengine.h>
#include "ui_appletconfig.h"
#include "fadingnavigationwidget.h"

class QGraphicsLinearLayout;
class QDBusPendingCallWatcher;

namespace Plasma
{
#if (PLASMA_VERSION_MAJOR < 3)
    class Icon;
#else
    class IconWidget;
#endif
    class Label;
}


namespace ktplasma
{
    class ChunkBar;

    /**
        Plasma applet for ktorrent
    */
    class Applet : public Plasma::PopupApplet
    {
        Q_OBJECT

    public:
        Applet(QObject* parent, const QVariantList& args);
        virtual ~Applet();

        virtual void init();
        virtual void createConfigurationInterface(KConfigDialog* parent);
        virtual void saveState(KConfigGroup& config) const;
        virtual QGraphicsWidget* graphicsWidget();
        virtual void constraintsEvent(Plasma::Constraints constraints);

    private slots:
        void dataUpdated(const QString& name, const Plasma::DataEngine::Data& data);
        void configUpdated();
        void sourceAdded(const QString& s);
        void sourceRemoved(const QString& s);
        void iconClicked();
        void selectPrev();
        void selectNext();
        void dbusCallFinished(QDBusPendingCallWatcher* self);

    private:
        void updateTorrentCombo();
        void updateCurrent(const Plasma::DataEngine::Data& data);
        void setSource(QString source);
        void initSource();
        void clearData();
        void updateSources();
        void updateConnection(bool connected);
        void updateNavigation();

    private:
        Ui_AppletConfig ui;
        QGraphicsWidget* desktop_widget;
        QGraphicsLinearLayout* root_layout;
#if (PLASMA_VERSION_MAJOR < 3)
        Plasma::Icon* icon;
#else
        Plasma::IconWidget* icon;
#endif
        Plasma::Label* title;
        Plasma::Label* misc;
        ChunkBar* chunk_bar;
        FadingNavigationWidget* navigation;

        Plasma::DataEngine* engine;
        bool connected_to_app;
        QString current_source;
        QStringList sources;
    };

}



#endif
