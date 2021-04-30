/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_ADDTRACKERSDIALOG_H
#define KT_ADDTRACKERSDIALOG_H

#include <QDialog>

#include <KEditListWidget>

namespace kt
{
/**
 * Dialog to add trackers
 */
class AddTrackersDialog : public QDialog
{
    Q_OBJECT
public:
    AddTrackersDialog(QWidget *parent, const QStringList &tracker_hints);
    ~AddTrackersDialog() override;

    /// Get the tracker list
    QStringList trackerList() const;

private:
    KEditListWidget *trackers;
};

}

#endif // KT_ADDTRACKERSDIALOG_H
