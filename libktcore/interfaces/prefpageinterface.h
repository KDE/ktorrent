/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#ifndef PREFPAGEINTERFACE_H
#define PREFPAGEINTERFACE_H

#include <QWidget>
#include <ktcore_export.h>

class KConfigSkeleton;

namespace kt
{
    /**
     * @author Ivan Vasic
     * @brief Interface to add configuration dialog page.
     *
     * This interface allows plugins and others to add their own pages in Configuration dialog.
     */
    class KTCORE_EXPORT PrefPageInterface : public QWidget
    {
        Q_OBJECT
    public:
        PrefPageInterface(KConfigSkeleton* cfg, const QString& name, const QString& icon, QWidget* parent);
        virtual ~PrefPageInterface();

        /**
         * Initialize the settings.
         * Called by the settings dialog when it is created.
         */
        virtual void loadSettings();

        /**
         * Load default settings.
         * Called when the defaults button is pressed in the settings dialog.
         */
        virtual void loadDefaults();

        /**
         * Called when user presses OK or apply.
         */
        virtual void updateSettings();

        KConfigSkeleton* config() {return cfg;}
        const QString& pageName() {return name;}
        const QString& pageIcon() {return icon;}

        /// Override if there are custom widgets outside which have changed
        virtual bool customWidgetsChanged() {return false;}

    signals:
        /// Emitted when buttons need to be updated
        void updateButtons();

    private:
        KConfigSkeleton* cfg;
        QString name;
        QString icon;
    };
}
#endif

