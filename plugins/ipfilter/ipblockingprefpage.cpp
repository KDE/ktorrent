/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <KLocalizedString>

#include <util/log.h>
#include "ipblockingprefpage.h"
#include "ipfilterpluginsettings.h"
#include "ipfilterplugin.h"
#include "downloadandconvertjob.h"


using namespace bt;

namespace kt
{

    IPBlockingPrefPage::IPBlockingPrefPage(IPFilterPlugin* p)
        : PrefPageInterface(IPBlockingPluginSettings::self(), i18n("IP Filter"), QStringLiteral("view-filter"), nullptr), m_plugin(p)
    {
        setupUi(this);
        connect(kcfg_useLevel1, &QCheckBox::toggled, this, &IPBlockingPrefPage::checkUseLevel1Toggled);
        connect(m_download, &QPushButton::clicked, this, &IPBlockingPrefPage::downloadClicked);
        connect(kcfg_autoUpdate, &QCheckBox::toggled, this, &IPBlockingPrefPage::autoUpdateToggled);
        connect(kcfg_autoUpdateInterval, static_cast<void(KPluralHandlingSpinBox::*)(int)>(&KPluralHandlingSpinBox::valueChanged), this, &IPBlockingPrefPage::autoUpdateIntervalChanged);
        kcfg_autoUpdateInterval->setSuffix(ki18np(" day", " days"));
        m_job = nullptr;
        m_verbose = true;
    }

    IPBlockingPrefPage::~IPBlockingPrefPage()
    {}

    void IPBlockingPrefPage::checkUseLevel1Toggled(bool check)
    {
        if (check)
        {
            kcfg_filterURL->setEnabled(true);
            m_download->setEnabled(true);
            m_plugin->loadAntiP2P();
        }
        else
        {
            m_status->setText(QString());
            kcfg_filterURL->setEnabled(false);
            m_download->setEnabled(false);
            m_plugin->unloadAntiP2P();
        }

        if (m_plugin->loadedAndRunning() && check)
            m_status->setText(i18n("Status: Loaded and running."));
        else
            m_status->setText(i18n("Status: Not loaded."));

        updateAutoUpdate();
    }

    void IPBlockingPrefPage::loadDefaults()
    {
        loadSettings();
    }

    void IPBlockingPrefPage::updateSettings()
    {
        m_plugin->checkAutoUpdate();
    }

    void IPBlockingPrefPage::loadSettings()
    {
        if (IPBlockingPluginSettings::useLevel1())
        {
            if (m_plugin->loadedAndRunning())
                m_status->setText(i18n("Status: Loaded and running."));
            else
                m_status->setText(i18n("Status: Not loaded."));

            kcfg_filterURL->setEnabled(true);
            m_download->setEnabled(true);
            m_last_updated->clear();
            m_next_update->clear();
            kcfg_autoUpdateInterval->setEnabled(IPBlockingPluginSettings::autoUpdate());
            m_auto_update_group_box->setEnabled(true);
        }
        else
        {
            m_status->setText(i18n("Status: Not loaded."));
            kcfg_filterURL->setEnabled(false);
            m_download->setEnabled(false);
            m_last_updated->clear();
            m_next_update->clear();
            kcfg_autoUpdateInterval->setEnabled(IPBlockingPluginSettings::autoUpdate());
            m_auto_update_group_box->setEnabled(false);
        }

        updateAutoUpdate();
    }

    void IPBlockingPrefPage::downloadClicked()
    {
        QUrl url = kcfg_filterURL->url();

        // block GUI so you cannot do stuff during conversion
        m_download->setEnabled(false);
        m_status->setText(i18n("Status: Downloading and converting new block list..."));
        kcfg_useLevel1->setEnabled(false);
        kcfg_filterURL->setEnabled(false);

        m_plugin->unloadAntiP2P();
        m_job = new DownloadAndConvertJob(url, m_verbose ? DownloadAndConvertJob::Verbose : DownloadAndConvertJob::Quietly);
        connect(m_job, &DownloadAndConvertJob::result, this, &IPBlockingPrefPage::downloadAndConvertFinished);
        connect(m_job, &DownloadAndConvertJob::notification, m_plugin, &IPFilterPlugin::notification);
        m_job->start();
    }

    bool IPBlockingPrefPage::doAutoUpdate()
    {
        if (m_job)
        {
            if (m_job->isAutoUpdate())
                return true; // if we are already auto updating, lets not start it again
            else
                return false;
        }

        m_verbose = false;
        Out(SYS_IPF | LOG_NOTICE) << "Doing ipfilter auto update !" << endl;
        downloadClicked();
        m_verbose = true;
        return true;
    }

    void IPBlockingPrefPage::restoreGUI()
    {
        m_download->setEnabled(true);
        kcfg_useLevel1->setEnabled(true);
        kcfg_filterURL->setEnabled(true);

        if (m_plugin->loadedAndRunning())
            m_status->setText(i18n("Status: Loaded and running."));
        else
            m_status->setText(i18n("Status: Not loaded."));
    }

    void IPBlockingPrefPage::downloadAndConvertFinished(KJob* j)
    {
        if (j != m_job)
            return;

        KConfigGroup g = KSharedConfig::openConfig()->group("IPFilterAutoUpdate");
        if (!j->error())
        {
            g.writeEntry("last_updated", QDateTime::currentDateTime());
            g.writeEntry("last_update_ok", true);
        }
        else
        {
            g.writeEntry("last_update_attempt", QDateTime::currentDateTime());
            g.writeEntry("last_update_ok", false);
        }

        g.sync();

        m_job = 0;
        m_plugin->loadAntiP2P();
        restoreGUI();
        updateAutoUpdate();
        updateFinished();
    }

    void IPBlockingPrefPage::updateAutoUpdate()
    {
        if (!kcfg_useLevel1->isChecked())
        {
            m_next_update->clear();
            m_last_updated->clear();
            return;
        }

        KConfigGroup g = KSharedConfig::openConfig()->group("IPFilterAutoUpdate");
        bool ok = g.readEntry("last_update_ok", true);
        QDate last_updated = g.readEntry("last_updated", QDate());

        if (last_updated.isNull())
            m_last_updated->setText(i18n("No update done yet."));
        else if (ok)
            m_last_updated->setText(last_updated.toString());
        else
            m_last_updated->setText(i18n("%1 (Last update attempt failed.)", last_updated.toString()));

        if (kcfg_autoUpdate->isChecked())
        {
            QDate next_update;
            if (last_updated.isNull())
                next_update = QDate::currentDate().addDays(kcfg_autoUpdateInterval->value());
            else
                next_update = last_updated.addDays(kcfg_autoUpdateInterval->value());

            m_next_update->setText(next_update.toString());
        }
        else
        {
            m_next_update->setText(i18n("Never"));
        }
    }

    void IPBlockingPrefPage::autoUpdateToggled(bool on)
    {
        Q_UNUSED(on);
        updateAutoUpdate();
    }

    void IPBlockingPrefPage::autoUpdateIntervalChanged(int val)
    {
        Q_UNUSED(val);
        updateAutoUpdate();
    }

}

