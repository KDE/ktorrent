/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include <KLocalizedString>
#include <KConfigDialogManager>

#include "settings.h"
#include "prefdialog.h"
#include "core.h"
#include "generalpref.h"
#include "advancedpref.h"
#include "networkpref.h"
#include "proxypref.h"
#include "qmpref.h"
#include "btpref.h"
#include "recommendedsettingsdlg.h"

namespace kt
{



    PrefDialog::PrefDialog(QWidget* parent, Core* core) : KConfigDialog(parent, QStringLiteral("settings"), Settings::self())
    {
        KConfigDialogManager::propertyMap()->insert(QStringLiteral("KUrlRequester"), QByteArrayLiteral("url"));
        setFaceType(KPageDialog::List);
        connect(this, SIGNAL(settingsChanged(const QString&)), core, SLOT(applySettings()));
        addPrefPage(new GeneralPref(this));
        net_pref = new NetworkPref(this);
        addPrefPage(net_pref);
        addPrefPage(new ProxyPref(this));
        addPrefPage(new BTPref(this));
        qm_pref = new QMPref(this);
        addPrefPage(qm_pref);
        addPrefPage(new AdvancedPref(this));

        connect(net_pref, &NetworkPref::calculateRecommendedSettings, this, &PrefDialog::calculateRecommendedSettings);
    }

    PrefDialog::~PrefDialog()
    {
    }

    void PrefDialog::addPrefPage(PrefPageInterface* page)
    {
        PrefPageScrollArea* area = new PrefPageScrollArea(page, this);
        connect(area->page, &PrefPageInterface::updateButtons, this, &PrefDialog::updateButtons);

        KPageWidgetItem* p = addPage(area, page->config(), page->pageName(), page->pageIcon());
        area->page_widget_item = p;
        pages.append(area);
        if (!isHidden())
            page->loadSettings();
    }

    void PrefDialog::removePrefPage(PrefPageInterface* page)
    {
        foreach (PrefPageScrollArea* area, pages)
        {
            if (area->page == page)
            {
                area->takeWidget();
                pages.removeAll(area);
                removePage(area->page_widget_item);
                break;
            }
        }
    }

    void PrefDialog::updateWidgetsAndShow()
    {
        updateWidgets();
        show();
    }

    void PrefDialog::updateWidgets()
    {
        foreach (PrefPageScrollArea* area, pages)
            area->page->loadSettings();
    }

    void PrefDialog::updateWidgetsDefault()
    {
        foreach (PrefPageScrollArea* area, pages)
            area->page->loadDefaults();
    }

    void PrefDialog::updateSettings()
    {
        foreach (PrefPageScrollArea* area, pages)
            area->page->updateSettings();
    }

    void PrefDialog::calculateRecommendedSettings()
    {
        RecommendedSettingsDlg dlg(this);
        if (dlg.exec() == QDialog::Accepted)
        {
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

        foreach (PrefPageScrollArea* area, pages)
            if (area->page->customWidgetsChanged())
                return true;

        return false;
    }


    ///////////////////////////////////////

    PrefPageScrollArea::PrefPageScrollArea(kt::PrefPageInterface* page, QWidget* parent) : QScrollArea(parent), page(page), page_widget_item(nullptr)
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
