/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTLOGPREFPAGE_H
#define KTLOGPREFPAGE_H

#include "ui_logprefwidget.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class LogFlags;

class LogPrefPage : public PrefPageInterface, public Ui_LogPrefWidget
{
    Q_OBJECT
public:
    LogPrefPage(LogFlags *flags, QWidget *parent);
    ~LogPrefPage() override;

    void loadDefaults() override;
    void loadSettings() override;
    void updateSettings() override;

    void saveState();
    void loadState();

private:
    bool state_loaded;
};
}

#endif
