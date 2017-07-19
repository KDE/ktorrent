/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include "fileselectdlg.h"

#include <QMenu>
#include <QPushButton>
#include <QTextCodec>

#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardGuiItem>

#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/error.h>
#include <util/treefiltermodel.h>
#include <interfaces/functions.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentfiletreemodel.h>
#include <torrent/torrentfilelistmodel.h>
#include "settings.h"

using namespace bt;

namespace kt
{

    FileSelectDlg::FileSelectDlg(kt::QueueManager* qman, kt::GroupManager* gman, const QString& group_hint, QWidget* parent)
        : QDialog(parent)
        , tc(nullptr)
        , model(nullptr)
        , qman(qman)
        , gman(gman)
        , start(nullptr)
        , skip_check(nullptr)
        , initial_group(nullptr)
        , show_file_tree(true)
        , already_downloaded(0)
    {
        setupUi(this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &FileSelectDlg::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &FileSelectDlg::reject);

        m_file_view->setAlternatingRowColors(true);
        filter_model = new TreeFilterModel(this);
        m_file_view->setModel(filter_model);
        //root = 0;
        connect(m_select_all, &QPushButton::clicked, this, &FileSelectDlg::selectAll);
        connect(m_select_none, &QPushButton::clicked, this, &FileSelectDlg::selectNone);
        connect(m_invert_selection, &QPushButton::clicked, this, &FileSelectDlg::invertSelection);
        connect(m_collapse_all, &QPushButton::clicked, m_file_view, &QTreeView::collapseAll);
        connect(m_expand_all, &QPushButton::clicked, m_file_view, &QTreeView::expandAll);

        m_downloadLocation->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);
        m_completedLocation->setMode(KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly);

        m_download_location_history->setIcon(QIcon::fromTheme(QStringLiteral("view-history")));
        m_download_location_history->setPopupMode(QToolButton::MenuButtonPopup);
        m_move_when_completed_history->setIcon(QIcon::fromTheme(QStringLiteral("view-history")));
        m_move_when_completed_history->setPopupMode(QToolButton::MenuButtonPopup);

        encodings = QTextCodec::availableMibs();
        foreach (int mib, encodings)
        {
            m_encoding->addItem(QString::fromUtf8(QTextCodec::codecForMib(mib)->name()));
        }

        if (!group_hint.isNull())
            initial_group = gman->find(group_hint);

        QButtonGroup* bg = new QButtonGroup(this);
        m_tree->setIcon(QIcon::fromTheme(QStringLiteral("view-list-tree")));
        m_tree->setToolTip(i18n("Show a file tree"));
        connect(m_tree, &QToolButton::clicked, this, &FileSelectDlg::fileTree);
        m_list->setIcon(QIcon::fromTheme(QStringLiteral("view-list-text")));
        m_list->setToolTip(i18n("Show a file list"));
        connect(m_list, &QToolButton::clicked, this, &FileSelectDlg::fileList);
        m_tree->setCheckable(true);
        m_list->setCheckable(true);
        bg->addButton(m_tree);
        bg->addButton(m_list);
        bg->setExclusive(true);

        m_filter->setClearButtonEnabled(true);
        m_filter->setPlaceholderText(i18n("Filter"));
        connect(m_filter, &QLineEdit::textChanged, this, &FileSelectDlg::setFilter);

        m_moveCompleted->setCheckState(Settings::useCompletedDir()?Qt::Checked:Qt::Unchecked);

