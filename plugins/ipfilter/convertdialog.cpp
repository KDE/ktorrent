/***************************************************************************
*   Copyright (C) 2005 by Joris Guisson                                   *
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
*   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
***************************************************************************/

#include <QCloseEvent>
#include <QTimer>

#include <KMessageBox>

#include <util/log.h>
#include <util/constants.h>
#include <torrent/globals.h>
#include <interfaces/coreinterface.h>

#include "convertdialog.h"
#include "convertthread.h"


using namespace bt;

namespace kt
{

    ConvertDialog::ConvertDialog(QWidget* parent) : QDialog(parent), convert_thread(nullptr)
    {
        setupUi(this);
        setModal(false);
        adjustSize();
        canceled = false;
        connect(m_cancel, &QPushButton::clicked, this, &ConvertDialog::btnCancelClicked);
        connect(&timer, &QTimer::timeout, this, &ConvertDialog::update);

        QTimer::singleShot(500, this, SLOT(convert()));
    }

    ConvertDialog::~ConvertDialog()
    {
    }

    void ConvertDialog::message(const QString& msg)
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
        if (failure != QString::null)
        {
            convert_thread->wait();
            convert_thread->deleteLater();
            convert_thread = 0;
            KMessageBox::error(this, failure);
            reject();
        }
        else
        {
            convert_thread->wait();
            convert_thread->deleteLater();
            convert_thread = 0;
            if (canceled)
                reject();
            else
                accept();
        }
    }


    void ConvertDialog::closeEvent(QCloseEvent* e)
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

