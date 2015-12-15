/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson, Ivan Vasic                  *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "searchprefpage.h"

#include <QUrl>
#include <QLineEdit>
#include <QToolTip>
#include <QFile>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QInputDialog>

#include <klocalizedstring.h>

#include <kiconloader.h>

#include <kmessagebox.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>

#include <util/log.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include "searchplugin.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"
#include "opensearchdownloadjob.h"

using namespace bt;

namespace kt
{
    SearchPrefPage::SearchPrefPage(SearchPlugin* plugin, SearchEngineList* sl, QWidget* parent)
        : PrefPageInterface(SearchPluginSettings::self(), i18nc("plugin name", "Search"), QStringLiteral("edit-find"), parent), plugin(plugin), engines(sl)
    {
        setupUi(this);
        m_engines->setModel(sl);

        connect(m_add, SIGNAL(clicked()), this, SLOT(addClicked()));
        connect(m_remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
        connect(m_add_default, SIGNAL(clicked()), this, SLOT(addDefaultClicked()));
        connect(m_remove_all, SIGNAL(clicked()), this, SLOT(removeAllClicked()));
        connect(m_clear_history, SIGNAL(clicked()), this, SLOT(clearHistory()));
        connect(m_engines->selectionModel(), SIGNAL(selectionChanged(const QItemSelection& , const QItemSelection&)),
                this, SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
        connect(m_reset_default_action, SIGNAL(clicked()), this, SLOT(resetDefaultAction()));

        connect(kcfg_useCustomBrowser, SIGNAL(toggled(bool)), this, SLOT(customToggled(bool)));
        connect(kcfg_openInExternal, SIGNAL(toggled(bool)), this, SLOT(openInExternalToggled(bool)));
        QButtonGroup* bg = new QButtonGroup(this);
        bg->addButton(kcfg_useCustomBrowser);
        bg->addButton(kcfg_useDefaultBrowser);

        m_remove_all->setEnabled(sl->rowCount(QModelIndex()) > 0);
        m_remove->setEnabled(false);
    }


    SearchPrefPage::~SearchPrefPage()
    {}

    void SearchPrefPage::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
        Q_UNUSED(deselected);
        m_remove->setEnabled(selected.count() > 0);
    }

    void SearchPrefPage::loadSettings()
    {
        openInExternalToggled(SearchPluginSettings::openInExternal());
    }

    void SearchPrefPage::loadDefaults()
    {
        loadSettings();
    }

    void SearchPrefPage::addClicked()
    {
        QString name = QInputDialog::getText(this, i18n("Add a Search Engine"),
                                             i18n("Enter the hostname of the search engine (for example www.google.com):"));
        if (name.isEmpty())
            return;

        if (!name.startsWith(QLatin1String("http://")) || !name.startsWith(QLatin1String("https://")))
            name = QLatin1String("http://") + name;

        QUrl url(name);
        QString dir = kt::DataDir() + QLatin1String("searchengines/") + url.host();
        int idx = 1;
        while (bt::Exists(dir))
        {
            dir += QString::number(idx++);
        }

        dir += '/';

        try
        {
            bt::MakeDir(dir, false);
        }
        catch (bt::Error& err)
        {
            KMessageBox::error(this, err.toString());
            return;
        }

        OpenSearchDownloadJob* j = new OpenSearchDownloadJob(url, dir);
        connect(j, SIGNAL(result(KJob*)), this, SLOT(downloadJobFinished(KJob*)));
        j->start();
    }

    void SearchPrefPage::downloadJobFinished(KJob* j)
    {
        OpenSearchDownloadJob* osdj = (OpenSearchDownloadJob*)j;
        if (osdj->error())
        {
            QString msg = i18n("Opensearch is not supported by %1, you will need to enter the search URL manually. "
                               "The URL should contain {searchTerms}, ktorrent will replace this by the thing you are searching for.", osdj->hostname());
            QString url = QInputDialog::getText(this, i18n("Add a Search Engine"), msg);
            if (!url.isEmpty())
            {
                if (!url.contains(QLatin1String("{searchTerms}")))
                {
                    KMessageBox::error(this, i18n("The URL %1 does not contain {searchTerms}.", url));
                }
                else
                {
                    try
                    {
                        engines->addEngine(osdj->directory(), url);
                    }
                    catch (bt::Error& err)
                    {
                        KMessageBox::error(this, err.toString());
                        bt::Delete(osdj->directory(), true);
                    }
                }
            }
        }
        else
        {
            engines->addEngine(osdj);
        }
    }

    void SearchPrefPage::removeClicked()
    {
        QModelIndexList sel = m_engines->selectionModel()->selectedRows();
        engines->removeEngines(sel);
        m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
        m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
    }

    void SearchPrefPage::addDefaultClicked()
    {
        engines->addDefaults();
        m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
        m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
    }

    void SearchPrefPage::removeAllClicked()
    {
        engines->removeAllEngines();
        m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
        m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
    }

    void SearchPrefPage::customToggled(bool toggled)
    {
        kcfg_customBrowser->setEnabled(toggled);
    }

    void SearchPrefPage::openInExternalToggled(bool on)
    {
        kcfg_useCustomBrowser->setEnabled(on);
        kcfg_customBrowser->setEnabled(on && SearchPluginSettings::useCustomBrowser());
        kcfg_useDefaultBrowser->setEnabled(on);
    }

    void SearchPrefPage::clearHistory()
    {
        emit clearSearchHistory();
    }

    void SearchPrefPage::resetDefaultAction()
    {
        KMessageBox::enableMessage(":TorrentDownloadFinishedQuestion");
    }

}

