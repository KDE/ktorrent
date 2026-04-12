/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2005-2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchprefpage.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFormLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QToolTip>
#include <QUrl>
#include <QVBoxLayout>

#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
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
namespace
{
class TorznabEditDialog : public QDialog
{
public:
    explicit TorznabEditDialog(QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle(i18n("Jackett / Torznab Engine"));

        auto *layout = new QVBoxLayout(this);
        auto *form = new QFormLayout;
        layout->addLayout(form);

        nameEdit = new QLineEdit(this);
        descriptionEdit = new QLineEdit(this);
        urlEdit = new QLineEdit(this);
        apiKeyEdit = new QLineEdit(this);
        threadCountSpin = new QSpinBox(this);
        trackerFirstCheck = new QCheckBox(i18n("Show tracker name before the result title"), this);

        threadCountSpin->setRange(1, 100);
        threadCountSpin->setValue(20);

        form->addRow(i18n("Name:"), nameEdit);
        form->addRow(i18n("Description:"), descriptionEdit);
        form->addRow(i18n("Jackett URL:"), urlEdit);
        form->addRow(i18n("API key:"), apiKeyEdit);
        form->addRow(i18n("Concurrent requests:"), threadCountSpin);
        form->addRow(QString(), trackerFirstCheck);

        auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            if (validate()) {
                accept();
            }
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    }

    void setConfig(const TorznabEngineConfig &config)
    {
        nameEdit->setText(config.name);
        descriptionEdit->setText(config.description);
        urlEdit->setText(config.serviceUrl.toString());
        apiKeyEdit->setText(config.apiKey);
        threadCountSpin->setValue(config.threadCount);
        trackerFirstCheck->setChecked(config.trackerFirst);
    }

    TorznabEngineConfig config() const
    {
        TorznabEngineConfig config;
        config.name = nameEdit->text().trimmed();
        config.description = descriptionEdit->text().trimmed();
        config.serviceUrl = QUrl(urlEdit->text().trimmed());
        config.apiKey = apiKeyEdit->text().trimmed();
        config.threadCount = threadCountSpin->value();
        config.trackerFirst = trackerFirstCheck->isChecked();
        return config;
    }

private:
    bool validate()
    {
        const TorznabEngineConfig current = config();
        if (current.name.isEmpty()) {
            KMessageBox::error(this, i18n("The engine name cannot be empty."));
            return false;
        }

        if (!current.serviceUrl.isValid() || current.serviceUrl.host().isEmpty()
            || (current.serviceUrl.scheme() != QLatin1String("http") && current.serviceUrl.scheme() != QLatin1String("https"))) {
            KMessageBox::error(this, i18n("The Jackett URL must be a valid HTTP or HTTPS address."));
            return false;
        }

        if (current.apiKey.isEmpty()) {
            KMessageBox::error(this, i18n("The API key cannot be empty."));
            return false;
        }

        return true;
    }

    QLineEdit *nameEdit = nullptr;
    QLineEdit *descriptionEdit = nullptr;
    QLineEdit *urlEdit = nullptr;
    QLineEdit *apiKeyEdit = nullptr;
    QSpinBox *threadCountSpin = nullptr;
    QCheckBox *trackerFirstCheck = nullptr;
};

QString sanitizedEngineDirName(const QString &seed)
{
    QString base = seed.trimmed().toLower();
    base.replace(QRegularExpression(QStringLiteral("[^a-z0-9._-]+")), QStringLiteral("_"));
    base.remove(QRegularExpression(QStringLiteral("^_+|_+$")));
    return base.isEmpty() ? QStringLiteral("torznab") : base;
}

QString uniqueEngineDir(const QString &baseDir, const QString &seed)
{
    QString dir = baseDir + sanitizedEngineDirName(seed);
    int suffix = 1;
    while (bt::Exists(dir)) {
        dir = baseDir + sanitizedEngineDirName(seed) + QString::number(suffix++);
    }

    if (!dir.endsWith(QLatin1Char('/'))) {
        dir += QLatin1Char('/');
    }

    return dir;
}
}

SearchPrefPage::SearchPrefPage(SearchPlugin *plugin, SearchEngineList *sl, QWidget *parent)
    : PrefPageInterface(SearchPluginSettings::self(), i18nc("plugin name", "Search"), QStringLiteral("edit-find"), parent)
    , plugin(plugin)
    , engines(sl)
{
    setupUi(this);
    m_engines->setModel(sl);

    connect(m_add, &QPushButton::clicked, this, &SearchPrefPage::addClicked);
    connect(m_add_torznab, &QPushButton::clicked, this, &SearchPrefPage::addTorznabClicked);
    connect(m_edit, &QPushButton::clicked, this, &SearchPrefPage::editClicked);
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
    m_edit->setEnabled(false);
}

SearchPrefPage::~SearchPrefPage()
{
}

void SearchPrefPage::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)
    m_remove->setEnabled(selected.count() > 0);

    const QModelIndexList rows = m_engines->selectionModel()->selectedRows();
    const SearchEngine *engine = rows.count() == 1 ? engines->engine(rows.front().row()) : nullptr;
    m_edit->setEnabled(engine && engine->isTorznab());
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
    if (name.isEmpty()) {
        return;
    }

    if (!name.startsWith(QLatin1String("http://")) && !name.startsWith(QLatin1String("https://"))) {
        name = QLatin1String("http://") + name;
    }

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

void SearchPrefPage::addTorznabClicked()
{
    TorznabEditDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const TorznabEngineConfig config = dialog.config();
    const QString dir =
        uniqueEngineDir(kt::DataDir() + QLatin1String("searchengines/"), !config.serviceUrl.host().isEmpty() ? config.serviceUrl.host() : config.name);

    QString errorMessage;
    if (!engines->addTorznabEngine(dir, config, &errorMessage)) {
        KMessageBox::error(this, errorMessage);
        bt::Delete(dir, true);
        return;
    }

    m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
}

void SearchPrefPage::editClicked()
{
    const QModelIndexList selectedRows = m_engines->selectionModel()->selectedRows();
    if (selectedRows.count() != 1) {
        return;
    }

    SearchEngine *engine = engines->engine(selectedRows.front().row());
    if (!engine || !engine->isTorznab()) {
        return;
    }

    TorznabEditDialog dialog(this);
    dialog.setConfig(engine->torznabConfig());
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QString errorMessage;
    if (!engines->updateTorznabEngine(selectedRows.front(), dialog.config(), &errorMessage)) {
        KMessageBox::error(this, errorMessage);
    }
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

#include "moc_searchprefpage.cpp"
