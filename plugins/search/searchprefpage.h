/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSEARCHPREFPAGE_H
#define KTSEARCHPREFPAGE_H

#include <QString>

#include "ui_searchpref.h"
#include <interfaces/prefpageinterface.h>
class KJob;

namespace kt
{
class SearchPlugin;
class SearchEngineList;

/**
 * @author Joris Guisson
 *
 * Preference page for the search plugin.
 */
class SearchPrefPage : public PrefPageInterface, public Ui_SearchPref
{
    Q_OBJECT
public:
    SearchPrefPage(SearchPlugin *plugin, SearchEngineList *sl, QWidget *parent);
    ~SearchPrefPage() override;

    void loadSettings() override;
    void loadDefaults() override;

public Q_SLOTS:
    void customToggled(bool toggled);

private Q_SLOTS:
    void addClicked();
    void removeClicked();
    void addDefaultClicked();
    void removeAllClicked();
    void clearHistory();
    void openInExternalToggled(bool on);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void downloadJobFinished(KJob *j);
    void resetDefaultAction();

Q_SIGNALS:
    void clearSearchHistory();

private:
    SearchPlugin *plugin;
    SearchEngineList *engines;
};

}

#endif
