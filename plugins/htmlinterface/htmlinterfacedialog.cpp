/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   Copyright (C) 2018 by Emmanuel Eytan                                  *
 *   eje211@gmail.com                                                      *
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


#include "htmlinterfacedialog.h"

#include <KConfig>
#include <KConfigGroup>

#include <QMenu>

#include <interfaces/torrentinterface.h>
#include "htmlinterfaceplugin.h"

namespace kt
{

    HtmlInterfaceDialog::HtmlInterfaceDialog(HtmlInterfacePlugin* plugin, bt::TorrentInterface* tor, QWidget* parent)
        : QDialog(parent), tor(tor), plugin(plugin)
    {
//         setupUi(this);
//         connect(buttonBox, &QDialogButtonBox::accepted, this, &HtmlInterfaceDialog::accept);
//         connect(buttonBox, &QDialogButtonBox::rejected, this, &HtmlInterfaceDialog::reject);
//         connect(this, &HtmlInterfaceDialog::accepted, this, &HtmlInterfaceDialog::commitHtmlInterface);
//         setWindowTitle(i18n("File Download Order"));
//         m_top_label->setText(i18n("File download order for <b>%1</b>:", tor->getDisplayName()));
// 
//         HtmlInterfaceManager* dom = plugin->manager(tor);
//         m_custom_order_enabled->setChecked(dom != 0);
//         m_order->setEnabled(dom != 0);
//         m_move_up->setEnabled(false);
//         m_move_down->setEnabled(false);
//         m_move_top->setEnabled(false);
//         m_move_bottom->setEnabled(false);
//         m_search_files->setEnabled(false);
// 
//         m_move_up->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
//         connect(m_move_up, &QPushButton::clicked, this, &HtmlInterfaceDialog::moveUp);
//         m_move_down->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
//         connect(m_move_down, &QPushButton::clicked, this, &HtmlInterfaceDialog::moveDown);
//         m_move_top->setIcon(QIcon::fromTheme(QStringLiteral("go-top")));
//         connect(m_move_top, &QPushButton::clicked, this, &HtmlInterfaceDialog::moveTop);
//         m_move_bottom->setIcon(QIcon::fromTheme(QStringLiteral("go-bottom")));
//         connect(m_move_bottom, &QPushButton::clicked, this, &HtmlInterfaceDialog::moveBottom);
// 
//         m_order->setSelectionMode(QAbstractItemView::ContiguousSelection);
//         m_order->setDragEnabled(true);
//         m_order->setAcceptDrops(true);
//         m_order->setDropIndicatorShown(true);
//         m_order->setDragDropMode(QAbstractItemView::InternalMove);
// 
//         model = new HtmlInterfaceModel(tor, this);
//         if (dom)
//             model->initOrder(dom->downloadOrder());
//         m_order->setModel(model);
// 
//         QSize s = KSharedConfig::openConfig()->group("HtmlInterfaceDialog").readEntry("size", size());
//         resize(s);
// 
//         connect(m_order->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
//                 this, SLOT(itemSelectionChanged(QItemSelection, QItemSelection)));
//         connect(m_custom_order_enabled, &QCheckBox::toggled, this, &HtmlInterfaceDialog::customOrderEnableToggled);
//         connect(m_search_files, &QLineEdit::textChanged, this, &HtmlInterfaceDialog::search);
// 
//         QMenu* sort_by_menu = new QMenu(m_sort_by);
//         sort_by_menu->addAction(i18n("Name"), model, SLOT(sortByName()));
//         sort_by_menu->addAction(i18n("Seasons and Episodes"), model, SLOT(sortBySeasonsAndEpisodes()));
//         sort_by_menu->addAction(i18n("Album Track Order"), model, SLOT(sortByAlbumTrackOrder()));
//         m_sort_by->setMenu(sort_by_menu);
//         m_sort_by->setPopupMode(QToolButton::InstantPopup);
//         m_sort_by->setEnabled(false);
    }


