/***************************************************************************
 *   Copyright (C) 2009 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#include "scanextender.h"

#include <KGuiItem>
#include <KLocalizedString>
#include <KStandardGuiItem>

#include <torrent/job.h>
#include <interfaces/torrentinterface.h>
#include <datachecker/datacheckerjob.h>



namespace kt
{

    ScanExtender::ScanExtender(bt::Job* job, QWidget* parent)
        : JobProgressWidget(job, parent)
    {
        setupUi(this);

        bt::DataCheckerJob* dcj = (bt::DataCheckerJob*)job;
        setAutomaticRemove(dcj->isAutoImport());
        connect(job, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));

        KGuiItem::assign(cancel_button, KStandardGuiItem::cancel());
        KGuiItem::assign(close_button, KStandardGuiItem::close());
        close_button->setEnabled(false);
        connect(close_button, &QPushButton::clicked, this, &ScanExtender::closeRequested);
        connect(cancel_button, &QPushButton::clicked, this, &ScanExtender::cancelPressed);

        progress_bar->setFormat(i18n("Checked %v of %m chunks"));
        progress_bar->setValue(0);
        progress_bar->setMaximum(tc->getStats().total_chunks);

        error_msg->clear();
        error_msg->hide();

        QFont font = chunks_failed->font();
        font.setBold(true);
        chunks_failed->setFont(font);
        chunks_found->setFont(font);
        chunks_downloaded->setFont(font);
        chunks_not_downloaded->setFont(font);
    }

    ScanExtender::~ScanExtender()
    {
    }

    void ScanExtender::description(const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2)
    {
        Q_UNUSED(title);
        chunks_failed->setText(field1.first);
        chunks_found->setText(field1.second);
        chunks_downloaded->setText(field2.first);
        chunks_not_downloaded->setText(field2.second);

        if (error_msg->isVisible())
        {
            error_msg->hide();
            emit resized(this);
        }
    }

    void ScanExtender::processedAmount(KJob::Unit unit, qulonglong amount)
    {
        Q_UNUSED(unit);
        progress_bar->setValue(amount);
    }

    void ScanExtender::totalAmount(KJob::Unit unit, qulonglong amount)
    {
        Q_UNUSED(unit);
        progress_bar->setMaximum(amount);
    }

    void ScanExtender::infoMessage(const QString& plain, const QString& rich)
    {
        Q_UNUSED(rich);
        error_msg->setText(plain);
        error_msg->show();
        emit resized(this);
    }

    void ScanExtender::warning(const QString& plain, const QString& rich)
    {
        Q_UNUSED(rich);
        Q_UNUSED(plain);
    }

    void ScanExtender::speed(long unsigned int value)
    {
        Q_UNUSED(value);
    }

    void ScanExtender::percent(long unsigned int percent)
    {
        Q_UNUSED(percent);
    }



    void ScanExtender::finished(KJob* j)
    {
        progress_bar->setValue(progress_bar->maximum());
        progress_bar->setEnabled(false);
        cancel_button->setDisabled(true);
        close_button->setEnabled(true);

        if (j->error() && !j->errorText().isEmpty())
        {
            error_msg->show();
            error_msg->setText(i18n("<font color=\"red\">%1</font>").arg(j->errorText()));
            emit resized(this);
        }
    }

    void ScanExtender::cancelPressed()
    {
        if (job)
            job->kill(false);
    }

    void ScanExtender::closeRequested()
    {
        closeRequest(this);
    }

    bool ScanExtender::similar(Extender* ext) const
    {
        return qobject_cast<ScanExtender*>(ext) != 0;
    }

}

