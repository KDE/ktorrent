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

#ifndef CONVERTDIALOG_H
#define CONVERTDIALOG_H

#include <QDialog>
#include <QEvent>
#include <QMutex>
#include <QThread>
#include <QTimer>

#include "ui_convertdialog.h"

namespace kt
{
    class ConvertThread;


    class ConvertDialog : public QDialog, public Ui_ConvertDialog
    {
        Q_OBJECT
    public:
        ConvertDialog(QWidget* parent);
        ~ConvertDialog();

        /**
         * Set the message.
         * @param msg The new message
         */
        void message(const QString& msg);

        /**
         * Update progress bar
         * @param val The value
         * @param total The max number of steps
         */
        void progress(int val, int total);

    private slots:
        void convert();
        void threadFinished();
        void btnCancelClicked();
        void update();

    private:
        void closeEvent(QCloseEvent* e) override;

    private:
        ConvertThread* convert_thread;
        QString msg;
        int prog, max;
        QMutex mutex;
        QTimer timer;
        bool canceled;
    };
}
#endif
