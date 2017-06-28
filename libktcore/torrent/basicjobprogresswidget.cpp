/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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

#include "basicjobprogresswidget.h"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <util/functions.h>


namespace kt
{

    BasicJobProgressWidget::BasicJobProgressWidget(bt::Job* job, QWidget* parent)
        : JobProgressWidget(job, parent)
    {
        setupUi(this);
        job_description->clear();
        job_title->clear();
        job_speed->clear();
        msg->clear();
        msg->setVisible(false);
    }

    BasicJobProgressWidget::~BasicJobProgressWidget()
    {

    }

    void BasicJobProgressWidget::description(const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2)
    {
        job_title->setText(title);
        job_description->setText(i18n("%1: %2<br/>%3: %4", field1.first, field1.second, field2.first, field2.second));
        resized(this);
    }

    void BasicJobProgressWidget::infoMessage(const QString& plain, const QString& rich)
    {
        Q_UNUSED(plain);
        msg->setText(rich);
        msg->setVisible(true);
        resized(this);
    }

    void BasicJobProgressWidget::warning(const QString& plain, const QString& rich)
    {
        Q_UNUSED(plain);
        msg->setText(i18n("Warning: %1", rich));
        msg->setVisible(true);
        resized(this);
    }

    void BasicJobProgressWidget::totalAmount(KJob::Unit unit, qulonglong amount)
    {
        Q_UNUSED(unit);
        progress->setMaximum(amount);
    }

    void BasicJobProgressWidget::processedAmount(KJob::Unit unit, qulonglong amount)
    {
        Q_UNUSED(unit);
        progress->setValue(amount);
    }

    void BasicJobProgressWidget::percent(long unsigned int percent)
    {
        progress->setValue(percent);
        progress->setMaximum(100);
    }

    void BasicJobProgressWidget::speed(long unsigned int value)
    {
        job_speed->setText(bt::BytesPerSecToString(value));
    }

}
