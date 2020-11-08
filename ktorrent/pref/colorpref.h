/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_COLORPREF_HH
#define KT_COLORPREF_HH

#include <interfaces/prefpageinterface.h>
#include "ui_colorpref.h"


namespace kt
{
class ColorPref : public PrefPageInterface, public Ui_ColorPref
{
    Q_OBJECT
public:
    ColorPref(QWidget* parent);
    ~ColorPref() override;

    void loadSettings() override;
    void loadDefaults() override;

};
}

#endif
