/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ConvertDialog(QWidget *parent);
    ~ConvertDialog() override;

    /**
     * Set the message.
     * @param msg The new message
     */
    void message(const QString &msg);

    /**
     * Update progress bar
     * @param val The value
     * @param total The max number of steps
     */
    void progress(int val, int total);

private Q_SLOTS:
    void convert();
    void threadFinished();
    void btnCancelClicked();
    void update();

private:
    void closeEvent(QCloseEvent *e) override;

private:
    ConvertThread *convert_thread;
    QString msg;
    int prog, max;
    QMutex mutex;
    QTimer timer;
    bool canceled;
};
}
#endif
