/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTCONVERTTHREAD_H
#define KTCONVERTTHREAD_H

#include "ipblocklist.h"
#include <QThread>

namespace kt
{
class ConvertDialog;

/**
 * Thread which does the converting of the text filter file to our own format.
 * @author Joris Guisson
 */
class ConvertThread : public QThread
{
public:
    ConvertThread(ConvertDialog *dlg);
    ~ConvertThread() override;

    void run() override;

    QString getFailureReason() const
    {
        return failure_reason;
    }

    void stop()
    {
        abort = true;
    }

private:
    void readInput();
    void writeOutput();
    void cleanUp(bool failed);
    void sort();
    void merge();

private:
    ConvertDialog *dlg;
    bool abort;
    QString txt_file;
    QString dat_file;
    QString tmp_file;
    QList<IPBlock> input;
    QString failure_reason;
};

}

#endif
