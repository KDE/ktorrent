/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#ifndef KTCONVERTTHREAD_H
#define KTCONVERTTHREAD_H

#include <QThread>
#include "ipblocklist.h"


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
        ConvertThread(ConvertDialog* dlg);
        ~ConvertThread();

        void run() override;

        QString getFailureReason() const {return failure_reason;}

        void stop() {abort = true;}

    private:
        void readInput();
        void writeOutput();
        void cleanUp(bool failed);
        void sort();
        void merge();

    private:
        ConvertDialog* dlg;
        bool abort;
        QString txt_file;
        QString dat_file;
        QString tmp_file;
        QList<IPBlock> input;
        QString failure_reason;
    };

}

#endif
