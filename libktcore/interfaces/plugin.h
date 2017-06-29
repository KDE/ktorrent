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

#ifndef KTPLUGIN_H
#define KTPLUGIN_H

#include <ktversion.h>
#include <ktcore_export.h>
#include <KParts/Plugin>

namespace bt
{
    class WaitJob;
}

namespace kt
{
    class CoreInterface;
    class GUIInterface;

    /**
     * @author Joris Guisson
     * @brief Base class for all plugins
     *
     * This is the base class for all plugins. Plugins should implement
     * the load and unload methods, any changes made in load must be undone in
     * unload.
     *
     * It's also absolutely forbidden to do any complex initialization in the constructor
     * (setting an int to 0 is ok, creating widgets isn't).
     * Only the name, author and description may be set in the constructor.
     */
    class KTCORE_EXPORT Plugin : public KParts::Plugin
    {
        Q_OBJECT
    public:
        Plugin(QObject* parent);
        virtual ~Plugin();

        /**
         * This gets called, when the plugin gets loaded by KTorrent.
         * Any changes made here must be later made undone, when unload is
         * called.
         * Upon error a bt::Error should be thrown. And the plugin should remain
         * in an uninitialized state. The Error contains an error message, which will
         * get show to the user.
         */
        virtual void load() = 0;

        /**
         * Gets called when the plugin gets unloaded.
         * Should undo anything load did.
         */
        virtual void unload() = 0;

        /**
         * For plugins who need to update something, the same time as the
         * GUI updates.
         */
        virtual void guiUpdate();

        /**
         * This should be implemented by plugins who need finish of some stuff which might take some time.
         * These operations must be finished or killed by a timeout before we can proceed with unloading the plugin.
         * @param job The WaitJob which monitors the plugin
         */
        virtual void shutdown(bt::WaitJob* job);

        /// Get a pointer to the CoreInterface
        CoreInterface* getCore() {return core;}

        /// Get a const pointer to the CoreInterface
        const CoreInterface* getCore() const {return core;}

        /**
         * Set the core, used by PluginManager to set the pointer
         * to the core.
         * @param c Pointer to the core
         */
        void setCore(CoreInterface* c) {core = c;}

        /// Get a pointer to the CoreInterface
        GUIInterface* getGUI() {return gui;}

        /// Get a const pointer to the CoreInterface
        const GUIInterface* getGUI() const {return gui;}

        /**
         * Set the core, used by PluginManager to set the pointer
         * to the core.
         * @param c Pointer to the core
         */
        void setGUI(GUIInterface* c) {gui = c;}

        /// See if the plugin is loaded
        bool isLoaded() const {return loaded;}

        /// Check whether the plugin matches the version of KT
        virtual bool versionCheck(const QString& version) const = 0;

        /// Returns the name of the parent part the GUI of the plugin should be created in
        virtual QString parentPart() const {return QStringLiteral("ktorrent");}

    private:
        CoreInterface* core;
        GUIInterface* gui;
        bool loaded;

        friend class PluginManager;
    };

}

#endif
