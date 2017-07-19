/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include <QLocale>
#include <QPushButton>

#include <KLocalizedString>

#include "edititemdlg.h"
#include "schedule.h"

namespace kt
{

    EditItemDlg::EditItemDlg(kt::Schedule* schedule, ScheduleItem* item, bool new_item, QWidget* parent)
        : QDialog(parent),
          schedule(schedule),
          item(item)
    {
        setupUi(this);
        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &EditItemDlg::accept);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        connect(m_suspended, &QCheckBox::toggled, this, &EditItemDlg::suspendedChanged);
        connect(m_screensaver_limits, &QCheckBox::toggled, this, &EditItemDlg::screensaverLimitsToggled);

        QLocale locale(QLocale::system());
        for (int i = 1; i <= 7; i++)
        {
            m_start_day->addItem(locale.dayName(i));
            m_end_day->addItem(locale.dayName(i));
        }

        m_from->setMaximumTime(QTime(23, 58, 0));
        m_to->setMinimumTime(QTime(0, 1, 0));

        m_start_day->setCurrentIndex(0);
        m_end_day->setCurrentIndex(6);

        m_from->setTime(item->start);
        m_to->setTime(item->end);
        m_start_day->setCurrentIndex(item->start_day - 1);
        m_end_day->setCurrentIndex(item->end_day - 1);
        m_suspended->setChecked(item->suspended);
        m_upload_limit->setValue(item->upload_limit);
        m_download_limit->setValue(item->download_limit);
        m_set_connection_limits->setChecked(item->set_conn_limits);
        m_max_conn_per_torrent->setEnabled(item->set_conn_limits);
        m_max_conn_per_torrent->setValue(item->torrent_conn_limit);
        m_max_conn_global->setValue(item->global_conn_limit);
        m_max_conn_global->setEnabled(item->set_conn_limits);
        m_screensaver_limits->setChecked(item->screensaver_limits);
        m_screensaver_limits->setEnabled(!item->suspended);
        m_ss_download_limit->setValue(item->ss_download_limit);
        m_ss_upload_limit->setValue(item->ss_upload_limit);
        m_ss_download_limit->setEnabled(!item->suspended && item->screensaver_limits);
        m_ss_upload_limit->setEnabled(!item->suspended && item->screensaver_limits);

        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!schedule->conflicts(item));

        connect(m_from, &QTimeEdit::timeChanged, this, &EditItemDlg::fromChanged);
        connect(m_to, &QTimeEdit::timeChanged, this, &EditItemDlg::toChanged);
        connect(m_start_day, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EditItemDlg::startDayChanged);
        connect(m_end_day, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &EditItemDlg::endDayChanged);

        setWindowTitle(new_item?i18n("Add an item"):i18n("Edit an item"));
    }


    EditItemDlg::~EditItemDlg()
    {}

    void EditItemDlg::fromChanged(const QTime& time)
    {
        // ensure that from is always smaller then to
        if (time >= m_to->time())
            m_to->setTime(time.addSecs(60));

        fillItem();
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!schedule->conflicts(item));
    }

    void EditItemDlg::toChanged(const QTime& time)
    {
        // ensure that from is always smaller then to
        if (time <= m_from->time())
            m_from->setTime(time.addSecs(-60));

        fillItem();
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!schedule->conflicts(item));
    }

    void EditItemDlg::startDayChanged(int idx)
    {
        // Make sure end day is >= start day
        if (idx > m_end_day->currentIndex())
            m_end_day->setCurrentIndex(idx);

        fillItem();
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!schedule->conflicts(item));
    }

    void EditItemDlg::endDayChanged(int idx)
    {
        // Make sure end day is >= start day
        if (idx < m_start_day->currentIndex())
            m_start_day->setCurrentIndex(idx);

        fillItem();
        m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!schedule->conflicts(item));
    }

    void EditItemDlg::suspendedChanged(bool on)
    {
        m_upload_limit->setDisabled(on);
        m_download_limit->setDisabled(on);
        m_screensaver_limits->setDisabled(on);
        screensaverLimitsToggled(m_screensaver_limits->isChecked());
    }

    void EditItemDlg::screensaverLimitsToggled(bool on)
    {
        m_ss_download_limit->setEnabled(!m_suspended->isChecked() && on);
        m_ss_upload_limit->setEnabled(!m_suspended->isChecked() && on);
    }

    void EditItemDlg::accept()
    {
        fillItem();
        if (!schedule->conflicts(item))
            QDialog::accept();
    }

    void EditItemDlg::fillItem()
    {
        item->start = m_from->time();
        item->end = m_to->time();
        item->start_day = m_start_day->currentIndex() + 1;
        item->end_day = m_end_day->currentIndex() + 1;
        item->upload_limit = m_upload_limit->value();
        item->download_limit = m_download_limit->value();
        item->suspended = m_suspended->isChecked();
        item->global_conn_limit = m_max_conn_global->value();
        item->torrent_conn_limit = m_max_conn_per_torrent->value();
        item->set_conn_limits = m_set_connection_limits->isChecked();
        item->screensaver_limits = m_screensaver_limits->isChecked();
        item->ss_download_limit = m_ss_download_limit->value();
        item->ss_upload_limit = m_ss_upload_limit->value();
        item->checkTimes();
    }

}

