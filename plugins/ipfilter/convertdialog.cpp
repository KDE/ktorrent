/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QCloseEvent>
#include <QTimer>

#include <KMessageBox>

#include <interfaces/coreinterface.h>
#include <torrent/globals.h>
#include <util/constants.h>
#include <util/log.h>

#include "convertdialog.h"
#include "convertthread.h"

using namespace bt;

namespace kt
{
ConvertDialog::ConvertDialog(QWidget *parent)
    : QDialog(parent)
    , convert_thread(nullptr)
{
    setupUi(this);
    setModal(false);
    adjustSize();
    canceled = false;
    connect(m_cancel, &QPushButton::clicked, this, &ConvertDialog::btnCancelClicked);
    connect(&timer, &QTimer::timeout, this, &ConvertDialog::update);

    QTimer::singleShot(500, this, &ConvertDialog::convert);
}

ConvertDialog::~ConvertDialog()
{
}

void ConvertDialog::message(const QString &msg)
{
    QMutexLocker lock(&mutex);
    this->msg = msg;
}

void ConvertDialog::progress(int val, int total)
{
    QMutexLocker lock(&mutex);
    prog = val;
    max = total;
}

void ConvertDialog::update()
{
    QMutexLocker lock(&mutex);
    m_msg->setText(msg);
    m_progress_bar->setValue(prog);
    m_progress_bar->setMaximum(max);
}

void ConvertDialog::convert()
{
    if (convert_thread)
        return;

    convert_thread = new ConvertThread(this);
    connect(convert_thread, &ConvertThread::finished, this, &ConvertDialog::threadFinished, Qt::QueuedConnection);
    convert_thread->start();
    timer.start(500);
}

void ConvertDialog::threadFinished()
{
    QString failure = convert_thread->getFailureReason();
    if (failure != QString()) {
        convert_thread->wait();
        convert_thread->deleteLater();
        convert_thread = 0;
        KMessageBox::error(this, failure);
        reject();
    } else {
        convert_thread->wait();
        convert_thread->deleteLater();
        convert_thread = 0;
        if (canceled)
            reject();
        else
            accept();
    }
}

void ConvertDialog::closeEvent(QCloseEvent *e)
{
    if (!convert_thread)
        e->accept();
    else
        e->ignore();
}

void ConvertDialog::btnCancelClicked()
{
    canceled = true;
    if (convert_thread)
        convert_thread->stop();
}

}
