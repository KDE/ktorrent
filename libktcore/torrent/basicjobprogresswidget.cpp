/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "basicjobprogresswidget.h"

#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

#include <KLocalizedString>

#include <util/functions.h>

namespace kt
{
BasicJobProgressWidget::BasicJobProgressWidget(bt::Job *job, QWidget *parent)
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

void BasicJobProgressWidget::description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2)
{
    job_title->setText(title);
    job_description->setText(i18n("%1: %2<br/>%3: %4", field1.first, field1.second, field2.first, field2.second));
    resized(this);
}

void BasicJobProgressWidget::infoMessage(const QString &plain, const QString &rich)
{
    Q_UNUSED(plain);
    msg->setText(rich);
    msg->setVisible(true);
    resized(this);
}

void BasicJobProgressWidget::warning(const QString &plain, const QString &rich)
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
