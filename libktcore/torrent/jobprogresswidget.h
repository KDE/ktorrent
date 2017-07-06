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

#ifndef KT_JOBPROGRESSWIDGET_H
#define KT_JOBPROGRESSWIDGET_H

#include <QWidget>
#include <KJob>

#include <ktcore_export.h>
#include <torrent/job.h>
#include <gui/extender.h>

class QProgressBar;
class QLabel;

namespace bt
{
    class TorrentInterface;
}

namespace kt
{
    /**
     * Base class for widgets displaying the progress of a job
     */
    class KTCORE_EXPORT JobProgressWidget : public Extender
    {
        Q_OBJECT
    public:
        JobProgressWidget(bt::Job* job, QWidget* parent);
        ~JobProgressWidget();

        /// Update the description
        virtual void description(const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2) = 0;

        /// Show an informational message
        virtual void infoMessage(const QString& plain, const QString& rich) = 0;

        /// Show a warning message
        virtual void warning(const QString& plain, const QString& rich) = 0;

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
        bool automaticRemove() const {return automatic_remove;}

    protected:
        void setAutomaticRemove(bool ar) {automatic_remove = ar;}

    protected:
        bt::Job* job;
        bool automatic_remove;
    };

}

#endif // KT_JOBPROGRESSWIDGET_H
