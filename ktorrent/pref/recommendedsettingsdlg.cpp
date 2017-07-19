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

#include <cmath>

#include <KFormat>
#include <KSharedConfig>

#include <util/constants.h>
#include <util/functions.h>
#include "recommendedsettingsdlg.h"
#include "settings.h"

using namespace bt;

namespace kt
{

    RecommendedSettingsDlg::RecommendedSettingsDlg(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle(i18n("Calculate Recommended Settings"));
        setupUi(this);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
        connect(m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &RecommendedSettingsDlg::apply);
        connect(m_calculate, &QPushButton::clicked, this, &RecommendedSettingsDlg::calculate);
        connect(m_chk_avg_speed_slot, &QCheckBox::toggled, this, &RecommendedSettingsDlg::avgSpeedSlotToggled);
        connect(m_chk_sim_torrents, &QCheckBox::toggled, this, &RecommendedSettingsDlg::simTorrentsToggled);
        connect(m_chk_slots, &QCheckBox::toggled, this, &RecommendedSettingsDlg::slotsToggled);
        connect(m_upload_bw, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RecommendedSettingsDlg::uploadBWChanged);
        connect(m_download_bw, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &RecommendedSettingsDlg::downloadBWChanged);

        m_avg_speed_slot->setEnabled(false);
        m_slots->setEnabled(false);
        m_sim_torrents->setEnabled(false);

        loadState(KSharedConfig::openConfig());

        calculate();
    }


    RecommendedSettingsDlg::~RecommendedSettingsDlg()
    {
    }

    void RecommendedSettingsDlg::apply()
    {
        saveState(KSharedConfig::openConfig());
        /*  Settings::setMaxDownloadRate(max_download_speed);
            Settings::setMaxUploadRate(max_upload_speed);
            Settings::setMaxConnections(max_conn_tor);
            Settings::setMaxTotalConnections(max_conn_glob);
            Settings::setNumUploadSlots(max_slots);
            Settings::setMaxDownloads(max_downloads);
            Settings::setMaxSeeds(max_seeds);*/
        QDialog::accept();
    }

