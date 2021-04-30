/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_BASICJOBPROGRESSWIDGET_H
#define KT_BASICJOBPROGRESSWIDGET_H

#include "ui_basicjobprogresswidget.h"
#include <torrent/jobprogresswidget.h>

namespace kt
{
/**
    Basic JobProgressWidget, showing a progress bar and the description
 */
class BasicJobProgressWidget : public kt::JobProgressWidget, public Ui_BasicJobProgressWidget
{
    Q_OBJECT
public:
    BasicJobProgressWidget(bt::Job *job, QWidget *parent);
    ~BasicJobProgressWidget() override;

    void description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) override;
    void infoMessage(const QString &plain, const QString &rich) override;
    void warning(const QString &plain, const QString &rich) override;
    void totalAmount(KJob::Unit unit, qulonglong amount) override;
    void processedAmount(KJob::Unit unit, qulonglong amount) override;
    void percent(long unsigned int percent) override;
    void speed(long unsigned int value) override;

    bool similar(Extender *ext) const override
    {
        Q_UNUSED(ext);
        return false;
    }
};

}

#endif // KT_BASICJOBPROGRESSWIDGET_H
