/***************************************************************************
 *   Copyright (C) 2007 by Ivan Vasić                                     *
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
#include "ipfilterwidget.h"

#include <peer/accessmanager.h>
#include <torrent/globals.h>
#include <util/log.h>
#include <util/error.h>
#include <util/constants.h>
#include <interfaces/functions.h>

#include <QtGui>
#include <QtCore>

#include <KMessageBox>
#include <KFileDialog>
#include <KUrl>

#include "ipfilterlist.h"

#define MAX_RANGES 500

using namespace bt;

namespace kt
{

    IPFilterList* IPFilterWidget::filter_list = 0;


    IPFilterWidget::IPFilterWidget(QWidget* parent)
        : KDialog(parent)
    {
        setupUi(mainWidget());
        setButtons(KDialog::None);
        setCaption(i18n("IP Filter List"));

        m_add->setGuiItem(KStandardGuiItem::add());
        m_clear->setGuiItem(KStandardGuiItem::clear());
        m_save_as->setGuiItem(KStandardGuiItem::saveAs());
        m_open->setGuiItem(KStandardGuiItem::open());
        m_remove->setGuiItem(KStandardGuiItem::remove());
        m_close->setGuiItem(KStandardGuiItem::close());

        registerFilterList();

        m_ip_list->setModel(filter_list);
        m_ip_list->setSelectionMode(QAbstractItemView::ContiguousSelection);

        setupConnections();
    }

    IPFilterWidget::~IPFilterWidget()
    {
    }

    void IPFilterWidget::registerFilterList()
    {
        if (!filter_list)
        {
            filter_list = new IPFilterList();
            AccessManager::instance().addBlockList(filter_list);
            loadFilter(kt::DataDir() + "ip_filter");
        }
    }


    void IPFilterWidget::setupConnections()
    {
        connect(m_add, SIGNAL(clicked()), this, SLOT(add()));
        connect(m_close, SIGNAL(clicked()), this, SLOT(accept()));
        connect(m_clear, SIGNAL(clicked()), this, SLOT(clear()));
        connect(m_save_as, SIGNAL(clicked()), this, SLOT(save()));
        connect(m_open, SIGNAL(clicked()), this, SLOT(open()));
        connect(m_remove, SIGNAL(clicked()), this, SLOT(remove()));
        connect(this, SIGNAL(closeClicked()), this, SLOT(reject()));
    }

    void IPFilterWidget::add()
    {
        int var = 0;

        try
        {
            QRegExp rx("(([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}))"
                       "|(([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3})-"
                       "([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}))");
            QRegExpValidator v(rx, 0);

            QString ip = m_ip_to_add->text();

            if (v.validate(ip, var) != QValidator::Acceptable || !filter_list->add(ip))
            {
                KMessageBox::sorry(this, i18n("Invalid IP address <b>%1</b>. IP addresses must be in the format 'XXX.XXX.XXX.XXX'."
                                              "<br/><br/>You can also use wildcards like '127.0.0.*' or specify ranges like '200.10.10.0-200.10.10.40'.").arg(ip));

                return;
            }
        }
        catch (bt::Error& err)
        {
            KMessageBox::sorry(this, err.toString());
        }
    }

    void IPFilterWidget::remove()
    {
        QModelIndexList idx = m_ip_list->selectionModel()->selectedRows();
        if (idx.count() == 0)
            return;

        filter_list->remove(idx.at(0).row(), idx.count());
    }

    void IPFilterWidget::clear()
    {
        filter_list->clear();
    }

    void IPFilterWidget::open()
    {
        QString lf = KFileDialog::getOpenFileName(KUrl("kfiledialog:///openTorrent"), "*.txt|", this, i18n("Choose a file"));

        if (lf.isEmpty())
            return;

        clear();

        loadFilter(lf);
    }

    void IPFilterWidget::save()
    {
        QString sf = KFileDialog::getSaveFileName(KUrl("kfiledialog:///openTorrent"), "*.txt|", this, i18n("Choose a filename to save under"));

        if (sf.isEmpty())
            return;

        saveFilter(sf);
    }

    void IPFilterWidget::accept()
    {
        saveFilter(kt::DataDir() + "ip_filter");
        KDialog::accept();
    }

    void IPFilterWidget::saveFilter(const QString& fn)
    {
        QFile fptr(fn);

        if (!fptr.open(QIODevice::WriteOnly))
        {
            Out(SYS_GEN | LOG_NOTICE) << QString("Could not open file %1 for writing.").arg(fn) << endl;
            return;
        }

        QTextStream out(&fptr);

        for (int i = 0; i < filter_list->rowCount(); ++i)
        {
            out << filter_list->data(filter_list->index(i, 0), Qt::DisplayRole).toString() << ::endl;
        }

        fptr.close();
    }

    void IPFilterWidget::loadFilter(const QString& fn)
    {
        QFile dat(fn);
        dat.open(QIODevice::ReadOnly);

        QTextStream stream(&dat);
        QString line;
        QRegExp rx("(([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}).([*]|[0-9]{1,3}))"
                   "|(([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3})-"
                   "([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}).([0-9]{1,3}))");
        QRegExpValidator v(rx, 0);

        bool err = false;
        int pos = 0;

        while (!stream.atEnd())
        {
            line = stream.readLine();
            if (v.validate(line, pos) != QValidator::Acceptable)
            {
                err = true;
            }
            else
            {
                try
                {
                    filter_list->add(line);
                }
                catch (...)
                {
                    err = true;
                }
            }
        }

        if (err)
            Out(SYS_IPF | LOG_NOTICE) << "Some lines could not be loaded. Check your filter file..." << endl;

        dat.close();
    }
}
#include "ipfilterwidget.moc"
