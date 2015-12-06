/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <QAction>
#include <QVBoxLayout>
#include <util/log.h>
#include <KRun>
#include <klocalizedstring.h>
#include <QMenu>
#include <QAction>
#include <KActionCollection>
#include <kross/core/manager.h>
#include "scriptmanager.h"
#include "scriptmodel.h"
#include "script.h"
#include "scriptdelegate.h"
#include "ui_scriptproperties.h"

using namespace Kross;
using namespace bt;

namespace kt
{

    ScriptManager::ScriptManager(ScriptModel* model, QWidget* parent)
        : Activity(i18n("Scripts"), "text-x-script", 40, parent), model(model)
    {
        setXMLGUIFile("ktscriptingpluginui.rc");
        setupActions();
        setToolTip(i18n("Widget to start, stop and manage scripts"));
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setMargin(0);

        view = new QListView(this);
        view->setItemDelegate(new ScriptDelegate(view));
        view->setAlternatingRowColors(true);
        layout->addWidget(view);

        view->setModel(model);
        view->setContextMenuPolicy(Qt::CustomContextMenu);
        view->setSelectionMode(QAbstractItemView::ExtendedSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(view->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection)),
                this, SLOT(onSelectionChanged(const QItemSelection&, const QItemSelection)));

        connect(view, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenu(const QPoint&)));

        connect(model, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

        add_script->setEnabled(true);
        remove_script->setEnabled(false);
        run_script->setEnabled(false);
        stop_script->setEnabled(false);
        edit_script->setEnabled(false);
        properties->setEnabled(false);
        configure_script->setEnabled(false);
    }


    ScriptManager::~ScriptManager()
    {
    }


    void ScriptManager::setupActions()
    {
        KActionCollection* ac = part()->actionCollection();

        add_script = new QAction(QIcon::fromTheme("list-add"), i18n("Add Script"), this);
        connect(add_script, SIGNAL(triggered()), this, SIGNAL(addScript()));
        ac->addAction("add_script", add_script);

        remove_script = new QAction(QIcon::fromTheme("list-remove"), i18n("Remove Script"), this);
        connect(remove_script, SIGNAL(triggered()), this, SIGNAL(removeScript()));
        ac->addAction("remove_script", remove_script);

        run_script = new QAction(QIcon::fromTheme("system-run"), i18n("Run Script"), this);
        connect(run_script, SIGNAL(triggered()), this, SLOT(runScript()));
        ac->addAction("run_script", run_script);

        stop_script = new QAction(QIcon::fromTheme("media-playback-stop"), i18n("Stop Script"), this);
        connect(stop_script, SIGNAL(triggered()), this, SLOT(stopScript()));
        ac->addAction("stop_script", stop_script);

        edit_script = new QAction(QIcon::fromTheme("document-open"), i18n("Edit Script"), this);
        connect(edit_script, SIGNAL(triggered()), this, SLOT(editScript()));
        ac->addAction("edit_script", edit_script);

        properties = new QAction(QIcon::fromTheme("dialog-information"), i18n("Properties"), this);
        connect(properties, SIGNAL(triggered()), this, SLOT(showProperties()));
        ac->addAction("script_properties", properties);

        configure_script = new QAction(QIcon::fromTheme("preferences-other"), i18n("Configure"), this);
        connect(configure_script, SIGNAL(triggered()), this, SLOT(configureScript()));
        ac->addAction("configure_script", configure_script);
    }

    void ScriptManager::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
    {
        Q_UNUSED(deselected);
        Q_UNUSED(selected);
        updateActions(selectedScripts());
    }

    void ScriptManager::updateActions(const QModelIndexList& selected)
    {
        int num_removeable = 0;
        int num_running = 0;
        int num_not_running = 0;
        foreach (const QModelIndex& idx, selected)
        {
            Script* s = model->scriptForIndex(idx);
            if (s)
            {
                if (s->running())
                    num_running++;
                else
                    num_not_running++;
                if (s->removeable())
                    num_removeable++;
            }
            else
                num_not_running++;
        }

        remove_script->setEnabled(num_removeable > 0);
        run_script->setEnabled(selected.count() > 0 && num_not_running > 0);
        stop_script->setEnabled(selected.count() > 0 && num_running > 0);
        Script* s = 0;
        if (selected.count() > 0)
            s = model->scriptForIndex(selected.front());
        properties->setEnabled(selected.count() == 1 && s && s->metaInfo().valid());
        configure_script->setEnabled(selected.count() == 1 && s && s->hasConfigure());
        edit_script->setEnabled(selected.count() == 1);
    }

    QModelIndexList ScriptManager::selectedScripts()
    {
        return view->selectionModel()->selectedRows();
    }

    void ScriptManager::showContextMenu(const QPoint& p)
    {
        QMenu* m = part()->menu("ScriptingMenu");
        if (m)
            m->popup(view->viewport()->mapToGlobal(p));
    }

    void ScriptManager::dataChanged(const QModelIndex& from, const QModelIndex& to)
    {
        Q_UNUSED(from);
        Q_UNUSED(to);
        updateActions(selectedScripts());
    }

    void ScriptManager::runScript()
    {
        QModelIndexList sel = selectedScripts();
        foreach (const QModelIndex& idx, sel)
        {
            if (!model->setData(idx, Qt::Checked, Qt::CheckStateRole))
                Out(SYS_SCR | LOG_DEBUG) << "setData failed" << endl;
        }
        updateActions(sel);
    }

    void ScriptManager::stopScript()
    {
        QModelIndexList sel = selectedScripts();
        foreach (const QModelIndex& idx, sel)
        {
            if (!model->setData(idx, Qt::Unchecked, Qt::CheckStateRole))
                Out(SYS_SCR | LOG_DEBUG) << "setData failed" << endl;
        }
        updateActions(sel);
    }

    void ScriptManager::editScript()
    {
        QModelIndexList sel = selectedScripts();
        foreach (const QModelIndex& idx, sel)
        {
            Script* s = model->scriptForIndex(idx);
            if (s)
                new KRun(KUrl(s->scriptFile()), 0, 0, true, true);
        }

    }

    void ScriptManager::showProperties()
    {
        QModelIndexList sel = selectedScripts();
        if (sel.count() != 1)
            return;

        Script* s = model->scriptForIndex(sel.front());
        if (!s || !s->metaInfo().valid())
            return;

        showProperties(s);
    }

    void ScriptManager::showProperties(kt::Script* s)
    {
        Ui_ScriptProperties prop;
        KDialog* dialog = new KDialog(this);
        dialog->setButtons(KDialog::Ok);
        dialog->setWindowTitle(i18n("Script Properties"));
        prop.setupUi(dialog->mainWidget());
        prop.m_icon->setPixmap(DesktopIcon(s->iconName()));
        prop.m_name->setText(s->name());
        prop.m_description->setText(s->metaInfo().comment);
        prop.m_author->setText(s->metaInfo().author);
        prop.m_license->setText(s->metaInfo().license);
        prop.m_email->setText(s->metaInfo().email);
        prop.m_website->setText(s->metaInfo().website);
        dialog->exec();
        delete dialog;
    }


    void ScriptManager::configureScript()
    {
        QModelIndexList sel = selectedScripts();
        if (sel.count() != 1)
            return;

        Script* s = model->scriptForIndex(sel.front());
        if (!s || !s->metaInfo().valid() || !s->hasConfigure())
            return;

        s->configure();
    }
}
