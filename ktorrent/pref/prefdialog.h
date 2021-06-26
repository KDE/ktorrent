/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_PREFDIALOG_HH
#define KT_PREFDIALOG_HH

#include <QList>
#include <QScrollArea>

#include <KConfigDialog>

namespace kt
{
class Core;
class PrefPageInterface;
class NetworkPref;
class QMPref;
class PrefPageScrollArea;

/**
 * KTorrent's preferences dialog, this uses the new KConfigDialog class which should make our live much easier.
 * In order for this to work properly the widgets have to be named kcfg_FOO where FOO is a somehting out of our settings class.
 *
 * The use of KConfigDialog should deprecate PrefPageInterface.
 * */
class PrefDialog : public KConfigDialog
{
    Q_OBJECT
public:
    PrefDialog(QWidget *parent, Core *core);
    ~PrefDialog() override;

    /**
     * Add a pref page to the dialog.
     * @param page The page
     * */
    void addPrefPage(PrefPageInterface *page);

    /**
     * Remove a pref page.
     * @param page The page
     * */
    void removePrefPage(PrefPageInterface *page);

    /**
     * Update the widgets and show
     */
    void updateWidgetsAndShow();

    /**
     * Load the state of the dialog
     */
    void loadState(KSharedConfigPtr cfg);

    /**
     * Save the state of the dialog
     */
    void saveState(KSharedConfigPtr cfg);

protected:
    void updateWidgets() override;
    void updateWidgetsDefault() override;
    void updateSettings() override;
    bool hasChanged() override;

private Q_SLOTS:
    void calculateRecommendedSettings();

private:
    QList<PrefPageScrollArea *> pages;
    NetworkPref *net_pref;
    QMPref *qm_pref;
};

class PrefPageScrollArea : public QScrollArea
{
public:
    PrefPageScrollArea(PrefPageInterface *page, QWidget *parent = nullptr);
    ~PrefPageScrollArea() override;

    PrefPageInterface *page;
    KPageWidgetItem *page_widget_item;
};
}

#endif
