/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IPBLOCKINGPREFPAGE_H
#define IPBLOCKINGPREFPAGE_H

#include "ipfilterplugin.h"
#include "ui_ipblockingprefpage.h"
#include <QThread>
#include <interfaces/coreinterface.h>
#include <interfaces/prefpageinterface.h>

class KJob;

namespace kt
{
class IPFilterPlugin;
class DownloadAndConvertJob;

/**
 * @author Ivan Vasic
 * @brief IPBlocking plugin interface page
 **/
class IPBlockingPrefPage : public PrefPageInterface, public Ui_IPBlockingPrefPage
{
    Q_OBJECT
public:
    IPBlockingPrefPage(IPFilterPlugin *p);
    ~IPBlockingPrefPage() override;

    void loadSettings() override;
    void loadDefaults() override;
    void updateSettings() override;

    /// Do an auto update, return false if this is not possible
    bool doAutoUpdate();

private Q_SLOTS:
    void downloadClicked();
    void checkUseLevel1Toggled(bool);
    void restoreGUI();
    void downloadAndConvertFinished(KJob *j);
    void autoUpdateToggled(bool on);
    void autoUpdateIntervalChanged(int val);

private:
    void updateAutoUpdate();

Q_SIGNALS:
    void updateFinished();

private:
    CoreInterface *m_core;
    IPFilterPlugin *m_plugin;
    DownloadAndConvertJob *m_job;
    bool m_verbose;
};
}
#endif
