/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_SCANEXTENDER_H
#define KT_SCANEXTENDER_H

#include <QTimer>
#include <QWidget>

#include "ui_scanextender.h"
#include <torrent/jobprogresswidget.h>

namespace kt
{
/**
    Extender widget which displays the results of a data scan
*/
class ScanExtender : public JobProgressWidget, public Ui_ScanExtender
{
    Q_OBJECT
public:
    ScanExtender(bt::Job *job, QWidget *parent);
    ~ScanExtender() override;

    void description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void infoMessage(const QString &plain, const QString &rich) override;
    void warning(const QString &plain, const QString &rich) override;
    void percent(long unsigned int percent) override;
    void speed(long unsigned int value) override;
    void processedAmount(KJob::Unit unit, qulonglong amount) override;
    void totalAmount(KJob::Unit unit, qulonglong amount) override;
    bool similar(Extender *ext) const override;

private Q_SLOTS:
    void cancelPressed();
    void finished(KJob *j);
    void closeRequested();
};
}

#endif // KT_SCANEXTENDER_H
