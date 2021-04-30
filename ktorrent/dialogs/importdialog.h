/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include "ui_importdialog.h"
#include <QDialog>
#include <torrent/torrent.h>
#include <util/constants.h>

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
    ImportDialog(CoreInterface *core, QWidget *parent = nullptr);
    ~ImportDialog() override;

public Q_SLOTS:
    void onImport();
    void onTorrentGetReult(KJob *j);

private Q_SLOTS:
    void progress(quint32 num, quint32 total);
    void finished();
    void cancelImport();

private:
    void writeIndex(const QString &file, const bt::BitSet &chunks);
    void makeDirs(const QString &dnd_dir, const QString &data_url, const QString &fpath);
    void saveStats(const QString &stats_file, const QString &data_url, bt::Uint64 imported, bool custom_output_name);
    bt::Uint64 calcImportedBytes(const bt::BitSet &chunks, const bt::Torrent &tor);
    void saveFileInfo(const QString &file_info_file, QList<bt::Uint32> &dnd);
    void saveFileMap(const bt::Torrent &tor, const QString &tor_dir);
    void saveFileMap(const QString &tor_dir, const QString &ddir);
    void import();

private:
    CoreInterface *core;
    bt::DataChecker *dc;
    bt::DataCheckerThread *dc_thread;
    bt::Torrent tor;
    bool canceled;
};
}

#endif
