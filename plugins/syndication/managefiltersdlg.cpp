/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfigGroup>
#include <KLocalizedString>

#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "filterlist.h"
#include "filterlistmodel.h"
#include "ktfeed.h"
#include "managefiltersdlg.h"
#include "syndicationactivity.h"

namespace kt
{
ManageFiltersDlg::ManageFiltersDlg(Feed *feed, FilterList *filters, SyndicationActivity *act, QWidget *parent)
    : QDialog(parent)
    , feed(feed)
    , filters(filters)
    , act(act)
{
    setWindowTitle(i18n("Add/Remove Filters"));
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    setupUi(mainWidget);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    auto okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ManageFiltersDlg::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ManageFiltersDlg::reject);
    mainLayout->addWidget(buttonBox);
    m_feed_text->setText(i18n("Feed: <b>%1</b>", feed->title()));
    m_add->setIcon(QIcon::fromTheme(QStringLiteral("go-previous")));
    m_add->setText(QString());
    m_remove->setIcon(QIcon::fromTheme(QStringLiteral("go-next")));
    m_remove->setText(QString());
    connect(m_add, &QPushButton::clicked, this, &ManageFiltersDlg::add);
    connect(m_remove, &QPushButton::clicked, this, &ManageFiltersDlg::remove);
    connect(m_remove_all, &QPushButton::clicked, this, &ManageFiltersDlg::removeAll);
    connect(m_new_filter, &QPushButton::clicked, this, &ManageFiltersDlg::newFilter);

    active = new FilterListModel(this);
    available = new FilterListModel(this);
    m_active_filters->setModel(active);
    m_available_filters->setModel(available);

    int nfilters = filters->rowCount(QModelIndex());
    for (int i = 0; i < nfilters; i++) {
        Filter *f = filters->filterByRow(i);
        if (!f)
            continue;

        if (feed->usingFilter(f))
            active->addFilter(f);
        else
            available->addFilter(f);
    }

    m_add->setEnabled(false);
    connect(m_available_filters->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ManageFiltersDlg::availableSelectionChanged);
    m_remove->setEnabled(false);
    connect(m_active_filters->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ManageFiltersDlg::activeSelectionChanged);

    m_remove_all->setEnabled(active->rowCount(QModelIndex()) > 0);
}

ManageFiltersDlg::~ManageFiltersDlg()
{
}

void ManageFiltersDlg::accept()
{
    feed->clearFilters();
    int nfilters = active->rowCount(QModelIndex());
    for (int i = 0; i < nfilters; i++) {
        Filter *f = active->filterByRow(i);
        if (!f)
            continue;

        feed->addFilter(f);
    }
    QDialog::accept();
}

void ManageFiltersDlg::add()
{
    const QModelIndexList idx = m_available_filters->selectionModel()->selectedRows();
    QList<Filter *> to_add;
    for (const QModelIndex &i : idx) {
        Filter *f = available->filterForIndex(i);
        if (f)
            to_add.append(f);
    }

    for (Filter *f : qAsConst(to_add)) {
        active->addFilter(f);
        available->removeFilter(f);
    }

    m_remove->setEnabled(m_active_filters->selectionModel()->selectedRows().count() > 0);
    m_add->setEnabled(m_available_filters->selectionModel()->selectedRows().count() > 0);
    m_remove_all->setEnabled(active->rowCount(QModelIndex()) > 0);
}

void ManageFiltersDlg::remove()
{
    const QModelIndexList idx = m_active_filters->selectionModel()->selectedRows();
    QList<Filter *> to_remove;
    for (const QModelIndex &i : idx) {
        Filter *f = active->filterForIndex(i);
        if (f)
            to_remove.append(f);
    }

    for (Filter *f : qAsConst(to_remove)) {
        available->addFilter(f);
        active->removeFilter(f);
    }

    m_remove->setEnabled(m_active_filters->selectionModel()->selectedRows().count() > 0);
    m_add->setEnabled(m_available_filters->selectionModel()->selectedRows().count() > 0);
    m_remove_all->setEnabled(active->rowCount(QModelIndex()) > 0);
}

void ManageFiltersDlg::removeAll()
{
    int nfilters = active->rowCount(QModelIndex());
    QList<Filter *> to_remove;
    for (int i = 0; i < nfilters; i++) {
        Filter *f = active->filterByRow(i);
        if (!f)
            continue;

        to_remove.append(f);
    }

    for (Filter *f : qAsConst(to_remove)) {
        available->addFilter(f);
        active->removeFilter(f);
    }

    m_remove_all->setEnabled(false);
}

void ManageFiltersDlg::newFilter()
{
    Filter *f = act->addNewFilter();
    if (f) {
        available->addFilter(f);
    }
}

void ManageFiltersDlg::activeSelectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    Q_UNUSED(sel);
    Q_UNUSED(desel);
    m_remove->setEnabled(m_active_filters->selectionModel()->selectedRows().count() > 0);
}

void ManageFiltersDlg::availableSelectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    Q_UNUSED(sel);
    Q_UNUSED(desel);
    m_add->setEnabled(m_available_filters->selectionModel()->selectedRows().count() > 0);
}
}
