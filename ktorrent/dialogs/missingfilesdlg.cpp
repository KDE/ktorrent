/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "missingfilesdlg.h"

#include <QFileDialog>
#include <QIcon>
#include <QMimeDatabase>

#include <KFileWidget>
#include <KGuiItem>
#include <KMessageBox>
#include <KRecentDirs>
#include <KStandardGuiItem>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
MissingFilesDlg::MissingFilesDlg(const QString &text, const QStringList &missing, bt::TorrentInterface *tc, QWidget *parent)
    : QDialog(parent)
    , ret(CANCEL)
    , tc(tc)
{
    setupUi(this);

    m_text->setText(text);
    connect(m_cancel, &QPushButton::clicked, this, &MissingFilesDlg::cancelPressed);
    connect(m_recreate, &QPushButton::clicked, this, &MissingFilesDlg::recreatePressed);
    connect(m_dnd, &QPushButton::clicked, this, &MissingFilesDlg::dndPressed);
    connect(m_select_new, &QPushButton::clicked, this, &MissingFilesDlg::selectNewPressed);

    KGuiItem::assign(m_cancel, KStandardGuiItem::cancel());

    QMimeDatabase mimeDatabase;
    for (const QString &s : missing) {
        QListWidgetItem *lwi = new QListWidgetItem(m_file_list);
        lwi->setText(s);
        lwi->setIcon(QIcon::fromTheme(mimeDatabase.mimeTypeForFile(s).iconName()));
    }

    m_dnd->setEnabled(tc->getStats().multi_file_torrent);
    int disabled_files = 0;
    for (bt::Uint32 i = 0; i < tc->getNumFiles(); i++)
        if (tc->getTorrentFile(i).getPriority() == bt::EXCLUDED)
            disabled_files++;
    // select new is only possible if all files are missing
    m_select_new->setEnabled(!tc->getStats().multi_file_torrent || missing.count() == (int)tc->getNumFiles() - disabled_files);
}

MissingFilesDlg::~MissingFilesDlg()
{
}

void MissingFilesDlg::dndPressed()
{
    ret = DO_NOT_DOWNLOAD;
    accept();
}

void MissingFilesDlg::selectNewPressed()
{
    if (tc->getStats().multi_file_torrent) {
        QString recentDirClass;
        QString dir =
            QFileDialog::getExistingDirectory(this,
                                              i18n("Select the directory where the data now is."),
                                              KFileWidget::getStartUrl(QUrl(QStringLiteral("kfiledialog:///saveTorrentData")), recentDirClass).toLocalFile());

        if (dir.isEmpty())
            return;

        if (!recentDirClass.isEmpty())
            KRecentDirs::add(recentDirClass, dir);

        QString old = tc->getStats().output_path;
        tc->changeOutputDir(dir, bt::TorrentInterface::FULL_PATH);
        QStringList dummy;
        if (tc->hasMissingFiles(dummy)) {
            int ans = KMessageBox::No;
            if ((bt::Uint32)dummy.count() == tc->getNumFiles())
                ans = KMessageBox::questionYesNo(
                    this,
                    i18n("The data files are not present in the location you selected. Do you want to create all the files in the selected directory?"));
            else
                ans = KMessageBox::questionYesNo(this,
                                                 i18n("Not all files were found in the new location; some are still missing. Do you want to create the missing "
                                                      "files in the selected directory?"));

            if (ans == KMessageBox::Yes) {
                tc->recreateMissingFiles();
                ret = NEW_LOCATION_SELECTED;
                accept();
            } else
                tc->changeOutputDir(old, bt::TorrentInterface::FULL_PATH);
        } else {
            ret = NEW_LOCATION_SELECTED;
            accept();
        }
    } else {
        QString recentDirClass;
        QString dir =
            QFileDialog::getExistingDirectory(this,
                                              i18n("Select the directory where the data now is."),
                                              KFileWidget::getStartUrl(QUrl(QStringLiteral("kfiledialog:///saveTorrentData")), recentDirClass).toLocalFile());

        if (dir.isEmpty())
            return;

        if (!recentDirClass.isEmpty())
            KRecentDirs::add(recentDirClass, dir);

        QString old = tc->getDataDir();
        tc->changeOutputDir(dir, 0);
        QStringList dummy;
        if (tc->hasMissingFiles(dummy)) {
            if (KMessageBox::questionYesNo(
                    this,
                    i18n("The data file is not present in the location you selected. Do you want to create the file in the selected directory?"))
                == KMessageBox::Yes) {
                tc->recreateMissingFiles();
                ret = NEW_LOCATION_SELECTED;
                accept();
            } else
                tc->changeOutputDir(old, 0);
        } else {
            ret = NEW_LOCATION_SELECTED;
            accept();
        }
    }
}

void MissingFilesDlg::recreatePressed()
{
    ret = RECREATE;
    accept();
}

void MissingFilesDlg::cancelPressed()
{
    ret = CANCEL;
    accept();
}

MissingFilesDlg::ReturnCode MissingFilesDlg::execute()
{
    exec();
    return ret;
}
}