    HtmlInterfaceDialog::~HtmlInterfaceDialog()
    {
        KSharedConfig::openConfig()->group("HtmlInterfaceDialog").writeEntry("size", size());
    }

//     void HtmlInterfaceDialog::commitHtmlInterface()
//     {
//         if (m_custom_order_enabled->isChecked())
//         {
//             HtmlInterfaceManager* dom = plugin->manager(tor);
//             if (!dom)
//             {
//                 dom = plugin->createManager(tor);
//                 connect(tor, &bt::TorrentInterface::chunkDownloaded, dom, &HtmlInterfaceManager::chunkDownloaded);
//             }
// 
//             dom->setHtmlInterface(model->downloadOrder());
//             dom->save();
//             dom->update();
//         }
//         else
//         {
//             HtmlInterfaceManager* dom = plugin->manager(tor);
//             if (dom)
//             {
//                 dom->disable();
//                 plugin->destroyManager(tor);
//             }
//         }
//     }
// 
//     void HtmlInterfaceDialog::moveUp()
//     {
//         QModelIndexList idx = m_order->selectionModel()->selectedRows();
//         model->moveUp(idx.front().row(), idx.count());
//         if (idx.front().row() > 0)
//         {
//             QItemSelection sel(model->index(idx.first().row() - 1), model->index(idx.last().row() - 1));
//             m_order->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
//         }
//     }
// 
//     void HtmlInterfaceDialog::moveTop()
//     {
//         QModelIndexList idx = m_order->selectionModel()->selectedRows();
//         model->moveTop(idx.front().row(), idx.count());
//         if (idx.front().row() > 0)
//         {
//             QItemSelection sel(model->index(0), model->index(idx.count() - 1));
//             m_order->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
//         }
//     }
// 
//     void HtmlInterfaceDialog::moveDown()
//     {
//         QModelIndexList idx = m_order->selectionModel()->selectedRows();
//         model->moveDown(idx.front().row(), idx.count());
//         if (idx.back().row() < (int)tor->getNumFiles() - 1)
//         {
//             QItemSelection sel(model->index(idx.first().row() + 1), model->index(idx.last().row() + 1));
//             m_order->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
//         }
//     }
// 
//     void HtmlInterfaceDialog::moveBottom()
//     {
//         QModelIndexList idx = m_order->selectionModel()->selectedRows();
//         model->moveBottom(idx.front().row(), idx.count());
//         if (idx.back().row() < (int)tor->getNumFiles() - 1)
//         {
//             QItemSelection sel(model->index(tor->getNumFiles() - idx.size()), model->index(tor->getNumFiles() - 1));
//             m_order->selectionModel()->select(sel, QItemSelectionModel::ClearAndSelect);
//         }
//     }
// 
//     void HtmlInterfaceDialog::itemSelectionChanged(const QItemSelection& new_sel, const QItemSelection& old_sel)
//     {
//         Q_UNUSED(old_sel);
//         if (new_sel.empty())
//         {
//             m_move_down->setEnabled(false);
//             m_move_up->setEnabled(false);
//             m_move_top->setEnabled(false);
//             m_move_down->setEnabled(false);
//         }
//         else
//         {
//             bool up_ok = new_sel.front().topLeft().row() > 0;
//             bool down_ok = new_sel.back().bottomRight().row() != (int)tor->getNumFiles() - 1;
//             m_move_up->setEnabled(up_ok);
//             m_move_top->setEnabled(up_ok);
//             m_move_down->setEnabled(down_ok);
//             m_move_bottom->setEnabled(down_ok);
//         }
//     }
// 
//     void HtmlInterfaceDialog::customOrderEnableToggled(bool on)
//     {
//         m_search_files->setEnabled(on);
//         m_sort_by->setEnabled(on);
//         if (!on)
//         {
//             m_move_down->setEnabled(false);
//             m_move_up->setEnabled(false);
//             m_move_top->setEnabled(false);
//             m_move_down->setEnabled(false);
//         }
//         else
//         {
//             itemSelectionChanged(m_order->selectionModel()->selection(), QItemSelection());
//         }
//     }
// 
//     void HtmlInterfaceDialog::search(const QString& text)
//     {
//         if (text.isEmpty())
//         {
//             model->clearHighLights();
//         }
//         else
//         {
//             QModelIndex idx = model->find(text);
//             if (idx.isValid())
//                 m_order->scrollTo(idx);
//         }
//     }
}
