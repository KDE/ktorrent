/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCANFOLDERPREFPAGE_H
#define KTSCANFOLDERPREFPAGE_H

#include "scanfolderplugin.h"
#include "ui_scanfolderprefpage.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
 * ScanFolder plugin preferences page
 * @author Ivan Vasić <ivasic@gmail.com>
 */
class ScanFolderPrefPage : public PrefPageInterface, public Ui_ScanFolderPrefPage
{
    Q_OBJECT

public:
    ScanFolderPrefPage(ScanFolderPlugin *plugin, QWidget *parent);
    ~ScanFolderPrefPage() override;

    void loadSettings() override;
    void loadDefaults() override;
    void updateSettings() override;
    bool customWidgetsChanged() override;

private Q_SLOTS:
    void addPressed();
    void removePressed();
    void selectionChanged();
    void currentGroupChanged(int idx);

private:
    ScanFolderPlugin *m_plugin;
    QStringList folders;
};

}

#endif
