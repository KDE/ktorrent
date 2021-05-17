/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "generalpref.h"
#include "settings.h"
#include <interfaces/functions.h>

namespace kt
{
GeneralPref::GeneralPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Application"), QStringLiteral("ktorrent"), parent)
{
    setupUi(this);
    kcfg_tempDir->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    kcfg_saveDir->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    kcfg_torrentCopyDir->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
    kcfg_completedDir->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
}

GeneralPref::~GeneralPref()
{
}

void GeneralPref::loadSettings()
{
    kcfg_tempDir->setProperty("kcfg_property", QStringLiteral("text"));
    kcfg_saveDir->setProperty("kcfg_property", QStringLiteral("text"));
    kcfg_torrentCopyDir->setProperty("kcfg_property", QStringLiteral("text"));
    kcfg_completedDir->setProperty("kcfg_property", QStringLiteral("text"));

    if (Settings::tempDir().isEmpty())
        kcfg_tempDir->setText(kt::DataDir());
    else
        kcfg_tempDir->setText(Settings::tempDir());

    kcfg_saveDir->setEnabled(Settings::useSaveDir());
    if (Settings::saveDir().isEmpty())
        kcfg_saveDir->setText(QDir::homePath());
    else
        kcfg_saveDir->setText(Settings::saveDir());

    kcfg_torrentCopyDir->setEnabled(Settings::useTorrentCopyDir());
    if (Settings::torrentCopyDir().isEmpty())
        kcfg_torrentCopyDir->setText(QDir::homePath());
    else
        kcfg_torrentCopyDir->setText(Settings::torrentCopyDir());

    kcfg_completedDir->setEnabled(Settings::useCompletedDir());
    if (Settings::completedDir().isEmpty())
        kcfg_completedDir->setText(QDir::homePath());
    else
        kcfg_completedDir->setText(Settings::completedDir());

    //          kcfg_downloadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());
    //          kcfg_uploadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());
}

void GeneralPref::loadDefaults()
{
    Settings::setTempDir(kt::DataDir());
    Settings::setSaveDir(QDir::homePath());
    Settings::setCompletedDir(QDir::homePath());
    Settings::setTorrentCopyDir(QDir::homePath());
    loadSettings();
}

}
