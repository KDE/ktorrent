/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_JOBPROGRESSWIDGET_H
#define KT_JOBPROGRESSWIDGET_H

#include <KJob>
#include <QWidget>

#include <gui/extender.h>
#include <ktcore_export.h>
#include <torrent/job.h>

namespace kt
{
/**
 * Base class for widgets displaying the progress of a job
 */
class KTCORE_EXPORT JobProgressWidget : public Extender
{
    Q_OBJECT
public:
    JobProgressWidget(bt::Job *job, QWidget *parent);
    ~JobProgressWidget() override;

    /// Update the description
    virtual void description(const QString &title, const QPair<QString, QString> &field1, const QPair<QString, QString> &field2) = 0;

    /// Show an informational message
    virtual void infoMessage(const QString &plain, const QString &rich) = 0;

    /// Show a warning message
    virtual void warning(const QString &plain, const QString &rich) = 0;

    /// The total amount of unit has changed
    virtual void totalAmount(KJob::Unit unit, qulonglong amount) = 0;

    /// The processed amount has changed
    virtual void processedAmount(KJob::Unit unit, qulonglong amount) = 0;

    /// The percentage has changed
    virtual void percent(long unsigned int percent) = 0;

    /// The speed has changed
    virtual void speed(long unsigned int value) = 0;

    /// Emit the close request so the ViewDelegate will clean things up
    void emitCloseRequest();

    /// Whether or not to automatically remove the widget
    bool automaticRemove() const
    {
        return automatic_remove;
    }

protected:
    void setAutomaticRemove(bool ar)
    {
        automatic_remove = ar;
    }

protected:
    bt::Job *job;
    bool automatic_remove;
};

}

#endif // KT_JOBPROGRESSWIDGET_H
