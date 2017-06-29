/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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

#include "logprefpage.h"

#include <QHeaderView>
#include <KSharedConfig>

#include "logviewerpluginsettings.h"
#include "logflags.h"
#include "logflagsdelegate.h"

namespace kt
{
    LogPrefPage::LogPrefPage(LogFlags* flags, QWidget* parent) : PrefPageInterface(LogViewerPluginSettings::self(), i18n("Log Viewer"), QStringLiteral("utilities-log-viewer"), parent)
    {
        setupUi(this);
        m_logging_flags->setModel(flags);
        m_logging_flags->setItemDelegate(new LogFlagsDelegate(this));
        state_loaded = false;
    }

    LogPrefPage::~LogPrefPage()
    {
    }

    void LogPrefPage::saveState()
    {
        KConfigGroup g = KSharedConfig::openConfig()->group("LogFlags");
        QByteArray s = m_logging_flags->header()->saveState();
        g.writeEntry("logging_flags_view_state", s.toBase64());
        g.sync();
    }

    void LogPrefPage::loadState()
    {
        KConfigGroup g = KSharedConfig::openConfig()->group("LogFlags");
        QByteArray s = QByteArray::fromBase64(g.readEntry("logging_flags_view_state", QByteArray()));
        if (!s.isEmpty())
            m_logging_flags->header()->restoreState(s);
    }

    void LogPrefPage::loadDefaults()
    {
        if (!state_loaded)
        {
            loadState();
            state_loaded = true;
        }
    }

    void LogPrefPage::loadSettings()
    {
        if (!state_loaded)
        {
            loadState();
            state_loaded = true;
        }
    }

    void LogPrefPage::updateSettings()
    {
    }
}
