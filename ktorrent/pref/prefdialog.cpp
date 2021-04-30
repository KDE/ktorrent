/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfigDialogManager>
#include <KLocalizedString>
#include <KSharedConfig>

#include "advancedpref.h"
#include "btpref.h"
#include "colorpref.h"
#include "core.h"
#include "generalpref.h"
#include "networkpref.h"
#include "prefdialog.h"
#include "proxypref.h"
#include "qmpref.h"
#include "recommendedsettingsdlg.h"
#include "settings.h"

namespace kt
{
PrefDialog::PrefDialog(QWidget *parent, Core *core)
    : KConfigDialog(parent, QStringLiteral("settings"), Settings::self())
{
    KConfigDialogManager::propertyMap()->insert(QStringLiteral("KUrlRequester"), QByteArrayLiteral("url"));
    setFaceType(KPageDialog::List);
    connect(this, &PrefDialog::settingsChanged, [core](const QString &) {
        core->applySettings();
    });
    addPrefPage(new GeneralPref(this));
    net_pref = new NetworkPref(this);
    addPrefPage(net_pref);
    addPrefPage(new ProxyPref(this));
    addPrefPage(new BTPref(this));
    qm_pref = new QMPref(this);
    addPrefPage(qm_pref);
    addPrefPage(new ColorPref(this));
    addPrefPage(new AdvancedPref(this));

    connect(net_pref, &NetworkPref::calculateRecommendedSettings, this, &PrefDialog::calculateRecommendedSettings);
}

PrefDialog::~PrefDialog()
{
}

void PrefDialog::addPrefPage(PrefPageInterface *page)
{
    PrefPageScrollArea *area = new PrefPageScrollArea(page, this);
    connect(area->page, &PrefPageInterface::updateButtons, this, &PrefDialog::updateButtons);

    KPageWidgetItem *p = addPage(area, page->config(), page->pageName(), page->pageIcon());
    area->page_widget_item = p;
    pages.append(area);
    if (!isHidden())
        page->loadSettings();
}

void PrefDialog::removePrefPage(PrefPageInterface *page)
{
    PrefPageScrollArea *found = nullptr;
    for (PrefPageScrollArea *area : qAsConst(pages)) {
        if (area->page == page) {
            found = area;
            break;
        }
    }

    if (found) {
        found->takeWidget();
        pages.removeAll(found);
        removePage(found->page_widget_item);
    }
}

void PrefDialog::updateWidgetsAndShow()
{
    updateWidgets();
    show();
}

void PrefDialog::updateWidgets()
{
    for (PrefPageScrollArea *area : qAsConst(pages))
        area->page->loadSettings();
}

void PrefDialog::updateWidgetsDefault()
{
    for (PrefPageScrollArea *area : qAsConst(pages))
        area->page->loadDefaults();
}

void PrefDialog::updateSettings()
{
    for (PrefPageScrollArea *area : qAsConst(pages))
        area->page->updateSettings();
}

void PrefDialog::calculateRecommendedSettings()
{
    RecommendedSettingsDlg dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        qm_pref->kcfg_maxSeeds->setValue(dlg.max_seeds);
        qm_pref->kcfg_maxDownloads->setValue(dlg.max_downloads);
        qm_pref->kcfg_numUploadSlots->setValue(dlg.max_slots);
        net_pref->kcfg_maxDownloadRate->setValue(dlg.max_download_speed);
        net_pref->kcfg_maxUploadRate->setValue(dlg.max_upload_speed);
        net_pref->kcfg_maxConnections->setValue(dlg.max_conn_tor);
        net_pref->kcfg_maxTotalConnections->setValue(dlg.max_conn_glob);
    }
}

void PrefDialog::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("PrefDialog");
    QSize s = g.readEntry("size", sizeHint());
    resize(s);
}

void PrefDialog::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("PrefDialog");
    g.writeEntry("size", size());
}

bool PrefDialog::hasChanged()
{
    if (KConfigDialog::hasChanged())
        return true;

    for (PrefPageScrollArea *area : qAsConst(pages))
        if (area->page->customWidgetsChanged())
            return true;

    return false;
}

///////////////////////////////////////

PrefPageScrollArea::PrefPageScrollArea(kt::PrefPageInterface *page, QWidget *parent)
    : QScrollArea(parent)
    , page(page)
    , page_widget_item(nullptr)
{
    setWidget(page);
    setWidgetResizable(true);
    setFrameStyle(QFrame::NoFrame);
    viewport()->setAutoFillBackground(false);
}

PrefPageScrollArea::~PrefPageScrollArea()
{
}

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; mixed-indent off;