        m_completedLocation->setEnabled(Settings::useCompletedDir());
        connect(m_moveCompleted, &QCheckBox::toggled, this, &FileSelectDlg::moveCompletedToggled);
    }

    FileSelectDlg::~FileSelectDlg()
    {}

    int FileSelectDlg::execute(bt::TorrentInterface* tc, bool* start, bool* skip_check, const QString& location_hint)
    {
        if (!tc)
            return QDialog::Rejected;

        setWindowTitle(i18n("Opening %1", tc->getDisplayName()));
        this->tc = tc;
        this->start = start;
        this->skip_check = skip_check;

        int idx = encodings.indexOf(tc->getTextCodec()->mibEnum());
        Out(SYS_GEN | LOG_DEBUG) << "Codec: " << QString::fromLatin1(tc->getTextCodec()->name()) << " " << idx << endl;
        m_encoding->setCurrentIndex(idx);
        connect(m_encoding, static_cast<void (KComboBox::*)(const QString &)>(&KComboBox::currentIndexChanged), this, &FileSelectDlg::onCodecChanged);


        for (Uint32 i = 0; i < tc->getNumFiles(); i++)
        {
            bt::TorrentFileInterface& file = tc->getTorrentFile(i);
            file.setEmitDownloadStatusChanged(false);
        }

        populateFields(location_hint);
        if (show_file_tree)
            model = new TorrentFileTreeModel(tc, TorrentFileTreeModel::DELETE_FILES, this);
        else
            model = new TorrentFileListModel(tc, TorrentFileTreeModel::DELETE_FILES, this);

        model->setFileNamesEditable(true);

        connect(model, &TorrentFileModel::checkStateChanged, this, &FileSelectDlg::updateSizeLabels);
        connect(m_downloadLocation, &KUrlRequester::textChanged, this, &FileSelectDlg::downloadLocationChanged);
        filter_model->setSourceModel(model);
        filter_model->setSortRole(Qt::UserRole);
        m_file_view->setSortingEnabled(true);
        m_file_view->expandAll();
        m_file_view->resizeColumnToContents(0);

        updateSizeLabels();

        bool multi_file_torrent = tc->getStats().multi_file_torrent;
        bool collapse_expand_enable = show_file_tree && multi_file_torrent;
        m_collapse_all->setEnabled(collapse_expand_enable);
        m_expand_all->setEnabled(collapse_expand_enable);

        m_select_all->setEnabled(multi_file_torrent);
        m_select_none->setEnabled(multi_file_torrent);
        m_invert_selection->setEnabled(multi_file_torrent);
        m_expand_all->setEnabled(multi_file_torrent);

        m_file_view->setRootIsDecorated(collapse_expand_enable);
        m_file_view->setAlternatingRowColors(false);
        return exec();
    }

    void FileSelectDlg::reject()
    {
        QDialog::reject();
    }

    void FileSelectDlg::accept()
    {
        QStringList pe_ex;

        QString cn = m_completedLocation->url().toLocalFile();
        if (!cn.endsWith(QLatin1Char('/'))) cn += QLatin1Char('/');
        if (m_moveCompleted->isChecked() && !cn.isEmpty())
        {
            move_on_completion_location_history.removeOne(cn);
            move_on_completion_location_history.prepend(cn);
        }

        QString dn = m_downloadLocation->url().toLocalFile();
        if (!dn.endsWith(QLatin1Char('/'))) dn += QLatin1Char('/');
        if (!dn.isEmpty())
        {
            download_location_history.removeOne(dn);
            download_location_history.prepend(dn);
        }


        QString tld = tc->getUserModifiedFileName();
        // If the move on completion is on, check completed dir for files of the torrent
        // but only if the completed directory is not selected
        if (m_moveCompleted->checkState() == Qt::Checked && dn != cn)
        {
            bool completed_files_found = false;
            bool all_found = true;
            QStringList cf;
            if (tc->getStats().multi_file_torrent)
            {
                for (Uint32 i = 0; i < tc->getNumFiles(); i++)
                {
                    bt::TorrentFileInterface& file = tc->getTorrentFile(i);
                    QString path = cn + tld + bt::DirSeparator() + file.getUserModifiedPath();
                    if (bt::Exists(path))
                    {
                        completed_files_found = true;
                        cf.append(file.getUserModifiedPath());
                    }
                    else
                        all_found = false;
                }
            }
            else
            {
                QString path = cn + tld;
                completed_files_found = bt::Exists(path);
            }

            if (completed_files_found)
            {
                QString msg;
                if (tc->getStats().multi_file_torrent)
                {
                    if (!all_found)
                        msg = i18n("Some files of this torrent have been found in the completed downloads directory. "
                                   "Do you want to import these files and use the completed downloads directory as the location?");
                    else
                        msg = i18n("All files of this torrent have been found in the completed downloads directory. "
                                   "Do you want to import these files and use the completed downloads directory as the location?");
                }
                else
                    msg = i18n("The file <b>%1</b> was found in the completed downloads directory. Do you want to import this file?", tld);

                // better ask the user if (s)he wants to delete the already existing data
                int ret = KMessageBox::questionYesNoList(0, msg, cf, QString::null);
                if (ret == KMessageBox::Yes)
                {
                    dn = cn;
                }
            }
        }

        if (!bt::Exists(dn))
        {
            try
            {
                if (KMessageBox::questionYesNo(this, i18n("The directory %1 does not exist, do you want to create it?", dn)) == KMessageBox::Yes)
                    MakePath(dn);
                else
                    return;
            }
            catch (bt::Error& err)
            {
                KMessageBox::error(this, err.toString());
                QDialog::reject();
                return;
            }
        }

        if (!bt::Exists(cn))
        {
            try
            {
                if (KMessageBox::questionYesNo(this, i18n("The directory %1 does not exist, do you want to create it?", cn)) == KMessageBox::Yes)
                    MakePath(cn);
                else
                    return;
            }
            catch (bt::Error& err)
            {
                KMessageBox::error(this, err.toString());
                QDialog::reject();
                return;
            }
        }


        for (Uint32 i = 0; i < tc->getNumFiles(); i++)
        {
            bt::TorrentFileInterface& file = tc->getTorrentFile(i);

            // check for preexisting files
            QString path = dn + tld + bt::DirSeparator() + file.getUserModifiedPath();
            if (bt::Exists(path))
                file.setPreExisting(true);

            if (file.doNotDownload() && file.isPreExistingFile())
            {
                // we have excluded a preexsting file
                pe_ex.append(file.getUserModifiedPath());
            }
            file.setPathOnDisk(path);
        }

        if (pe_ex.count() > 0)
        {
            QString msg = i18n("You have deselected the following existing files. "
                               "You will lose all data in these files, are you sure you want to do this?");
            // better ask the user if (s)he wants to delete the already existing data
            int ret = KMessageBox::warningYesNoList(0, msg, pe_ex, QString::null,
                                                    KGuiItem(i18n("Yes, delete the files")),
                                                    KGuiItem(i18n("No, keep the files")));
            if (ret == KMessageBox::No)
            {
                for (Uint32 i = 0; i < tc->getNumFiles(); i++)
                {
                    bt::TorrentFileInterface& file = tc->getTorrentFile(i);
                    if (file.doNotDownload() && file.isPreExistingFile())
                        file.setDoNotDownload(false);
                }
            }
        }

        //Setup custom download location
        QString ddir = tc->getDataDir();
        if (!ddir.endsWith(bt::DirSeparator()))
            ddir += bt::DirSeparator();

        if (tc->getStats().multi_file_torrent && tld != tc->getStats().torrent_name)
            tc->changeOutputDir(dn + tld, bt::TorrentInterface::FULL_PATH);
        else if (dn != ddir)
            tc->changeOutputDir(dn, 0);

        QStringList conflicting;
        if (qman->checkFileConflicts(tc, conflicting))
        {
            QString err = i18n("Opening the torrent <b>%1</b>, "
                               "would share one or more files with the following torrents. "
                               "Torrents are not allowed to write to the same files. "
                               "Please select a different location.", tc->getDisplayName());
            KMessageBox::errorList(this, err, conflicting);
            return;
        }

        for (Uint32 i = 0; i < tc->getNumFiles(); i++)
        {
            bt::TorrentFileInterface& file = tc->getTorrentFile(i);
            file.setEmitDownloadStatusChanged(true);
        }

        //Make it user controlled if needed
        *start = m_chkStartTorrent->isChecked();
        *skip_check = m_skip_data_check->isChecked();

        // set display name for non-multifile torrent as file name inside
        if (Settings::autoRenameSingleFileTorrents() && !tc->getStats().multi_file_torrent)
            tc->setDisplayName(QFileInfo(tc->getUserModifiedFileName()).completeBaseName());

        //Now add torrent to selected group
        if (m_cmbGroups->currentIndex() > 0)
        {
            QString groupName = m_cmbGroups->currentText();

            Group* group = gman->find(groupName);
            if (group)
            {
                group->addTorrent(tc, true);
                gman->saveGroups();
            }
        }

        // Set this value after the group policy is applied,
        // so that the user selection in the dialog is not
        // overwritten by the group policy
        if (m_moveCompleted->checkState() == Qt::Checked)
            tc->setMoveWhenCompletedDir(cn);
        else
            tc->setMoveWhenCompletedDir(QString());

        // update the last save directory
        Settings::setLastSaveDir(dn);
        Settings::self()->save();
        QDialog::accept();
    }

    QString FileSelectDlg::selectedGroup() const
    {
        if (m_cmbGroups->currentIndex() == 0)
            return QString();

        return m_cmbGroups->currentText();
    }


    void FileSelectDlg::selectAll()
    {
        model->checkAll();
    }

    void FileSelectDlg::selectNone()
    {
        model->uncheckAll();
    }

    void FileSelectDlg::invertSelection()
    {
        model->invertCheck();
    }

    void FileSelectDlg::populateFields(const QString& location_hint)
    {
        QString dir;
        QString comp_dir;
        if (!location_hint.isEmpty() && QDir(location_hint).exists())
        {
            dir = location_hint;
        }
        else
        {
            dir = Settings::saveDir();
            if (!Settings::useSaveDir() || dir.isNull())
            {
                dir = Settings::lastSaveDir();
                if (dir.isNull())
                    dir = QDir::homePath();
            }
        }

        comp_dir = Settings::completedDir();
        if (!Settings::useCompletedDir() || comp_dir.isEmpty())
        {
            comp_dir = dir;
        }

        m_downloadLocation->setUrl(QUrl::fromLocalFile(dir));
        m_completedLocation->setUrl(QUrl::fromLocalFile(comp_dir));
        loadGroups();
    }

    void FileSelectDlg::loadGroups()
    {
        GroupManager::Itr it = gman->begin();

        QStringList grps;

        //First default group
        grps << i18n("All Torrents");

        int cnt = 0;
        int selected = 0;
        //now custom ones
        while (it != gman->end())
        {
            grps << it->first;
            if (!it->second->isStandardGroup())
            {
                if (it->second == initial_group)
                    selected = cnt + 1;
                cnt++;
            }
            ++it;
        }

        m_cmbGroups->addItems(grps);
        connect(m_cmbGroups, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated), this, &FileSelectDlg::groupActivated);

        if (selected > 0 && initial_group)
        {
            m_cmbGroups->setCurrentIndex(selected);
            QString dir = initial_group->groupPolicy().default_save_location;
            if (!dir.isEmpty() && bt::Exists(dir))
                m_downloadLocation->setUrl(QUrl(dir));

            dir = initial_group->groupPolicy().default_move_on_completion_location;
            if (!dir.isEmpty() && bt::Exists(dir))
                m_completedLocation->setUrl(QUrl::fromLocalFile(dir));
        }
    }

    void FileSelectDlg::groupActivated(int idx)
    {
        if (idx == 0)
            return; // No group selected

        // find the selected group
        Group* g = gman->find(m_cmbGroups->itemText(idx));
        if (!g)
            return;

        QString dir = g->groupPolicy().default_save_location;
        if (!dir.isEmpty() && bt::Exists(dir))
            m_downloadLocation->setUrl(QUrl(dir));

        dir = g->groupPolicy().default_move_on_completion_location;
        if (!dir.isEmpty() && bt::Exists(dir))
        {
            m_moveCompleted->setChecked(true);
            m_completedLocation->setUrl(QUrl::fromLocalFile(dir));
        }
        else
        {
            m_moveCompleted->setChecked(false);
        }
    }

    void FileSelectDlg::updateSizeLabels()
    {
        if (!model)
            return;

        updateExistingFiles();

        //calculate free disk space
        QUrl sdir = m_downloadLocation->url();
        while (sdir.isValid() && sdir.isLocalFile() && (!sdir.isEmpty())  && (! QDir(sdir.toLocalFile()).exists()))
        {
            sdir = KIO::upUrl(sdir);
        }

        Uint64 bytes_free = 0;
        if (!FreeDiskSpace(sdir.toLocalFile(), bytes_free))
        {
            lblRequired->setText(bt::BytesToString(model->bytesToDownload()));
            lblFree->setText(i18n("<b>Unable to determine free space</b>"));
            lblStatus->clear();
        }
        else
        {
            Uint64 bytes_to_download = model->bytesToDownload();

            lblFree->setText(bt::BytesToString(bytes_free));
            if (already_downloaded > 0)
                lblRequired->setText(
                    i18n("%1 (%2 in use by existing files)",
                         bt::BytesToString(bytes_to_download),
                         bt::BytesToString(already_downloaded)));
            else
                lblRequired->setText(bt::BytesToString(bytes_to_download));

            bytes_to_download -= already_downloaded;
            if (bytes_to_download > bytes_free)
                lblStatus->setText(
                    QLatin1String("<font color=\"#ff0000\">") + i18nc("We are %1 bytes short of what we need", "%1 short",
                                                       bt::BytesToString(-1 * (long long)(bytes_free - bytes_to_download))));
            else
                lblStatus->setText(bt::BytesToString(bytes_free - bytes_to_download));
        }
    }

    void FileSelectDlg::updateExistingFiles()
    {
        if (tc->getStats().multi_file_torrent)
        {
            already_downloaded = 0;
            bt::Uint32 found = 0;
            QString path = m_downloadLocation->url().path() + QLatin1Char('/') + tc->getDisplayName() + QLatin1Char('/');
            for (bt::Uint32 i = 0; i < tc->getNumFiles(); i++)
            {
                const bt::TorrentFileInterface& file = tc->getTorrentFile(i);
                if (bt::Exists(path + file.getUserModifiedPath()))
                {
                    found++;
                    if (!file.doNotDownload()) // Do not include excluded files in the already downloaded calculation
                    {
                        bt::Uint64 size = bt::FileSize(path + file.getUserModifiedPath());
                        if (size <= file.getSize())
                            already_downloaded += file.getSize() - size;
                        else
                            already_downloaded += file.getSize();
                    }
                }
            }

            if (found == 0)
                m_existing_found->setText(i18n("Existing files: <b>None</b>"));
            else if (found == tc->getNumFiles())
                m_existing_found->setText(i18n("Existing files: <b>All</b>"));
            else
                m_existing_found->setText(i18n("Existing files: <b>%1</b> of <b>%2</b>", found, tc->getNumFiles()));
        }
        else
        {
            QString path = m_downloadLocation->url().path() + QLatin1Char('/') + tc->getDisplayName();
            if (!bt::Exists(path))
            {
                m_existing_found->setText(i18n("Existing file: <b>No</b>"));
            }
            else
            {
                already_downloaded = bt::FileSize(path);
                m_existing_found->setText(i18n("Existing file: <b>Yes</b>"));
            }
        }
    }

    void FileSelectDlg::downloadLocationChanged(const QString& path)
    {
        Q_UNUSED(path);
        updateSizeLabels();
    }


    void FileSelectDlg::onCodecChanged(const QString& text)
    {
        QTextCodec* codec = QTextCodec::codecForName(text.toLocal8Bit());
        if (codec)
        {
            tc->changeTextCodec(codec);
            model->onCodecChange();
        }
    }


    void FileSelectDlg::loadState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("FileSelectDlg");
        QSize s = g.readEntry("size", sizeHint());
        resize(s);
        show_file_tree = g.readEntry("show_file_tree", true);
        m_tree->setChecked(show_file_tree);
        m_list->setChecked(!show_file_tree);
        download_location_history = g.readEntry("download_location_history", QStringList());
        for (int i=0; i<download_location_history.count(); i++)
            if (download_location_history.at(i).endsWith(QLatin1String("//"))) download_location_history[i].chop(1);
        download_location_history.removeDuplicates();
        move_on_completion_location_history = g.readEntry("move_on_completion_location_history", QStringList());
        move_on_completion_location_history.removeDuplicates();

        if (download_location_history.count())
        {
            QMenu* m = createHistoryMenu(download_location_history,
                                         SLOT(downloadLocationHistoryTriggered(QAction*)));
            m_download_location_history->setMenu(m);
        }
        else
            m_download_location_history->setEnabled(false);

        if (move_on_completion_location_history.count())
        {
            QMenu* m = createHistoryMenu(
                           move_on_completion_location_history,
                           SLOT(moveOnCompletionLocationHistoryTriggered(QAction*)));
            m_move_when_completed_history->setMenu(m);
        }
        else
            m_move_when_completed_history->setEnabled(false);

        QByteArray state = g.readEntry("file_view", QByteArray());
        if (state.size() > 0)
            m_file_view->header()->restoreState(state);
    }


    void FileSelectDlg::saveState(KSharedConfigPtr cfg)
    {
        KConfigGroup g = cfg->group("FileSelectDlg");
        g.writeEntry("size", size());
        g.writeEntry("show_file_tree", show_file_tree);
        g.writeEntry("download_location_history", download_location_history);
        g.writeEntry("move_on_completion_location_history", move_on_completion_location_history);
        g.writeEntry("file_view", m_file_view->header()->saveState());
    }

    QMenu* FileSelectDlg::createHistoryMenu(const QStringList & urls, const char* slot)
    {
        QMenu* m = new QMenu(this);
        foreach (const QString& url, urls)
        {
            QAction* a = m->addAction(url);
            a->setData(url);
        }
        m->addSeparator();
        m->addAction(i18n("Clear History"));
        connect(m, SIGNAL(triggered(QAction*)), this, slot);
        return m;
    }

    void FileSelectDlg::clearDownloadLocationHistory()
    {
        download_location_history.clear();
        m_download_location_history->setEnabled(false);
    }

    void FileSelectDlg::clearMoveOnCompletionLocationHistory()
    {
        move_on_completion_location_history.clear();
        m_move_when_completed_history->setEnabled(false);
    }

    void FileSelectDlg::downloadLocationHistoryTriggered(QAction* act)
    {
        if (!act->data().isNull())
            m_downloadLocation->setUrl(QUrl::fromLocalFile(act->data().toString()));
        else
            clearDownloadLocationHistory();
    }
    void FileSelectDlg::moveOnCompletionLocationHistoryTriggered(QAction* act)
    {
        if (!act->data().isNull())
            m_completedLocation->setUrl(QUrl::fromLocalFile(act->data().toString()));
        else
            clearMoveOnCompletionLocationHistory();
    }


    void FileSelectDlg::fileTree(bool)
    {
        setShowFileTree(true);
    }

    void FileSelectDlg::fileList(bool)
    {
        setShowFileTree(false);
    }

    void FileSelectDlg::setShowFileTree(bool on)
    {
        if (show_file_tree == on)
            return;

        show_file_tree = on;
        QByteArray hs = m_file_view->header()->saveState();

        filter_model->setSourceModel(0);
        delete model;
        if (show_file_tree)
            model = new TorrentFileTreeModel(tc, TorrentFileTreeModel::DELETE_FILES, this);
        else
            model = new TorrentFileListModel(tc, TorrentFileTreeModel::DELETE_FILES, this);

        model->setFileNamesEditable(true);
        connect(model, &TorrentFileModel::checkStateChanged, this, &FileSelectDlg::updateSizeLabels);

        filter_model->setSourceModel(model);
        m_file_view->header()->restoreState(hs);
        m_file_view->expandAll();
        m_file_view->setRootIsDecorated(show_file_tree && tc->getStats().multi_file_torrent);

        m_collapse_all->setEnabled(show_file_tree);
        m_expand_all->setEnabled(show_file_tree);
    }

    void FileSelectDlg::setFilter(const QString& f)
    {
        Q_UNUSED(f);
        filter_model->setFilterFixedString(m_filter->text());
    }

    void FileSelectDlg::moveCompletedToggled(bool on)
    {
        m_completedLocation->setEnabled(on);
    }
}
