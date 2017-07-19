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

#include <KLocalizedString>
#include <KConfigGroup>

#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "managefiltersdlg.h"
#include "filterlistmodel.h"
#include "filterlist.h"
#include "ktfeed.h"
#include "syndicationactivity.h"

namespace kt
{

    ManageFiltersDlg::ManageFiltersDlg(Feed* feed, FilterList* filters, SyndicationActivity* act, QWidget* parent) : QDialog(parent), feed(feed), filters(filters), act(act)
    {
        setWindowTitle(i18n("Add/Remove Filters"));
        QWidget *mainWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout;
        setLayout(mainLayout);
        mainLayout->addWidget(mainWidget);
        setupUi(mainWidget);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
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
        for (int i = 0; i < nfilters; i++)
        {
            Filter* f = filters->filterByRow(i);
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
        for (int i = 0; i < nfilters; i++)
        {
            Filter* f = active->filterByRow(i);
            if (!f)
                continue;

            feed->addFilter(f);
        }
        QDialog::accept();
    }

    void ManageFiltersDlg::add()
    {
        QModelIndexList idx = m_available_filters->selectionModel()->selectedRows();
        QList<Filter*> to_add;
        foreach (const QModelIndex& i, idx)
        {
            Filter* f = available->filterForIndex(i);
            if (f)
                to_add.append(f);
        }

        foreach (Filter* f, to_add)
        {
            active->addFilter(f);
            available->removeFilter(f);
        }

        m_remove->setEnabled(m_active_filters->selectionModel()->selectedRows().count() > 0);
        m_add->setEnabled(m_available_filters->selectionModel()->selectedRows().count() > 0);
        m_remove_all->setEnabled(active->rowCount(QModelIndex()) > 0);
    }

    void ManageFiltersDlg::remove()
    {
        QModelIndexList idx = m_active_filters->selectionModel()->selectedRows();
        QList<Filter*> to_remove;
        foreach (const QModelIndex& i, idx)
        {
            Filter* f = active->filterForIndex(i);
            if (f)
                to_remove.append(f);
        }

        foreach (Filter* f, to_remove)
        {
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
        QList<Filter*> to_remove;
        for (int i = 0; i < nfilters; i++)
        {
            Filter* f = active->filterByRow(i);
            if (!f)
                continue;

            to_remove.append(f);
        }

        foreach (Filter* f, to_remove)
        {
            available->addFilter(f);
            active->removeFilter(f);
        }

        m_remove_all->setEnabled(false);
    }

    void ManageFiltersDlg::newFilter()
    {
        Filter* f = act->addNewFilter();
        if (f)
        {
            available->addFilter(f);
        }
    }

    void ManageFiltersDlg::activeSelectionChanged(const QItemSelection& sel, const QItemSelection& desel)
    {
        Q_UNUSED(sel);
        Q_UNUSED(desel);
        m_remove->setEnabled(m_active_filters->selectionModel()->selectedRows().count() > 0);
    }

    void ManageFiltersDlg::availableSelectionChanged(const QItemSelection& sel, const QItemSelection& desel)
    {
        Q_UNUSED(sel);
        Q_UNUSED(desel);
        m_add->setEnabled(m_available_filters->selectionModel()->selectedRows().count() > 0);
    }
}
