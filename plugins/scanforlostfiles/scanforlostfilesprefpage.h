/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCANFORLOSTFILESPREFPAGE_H
#define KTSCANFORLOSTFILESPREFPAGE_H

#include "scanforlostfilesplugin.h"
#include "ui_scanforlostfilesprefpage.h"

#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
 * ScanForLostFiles plugin preferences page
 */

class ScanForLostFilesPrefPage : public PrefPageInterface, public Ui::ScanForLostFilesPrefPage
{
    Q_OBJECT

public:
    ScanForLostFilesPrefPage(ScanForLostFilesPlugin *plugin, QWidget *parent);
    ~ScanForLostFilesPrefPage() override;

    void loadSettings() override;
    void loadDefaults() override;
    void updateSettings() override;
    bool customWidgetsChanged() override;

    void saveSettings();

private:
    ScanForLostFilesPlugin *m_plugin;
};

}

#endif
