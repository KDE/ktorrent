/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "logprefpage.h"

#include <KSharedConfig>
#include <QHeaderView>

#include "logflags.h"
#include "logflagsdelegate.h"
#include "logviewerpluginsettings.h"

namespace kt
{
LogPrefPage::LogPrefPage(LogFlags *flags, QWidget *parent)
    : PrefPageInterface(LogViewerPluginSettings::self(), i18n("Log Viewer"), QStringLiteral("utilities-log-viewer"), parent)
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
    if (!state_loaded) {
        loadState();
        state_loaded = true;
    }
}

void LogPrefPage::loadSettings()
{
    if (!state_loaded) {
        loadState();
        state_loaded = true;
    }
}

void LogPrefPage::updateSettings()
{
}
}
