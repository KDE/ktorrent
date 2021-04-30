/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PASTEDIALOG_H
#define PASTEDIALOG_H

#include "ui_pastedlgbase.h"
#include <KSharedConfig>
#include <QDialog>

namespace kt
{
class Core;

/**
 * @author Ivan Vasic
 * @brief Torrent URL paste dialog
 **/
class PasteDialog : public QDialog, public Ui_PasteDlgBase
{
    Q_OBJECT
public:
    PasteDialog(Core *core, QWidget *parent = nullptr, Qt::WindowFlags fl = {});
    ~PasteDialog() override;

    /**
     * Load the state of the dialog
     */
    void loadState(KSharedConfig::Ptr cfg);

    /**
     * Save the state of the dialog
     */
    void saveState(KSharedConfig::Ptr cfg);

public Q_SLOTS:
    void accept() override;

private:
    void loadGroups();

private:
    Core *m_core;
};
}
#endif
