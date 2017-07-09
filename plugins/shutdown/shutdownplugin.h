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

#ifndef KT_SHUTDOWNPLUGIN_H
#define KT_SHUTDOWNPLUGIN_H

#include <interfaces/plugin.h>

class KToggleAction;

namespace kt
{
    class ShutdownRuleSet;

    class ShutdownPlugin : public kt::Plugin
    {
        Q_OBJECT
    public:
        ShutdownPlugin(QObject* parent, const QVariantList& args);
        ~ShutdownPlugin();

        bool versionCheck(const QString& version) const override;
        void unload() override;
        void load() override;

    public slots:
        void shutdownComputer();
        void lock();
        void suspendToDisk();
        void suspendToRam();

    private slots:
        void shutdownToggled(bool on);
        void configureShutdown();
        void updateAction();

    private:
        KToggleAction* shutdown_enabled;
        QAction * configure_shutdown;
        ShutdownRuleSet* rules;
    };

}

#endif // KT_SHUTDOWNPLUGIN_H
