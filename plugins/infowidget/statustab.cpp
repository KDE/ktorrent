/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#include <cmath>

#include <QCheckBox>
#include <QDateTime>

#include <KLocalizedString>
#include <KRun>

#include <util/functions.h>
#include <util/log.h>
#include <util/sha1hash.h>
#include "downloadedchunkbar.h"
#include "availabilitychunkbar.h"
#include "statustab.h"
#include "settings.h"



using namespace bt;

namespace kt
{

    StatusTab::StatusTab(QWidget* parent) : QWidget(parent)
    {
        setupUi(this);
        // do not use hardcoded colors
        hdr_info->setBackgroundRole(QPalette::Mid);
        hdr_chunks->setBackgroundRole(QPalette::Mid);
        hdr_sharing->setBackgroundRole(QPalette::Mid);

        QFont f = font();
        f.setBold(true);
        share_ratio->setFont(f);
        avg_down_speed->setFont(f);
        avg_up_speed->setFont(f);
        type->setFont(f);
        comments->setFont(f);
        info_hash->setFont(f);

        ratio_limit->setMinimum(0.0f);
        ratio_limit->setMaximum(100.0f);
        ratio_limit->setSingleStep(0.1f);
        ratio_limit->setKeyboardTracking(false);
        connect(ratio_limit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &StatusTab::maxRatioChanged);
        connect(use_ratio_limit, &QCheckBox::toggled, this, &StatusTab::useRatioLimitToggled);

        time_limit->setMinimum(0.0f);
        time_limit->setMaximum(10000000.0f);
        time_limit->setSingleStep(0.05f);
        time_limit->setSpecialValueText(i18n("No limit"));
        time_limit->setKeyboardTracking(false);
        connect(use_time_limit, &QCheckBox::toggled, this, &StatusTab::useTimeLimitToggled);
        connect(time_limit, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &StatusTab::maxTimeChanged);

        int h = (int)ceil(fontMetrics().height() * 1.25);
        downloaded_bar->setFixedHeight(h);
        availability_bar->setFixedHeight(h);

        comments->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard | Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
        connect(comments, &KSqueezedTextLabel::linkActivated, this, &StatusTab::linkActivated);

        // initialize everything with curr_tc == 0
        setEnabled(false);
        ratio_limit->setValue(0.00f);
        share_ratio->clear();
        type->clear();
        comments->clear();
        avg_up_speed->clear();
        avg_down_speed->clear();
        info_hash->clear();
    }

    StatusTab::~StatusTab()
    {}

    void StatusTab::changeTC(bt::TorrentInterface* tc)
    {
        if (tc == curr_tc.data())
            return;

        curr_tc = tc;

        downloaded_bar->setTC(tc);
        availability_bar->setTC(tc);
        setEnabled(tc != 0);

        if (curr_tc)
        {
            info_hash->setText(tc->getInfoHash().toString());
            type->setText(tc->getStats().priv_torrent ? i18n("Private") : i18n("Public"));

            // Don't allow multiple lines in the comments field
            QString text = tc->getComments();
            if (text.contains(QLatin1String("\n")))
                text = text.replace(QLatin1Char('\n'), QLatin1Char(' '));

            // Make links clickable
            QStringList words = text.split(QLatin1Char(' '), QString::KeepEmptyParts);
            for (QStringList::iterator i = words.begin(); i != words.end(); i++)
            {
                QString& w = *i;
                if (w.startsWith(QLatin1String("http://")) || w.startsWith(QLatin1String("https://")) || w.startsWith(QLatin1String("ftp://")))
                    w = QStringLiteral("<a href=\"") + w + QStringLiteral("\">") + w + QStringLiteral("</a>");
            }

            comments->setText(words.join(QStringLiteral(" ")));


            float ratio = tc->getMaxShareRatio();
            if (ratio > 0)
            {
                use_ratio_limit->setChecked(true);
                ratio_limit->setValue(ratio);
                ratio_limit->setEnabled(true);
            }
            else
            {
                ratio_limit->setValue(0.0);
                use_ratio_limit->setChecked(false);
                ratio_limit->setEnabled(false);
            }

            float hours = tc->getMaxSeedTime();
            if (hours > 0)
            {
                time_limit->setEnabled(true);
                use_time_limit->setChecked(true);
                time_limit->setValue(hours);
            }
            else
            {
                time_limit->setEnabled(false);
                time_limit->setValue(0.0);
                use_time_limit->setChecked(false);
            }
        }
        else
        {
            info_hash->clear();
            ratio_limit->setValue(0.00f);
            time_limit->setValue(0.0);
            share_ratio->clear();
            type->clear();
            comments->clear();
            avg_up_speed->clear();
            avg_down_speed->clear();
        }

        update();
    }