    void RecommendedSettingsDlg::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("RecommendedSettingsDlg");
        g.writeEntry("upload_bw", m_upload_bw->value());
        g.writeEntry("download_bw", m_download_bw->value());
        g.writeEntry("avg_speed_slot_enabled", m_chk_avg_speed_slot->isChecked());
        g.writeEntry("avg_speed_slot", m_avg_speed_slot->value());
        g.writeEntry("slots_enabled", m_chk_slots->isChecked());
        g.writeEntry("slots", m_slots->value());
        g.writeEntry("sim_torrents_enabled", m_chk_sim_torrents->isChecked());
        g.writeEntry("sim_torrents", m_sim_torrents->value());

    }

    void RecommendedSettingsDlg::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("RecommendedSettingsDlg");
        m_upload_bw->setValue(g.readEntry("upload_bw", 256));
        m_download_bw->setValue(g.readEntry("download_bw", 4000));
        m_chk_avg_speed_slot->setChecked(g.readEntry("avg_speed_slot_enabled", false));
        m_avg_speed_slot->setValue(g.readEntry("avg_speed_slot", 4));
        m_chk_slots->setChecked(g.readEntry("slots_enabled", false));
        m_slots->setValue(g.readEntry("slots", 3));
        m_chk_sim_torrents->setChecked(g.readEntry("sim_torrents_enabled", false));
        m_sim_torrents->setValue(g.readEntry("sim_torrents", 2));

        uploadBWChanged(m_upload_bw->value());
        downloadBWChanged(m_download_bw->value());
    }

    void RecommendedSettingsDlg::calculate()
    {
        Uint32 upload_rate = m_upload_bw->value() / 8;
        Uint32 download_rate = m_download_bw->value() / 8;

        max_upload_speed = floor(upload_rate * 0.8);
        max_download_speed = floor(download_rate * 0.8);

        qreal avg_slot_up = ceil(qMax(pow(upload_rate / 3.5, 0.55), 4.0));

        Uint32 max_torrents = qRound(pow(upload_rate * 0.25, 0.3));
        max_downloads = ceil((float)(max_torrents * 2 / 3));
        max_seeds = qMax(max_torrents - max_downloads, (bt::Uint32)1);

        if (m_chk_avg_speed_slot->isChecked())
            avg_slot_up = (qreal)m_avg_speed_slot->value();

        if (m_chk_sim_torrents->isChecked())
        {
            max_downloads = m_sim_torrents->value();
            max_torrents = floor(max_downloads * 1.33);
            max_seeds = qMax(max_torrents - max_downloads, (bt::Uint32)1);
        }

        max_slots = floor(upload_rate / (max_torrents * avg_slot_up));

        if (m_chk_slots->isChecked())
            max_slots = m_slots->value();


        if (m_chk_avg_speed_slot->isChecked() && m_chk_sim_torrents->isChecked())
        {
            max_slots = floor(max_upload_speed / (max_torrents * avg_slot_up));
        }
        else if (m_chk_avg_speed_slot->isChecked() && m_chk_slots->isChecked())
        {
            max_torrents = qRound(max_upload_speed / (avg_slot_up * max_slots));
            max_downloads = ceil((float)(max_torrents * 2 / 3));
            max_seeds = qMax(max_torrents - max_downloads, (bt::Uint32)1);
        }
        else if (m_chk_sim_torrents->isChecked() && m_chk_slots->isChecked())
        {
            avg_slot_up = ceil((float)(max_upload_speed / (max_slots * max_torrents)));
        }
        else if (m_chk_slots->isChecked())
        {
            avg_slot_up = ceil(qMax(pow(max_upload_speed / 3.5, 0.55), 4.0)); // basis to calculate the number of torrents
            max_torrents = qRound(max_upload_speed / (avg_slot_up * max_slots));
            max_downloads = ceil((float)(max_torrents * 2 / 3));
            max_seeds = qMax(max_torrents - max_downloads, (bt::Uint32)1);
            avg_slot_up = ceil((float)(max_upload_speed / (max_slots * max_torrents))); // real number after the slots have been multiplied with the torrents
        }

        if (max_downloads == 0)
            max_downloads = 1;

        max_conn_glob = qRound(qMin((double)pow((int)(upload_rate * 8), 0.8) + 50, 900.0));
        max_conn_tor = qRound(qMin((qreal)(max_conn_glob * 1.2 / max_torrents), (qreal)max_conn_glob));

        m_max_upload->setText(QStringLiteral("<b>%1</b>").arg(BytesPerSecToString(max_upload_speed * 1024)));
        m_max_download->setText(QStringLiteral("<b>%1</b>").arg(BytesPerSecToString(max_download_speed * 1024)));
        m_max_conn_per_torrent->setText(QStringLiteral("<b>%1</b>").arg(max_conn_tor));
        m_max_conn_global->setText(QStringLiteral("<b>%1</b>").arg(max_conn_glob));
        m_max_downloads->setText(QStringLiteral("<b>%1</b>").arg(max_downloads));
        m_max_seeds->setText(QStringLiteral("<b>%1</b>").arg(max_seeds));
        m_upload_slots->setText(QStringLiteral("<b>%1</b>").arg(max_slots));
    }

    void RecommendedSettingsDlg::avgSpeedSlotToggled(bool on)
    {
        m_avg_speed_slot->setEnabled(on);
        if (on && m_chk_slots->isChecked())
        {
            m_chk_sim_torrents->setEnabled(false);
            m_sim_torrents->setEnabled(false);
        }
        else if (on && m_chk_sim_torrents->isChecked())
        {
            m_chk_slots->setEnabled(false);
            m_slots->setEnabled(false);
        }
        else
        {
            m_chk_slots->setEnabled(true);
            m_slots->setEnabled(m_chk_slots->isChecked());
            m_chk_sim_torrents->setEnabled(true);
            m_sim_torrents->setEnabled(m_chk_sim_torrents->isChecked());
            m_chk_avg_speed_slot->setEnabled(true);
        }
    }

    void RecommendedSettingsDlg::simTorrentsToggled(bool on)
    {
        m_sim_torrents->setEnabled(on);
        if (on && m_chk_slots->isChecked())
        {
            m_chk_avg_speed_slot->setEnabled(false);
            m_avg_speed_slot->setEnabled(false);
        }
        else if (on && m_chk_avg_speed_slot->isChecked())
        {
            m_chk_slots->setEnabled(false);
            m_slots->setEnabled(false);
        }
        else
        {
            m_chk_slots->setEnabled(true);
            m_slots->setEnabled(m_chk_slots->isChecked());
            m_chk_sim_torrents->setEnabled(true);
            m_chk_avg_speed_slot->setEnabled(true);
            m_avg_speed_slot->setEnabled(m_chk_avg_speed_slot->isChecked());
        }
    }

    void RecommendedSettingsDlg::slotsToggled(bool on)
    {
        m_slots->setEnabled(on);
        if (on && m_chk_avg_speed_slot->isChecked())
        {
            m_chk_sim_torrents->setEnabled(false);
            m_sim_torrents->setEnabled(false);
        }
        else if (on && m_chk_sim_torrents->isChecked())
        {
            m_chk_avg_speed_slot->setEnabled(false);
            m_avg_speed_slot->setEnabled(false);
        }
        else
        {
            m_chk_slots->setEnabled(true);
            m_chk_sim_torrents->setEnabled(true);
            m_sim_torrents->setEnabled(m_chk_sim_torrents->isChecked());
            m_chk_avg_speed_slot->setEnabled(true);
            m_avg_speed_slot->setEnabled(m_chk_avg_speed_slot->isChecked());
        }
    }

    void RecommendedSettingsDlg::uploadBWChanged(int val)
    {
        m_upload_bw_display->setText(i18n("(= %1/s)", KFormat().formatByteSize(val * 128)));
    }

    void RecommendedSettingsDlg::downloadBWChanged(int val)
    {
        m_download_bw_display->setText(i18n("(= %1/s)", KFormat().formatByteSize(val * 128)));
    }
}
