/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QMenu>
#include <QPushButton>
#include <QVBoxLayout>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <KRun>
#include <Kross/Core/Manager>

#include "script.h"
#include "scriptdelegate.h"
#include "scriptmanager.h"
#include "scriptmodel.h"
#include "ui_scriptproperties.h"
#include <util/log.h>

using namespace Kross;
using namespace bt;

namespace kt
{
ScriptManager::ScriptManager(ScriptModel *model, QWidget *parent)
    : Activity(i18n("Scripts"), QStringLiteral("text-x-script"), 40, parent)
    , model(model)
{
    setXMLGUIFile(QStringLiteral("ktorrent_scriptingui.rc"));
    setupActions();
    setToolTip(i18n("Widget to start, stop and manage scripts"));
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);

    view = new QListView(this);
    delegate = new ScriptDelegate(view);
    view->setItemDelegate(delegate);
    view->setAlternatingRowColors(true);
    layout->addWidget(view);

    view->setModel(model);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ScriptManager::onSelectionChanged);

    connect(view, &QListView::customContextMenuRequested, this, &ScriptManager::showContextMenu);

    connect(model, &ScriptModel::dataChanged, this, &ScriptManager::dataChanged);

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
    delete delegate;
}

void ScriptManager::setupActions()
{
    KActionCollection *ac = part()->actionCollection();

    add_script = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add Script"), this);
    connect(add_script, &QAction::triggered, this, &ScriptManager::addScript);
    ac->addAction(QStringLiteral("add_script"), add_script);

    remove_script = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")), i18n("Remove Script"), this);
    connect(remove_script, &QAction::triggered, this, &ScriptManager::removeScript);
    ac->addAction(QStringLiteral("remove_script"), remove_script);

    run_script = new QAction(QIcon::fromTheme(QStringLiteral("system-run")), i18n("Run Script"), this);
    connect(run_script, &QAction::triggered, this, &ScriptManager::runScript);
    ac->addAction(QStringLiteral("run_script"), run_script);

    stop_script = new QAction(QIcon::fromTheme(QStringLiteral("media-playback-stop")), i18n("Stop Script"), this);
    connect(stop_script, &QAction::triggered, this, &ScriptManager::stopScript);
    ac->addAction(QStringLiteral("stop_script"), stop_script);

    edit_script = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), i18n("Edit Script"), this);
    connect(edit_script, &QAction::triggered, this, &ScriptManager::editScript);
    ac->addAction(QStringLiteral("edit_script"), edit_script);

    properties = new QAction(QIcon::fromTheme(QStringLiteral("dialog-information")), i18n("Properties"), this);
    connect(properties, &QAction::triggered, this, qOverload<>(&ScriptManager::showProperties));
    ac->addAction(QStringLiteral("script_properties"), properties);

    configure_script = new QAction(QIcon::fromTheme(QStringLiteral("preferences-other")), i18n("Configure"), this);
    connect(configure_script, &QAction::triggered, this, &ScriptManager::configureScript);
    ac->addAction(QStringLiteral("configure_script"), configure_script);
}

void ScriptManager::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected);
    Q_UNUSED(selected);
    updateActions(selectedScripts());
}

void ScriptManager::updateActions(const QModelIndexList &selected)
{
    int num_removable = 0;
    int num_running = 0;
    int num_not_running = 0;
    for (const QModelIndex &idx : selected) {
        Script *s = model->scriptForIndex(idx);
        if (s) {
            if (s->running())
                num_running++;
            else
                num_not_running++;
            if (s->removable())
                num_removable++;
        } else
            num_not_running++;
    }

    remove_script->setEnabled(num_removable > 0);
    run_script->setEnabled(selected.count() > 0 && num_not_running > 0);
    stop_script->setEnabled(selected.count() > 0 && num_running > 0);
    Script *s = nullptr;
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

void ScriptManager::showContextMenu(const QPoint &p)
{
    QMenu *m = part()->menu(QStringLiteral("ScriptingMenu"));
    if (m)
        m->popup(view->viewport()->mapToGlobal(p));
}

void ScriptManager::dataChanged(const QModelIndex &from, const QModelIndex &to)
{
    Q_UNUSED(from);
    Q_UNUSED(to);
    updateActions(selectedScripts());
}

void ScriptManager::runScript()
{
    const QModelIndexList sel = selectedScripts();
    for (const QModelIndex &idx : sel) {
        if (!model->setData(idx, Qt::Checked, Qt::CheckStateRole))
            Out(SYS_SCR | LOG_DEBUG) << "setData failed" << endl;
    }
    updateActions(sel);
}

void ScriptManager::stopScript()
{
    const QModelIndexList sel = selectedScripts();
    for (const QModelIndex &idx : sel) {
        if (!model->setData(idx, Qt::Unchecked, Qt::CheckStateRole))
            Out(SYS_SCR | LOG_DEBUG) << "setData failed" << endl;
    }
    updateActions(sel);
}

void ScriptManager::editScript()
{
    const QModelIndexList sel = selectedScripts();
    for (const QModelIndex &idx : sel) {
        Script *s = model->scriptForIndex(idx);
        if (s)
            new KRun(QUrl::fromLocalFile(s->scriptFile()), nullptr);
    }
}

void ScriptManager::showProperties()
{
    QModelIndexList sel = selectedScripts();
    if (sel.count() != 1)
        return;

    Script *s = model->scriptForIndex(sel.front());
    if (!s || !s->metaInfo().valid())
        return;

    showProperties(s);
}

void ScriptManager::showProperties(kt::Script *s)
{
    Ui_ScriptProperties prop;
    QDialog *dialog = new QDialog(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    dialog->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    dialog->connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
    mainLayout->addWidget(buttonBox);
    dialog->setWindowTitle(i18n("Script Properties"));
    prop.setupUi(mainWidget);
    prop.m_icon->setPixmap(QIcon::fromTheme(s->iconName()).pixmap(KIconLoader::StdSizes::SizeMedium));
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

    Script *s = model->scriptForIndex(sel.front());
    if (!s || !s->metaInfo().valid() || !s->hasConfigure())
        return;

    s->configure();
}
}
