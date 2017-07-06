/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <torrent/torrent.h>
#include <util/constants.h>
#include "ui_importdialog.h"

class KJob;

namespace bt
{
    class BitSet;
    class Torrent;
    class DataChecker;
    class DataCheckerThread;
}


namespace kt
{
    class CoreInterface;

    class ImportDialog : public QDialog, public Ui_ImportDialog
    {
        Q_OBJECT

    public:
        ImportDialog(CoreInterface* core, QWidget* parent = 0);
        ~ImportDialog();

    public slots:
        void onImport();
        void onTorrentGetReult(KJob* j);

    private slots:
        void progress(quint32 num, quint32 total);
        void finished();
        void cancelImport();

    private:
        void writeIndex(const QString& file, const bt::BitSet& chunks);
        void makeDirs(const QString& dnd_dir, const QString &data_url, const QString& fpath);
        void saveStats(const QString& stats_file, const QString &data_url, bt::Uint64 imported, bool custom_output_name);
        bt::Uint64 calcImportedBytes(const bt::BitSet& chunks, const bt::Torrent& tor);
        void saveFileInfo(const QString& file_info_file, QList<bt::Uint32> & dnd);
        void saveFileMap(const bt::Torrent& tor, const QString& tor_dir);
        void saveFileMap(const QString& tor_dir, const QString& ddir);
        void import();

    private:
        CoreInterface* core;
        bt::DataChecker* dc;
        bt::DataCheckerThread* dc_thread;
        bt::Torrent tor;
        bool canceled;
    };
}

#endif