    void StatusTab::update()
    {
        if (!curr_tc)
            return;

        bt::TorrentInterface* tc = curr_tc.data();
        const bt::TorrentStats& s = tc->getStats();

        downloaded_bar->updateBar();
        availability_bar->updateBar();

        float ratio = s.shareRatio();
        if (!ratio_limit->hasFocus())
            maxRatioUpdate();

        if (!time_limit->hasFocus())
            maxSeedTimeUpdate();

        static QLocale locale;
        share_ratio->setText(QStringLiteral("<font color=\"%1\">%2</font>").arg(ratio <= Settings::greenRatio() ? QStringLiteral("#ff0000") : QStringLiteral("#1c9a1c")).arg(locale.toString(ratio, 'g', 2)));

        Uint32 secs = tc->getRunningTimeUL();
        if (secs == 0)
        {
            avg_up_speed->setText(BytesPerSecToString(0));
        }
        else
        {
            double r = (double)s.bytes_uploaded;
            avg_up_speed->setText(BytesPerSecToString(r / secs));
        }

        secs = tc->getRunningTimeDL();
        if (secs == 0)
        {
            avg_down_speed->setText(BytesPerSecToString(0));
        }
        else
        {
            double r = 0;
            if (s.imported_bytes <= s.bytes_downloaded)
                r = (double)(s.bytes_downloaded - s.imported_bytes);
            else
                r = (double)s.bytes_downloaded;

            avg_down_speed->setText(BytesPerSecToString(r / secs));
        }
    }

    void StatusTab::maxRatioChanged(double v)
    {
        if (!curr_tc)
            return;

        curr_tc.data()->setMaxShareRatio(v);
    }

    void StatusTab::useRatioLimitToggled(bool state)
    {
        if (!curr_tc)
            return;

        bt::TorrentInterface* tc = curr_tc.data();

        ratio_limit->setEnabled(state);
        if (!state)
        {
            tc->setMaxShareRatio(0.00f);
            ratio_limit->setValue(0.00f);
        }
        else
        {
            float msr = tc->getMaxShareRatio();
            if (msr == 0.00f)
            {
                tc->setMaxShareRatio(1.00f);
                ratio_limit->setValue(1.00f);
            }

            float sr = tc->getStats().shareRatio();
            if (sr >= 1.00f)
            {
                //always add 1 to max share ratio to prevent stopping if torrent is running.
                tc->setMaxShareRatio(sr + 1.00f);
                ratio_limit->setValue(sr + 1.00f);
            }
        }
    }

    void StatusTab::maxRatioUpdate()
    {
        if (!curr_tc)
            return;

        float ratio = curr_tc.data()->getMaxShareRatio();
        if (ratio > 0.00f)
        {
            // only update when needed
            if (ratio_limit->isEnabled() && use_ratio_limit->isChecked() && ratio_limit->value() == ratio)
                return;

            ratio_limit->setEnabled(true);
            use_ratio_limit->setChecked(true);
            ratio_limit->setValue(ratio);
        }
        else
        {
            // only update when needed
            if (!ratio_limit->isEnabled() && !use_ratio_limit->isChecked() && ratio_limit->value() != 0.00f)
                return;

            ratio_limit->setEnabled(false);
            use_ratio_limit->setChecked(false);
            ratio_limit->setValue(0.00f);
        }
    }

    void StatusTab::maxSeedTimeUpdate()
    {
        if (!curr_tc)
            return;

        float time = curr_tc.data()->getMaxSeedTime();
        if (time > 0.00f)
        {
            // only update when needed
            if (time_limit->isEnabled() && use_time_limit->isChecked() && time_limit->value() == time)
                return;

            time_limit->setEnabled(true);
            use_time_limit->setChecked(true);
            time_limit->setValue(time);
        }
        else
        {
            // only update when needed
            if (!time_limit->isEnabled() && !use_time_limit->isChecked() && time_limit->value() != 0.00f)
                return;

            time_limit->setEnabled(false);
            use_time_limit->setChecked(false);
            time_limit->setValue(0.00f);
        }
    }

    void StatusTab::useTimeLimitToggled(bool on)
    {
        if (!curr_tc)
            return;

        bt::TorrentInterface* tc = curr_tc.data();
        time_limit->setEnabled(on);
        if (on)
        {
            Uint32 dl = tc->getRunningTimeDL();
            Uint32 ul = tc->getRunningTimeUL();
            float hours = (ul - dl) / 3600.0f + 1.0; // add one hour to current seed time to not stop immediately
            time_limit->setValue(hours);
            tc->setMaxSeedTime(hours);
        }
        else
        {
            tc->setMaxSeedTime(0.0f);
        }
    }

    void StatusTab::maxTimeChanged(double v)
    {
        if (curr_tc)
            curr_tc.data()->setMaxSeedTime(v);
    }

    void StatusTab::linkActivated(const QString& link)
    {
        new KRun(QUrl(link), QApplication::activeWindow());
    }


}
