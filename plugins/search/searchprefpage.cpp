/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2005-2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchprefpage.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QFile>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QToolTip>
#include <QUrl>

#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
#include <KIconLoader>
#include <KLocalizedString>
#include <KMessageBox>

#include "opensearchdownloadjob.h"
#include "searchenginelist.h"
#include "searchplugin.h"
#include "searchpluginsettings.h"
#include <interfaces/functions.h>
#include <util/constants.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
SearchPrefPage::SearchPrefPage(SearchPlugin *plugin, SearchEngineList *sl, QWidget *parent)
    : PrefPageInterface(SearchPluginSettings::self(), i18nc("plugin name", "Search"), QStringLiteral("edit-find"), parent)
    , plugin(plugin)
    , engines(sl)
{
    setupUi(this);
    m_engines->setModel(sl);

    connect(m_add, &QPushButton::clicked, this, &SearchPrefPage::addClicked);
    connect(m_remove, &QPushButton::clicked, this, &SearchPrefPage::removeClicked);
    connect(m_add_default, &QPushButton::clicked, this, &SearchPrefPage::addDefaultClicked);
    connect(m_remove_all, &QPushButton::clicked, this, &SearchPrefPage::removeAllClicked);
    connect(m_clear_history, &QPushButton::clicked, this, &SearchPrefPage::clearHistory);
    connect(m_engines->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SearchPrefPage::selectionChanged);
    connect(m_reset_default_action, &QPushButton::clicked, this, &SearchPrefPage::resetDefaultAction);

    connect(kcfg_useCustomBrowser, &QRadioButton::toggled, this, &SearchPrefPage::customToggled);
    connect(kcfg_openInExternal, &QCheckBox::toggled, this, &SearchPrefPage::openInExternalToggled);
    QButtonGroup *bg = new QButtonGroup(this);
    bg->addButton(kcfg_useCustomBrowser);
    bg->addButton(kcfg_useDefaultBrowser);

    m_remove_all->setEnabled(sl->rowCount(QModelIndex()) > 0);
    m_remove->setEnabled(false);
}

SearchPrefPage::~SearchPrefPage()
{
}

void SearchPrefPage::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
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
    QString name = QInputDialog::getText(this, i18n("Add a Search Engine"), i18n("Enter the hostname of the search engine (for example www.google.com):"));
    if (name.isEmpty())
        return;

    if (!name.startsWith(QLatin1String("http://")) || !name.startsWith(QLatin1String("https://")))
        name = QLatin1String("http://") + name;

    QUrl url(name);
    QString dir = kt::DataDir() + QLatin1String("searchengines/") + url.host();
    int idx = 1;
    while (bt::Exists(dir)) {
        dir += QString::number(idx++);
    }

    dir += QLatin1Char('/');

    try {
        bt::MakeDir(dir, false);
    } catch (bt::Error &err) {
        KMessageBox::error(this, err.toString());
        return;
    }

    OpenSearchDownloadJob *j = new OpenSearchDownloadJob(url, dir, plugin->getProxy());
    connect(j, &OpenSearchDownloadJob::result, this, &SearchPrefPage::downloadJobFinished);
    j->start();
}

void SearchPrefPage::downloadJobFinished(KJob *j)
{
    OpenSearchDownloadJob *osdj = (OpenSearchDownloadJob *)j;
    if (osdj->error()) {
        QString msg = i18n(
            "Opensearch is not supported by %1, you will need to enter the search URL manually. "
            "The URL should contain {searchTerms}, ktorrent will replace this by the thing you are searching for.",
            osdj->hostname());
        QString url = QInputDialog::getText(this, i18n("Add a Search Engine"), msg);
        if (!url.isEmpty()) {
            if (!url.contains(QLatin1String("{searchTerms}"))) {
                KMessageBox::error(this, i18n("The URL %1 does not contain {searchTerms}.", url));
            } else {
                try {
                    engines->addEngine(osdj->directory(), url);
                } catch (bt::Error &err) {
                    KMessageBox::error(this, err.toString());
                    bt::Delete(osdj->directory(), true);
                }
            }
        }
    } else {
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
    kcfg_useProxySettings->setEnabled(!on);
    kcfg_customBrowser->setEnabled(on && SearchPluginSettings::useCustomBrowser());
    kcfg_useDefaultBrowser->setEnabled(on);
}

void SearchPrefPage::clearHistory()
{
    Q_EMIT clearSearchHistory();
}

void SearchPrefPage::resetDefaultAction()
{
    KMessageBox::enableMessage(QStringLiteral(":TorrentDownloadFinishedQuestion"));
}

}
