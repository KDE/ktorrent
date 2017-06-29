/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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

#include "pastedialog.h"
#include "core.h"

#include <QApplication>
#include <QClipboard>
#include <QLineEdit>
#include <QUrl>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KStandardGuiItem>

#include <groups/groupmanager.h>

namespace kt
{
    PasteDialog::PasteDialog(Core* core, QWidget* parent, Qt::WindowFlags fl)
        : QDialog(parent, fl)
    {
        setupUi(this);
        setWindowTitle(i18n("Open an URL"));

        m_core = core;
        QClipboard* cb = QApplication::clipboard();
        QString text = cb->text(QClipboard::Clipboard);

        QUrl url = QUrl(text);

        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        if (url.isValid())
            m_url->setText(text);

        loadGroups();
    }

    PasteDialog::~PasteDialog()
    {
    }

    void PasteDialog::loadGroups()
    {
        GroupManager* gman = m_core->getGroupManager();
        GroupManager::Itr it = gman->begin();
        QStringList grps;
        //First default group
        grps << i18n("All Torrents");

        //now custom ones
        while (it != gman->end())
        {
            if (!it->second->isStandardGroup())
                grps << it->first;
            ++it;
        }

        m_groups->addItems(grps);
    }

    void PasteDialog::loadState(KSharedConfig::Ptr cfg)
    {
        KConfigGroup g = cfg->group("PasteDlg");
        m_silently->setChecked(g.readEntry("silently", false));
        m_groups->setCurrentIndex(g.readEntry("group", 0));
    }

    void PasteDialog::saveState(KSharedConfig::Ptr cfg)
    {
        KConfigGroup g = cfg->group("PasteDlg");
        g.writeEntry("silently", m_silently->isChecked());
        g.writeEntry("group", m_groups->currentIndex());
    }

    void PasteDialog::accept()
    {
        QUrl url = QUrl(m_url->text());
        if (url.isValid())
        {
            QString group;
            if (m_groups->currentIndex() > 0)
                group = m_groups->currentText();

            if (m_silently->isChecked())
                m_core->loadSilently(url, group);
            else
                m_core->load(url, group);
            QDialog::accept();
        }
        else
        {
            KMessageBox::error(this, i18n("Invalid URL: %1", m_url->text()));
        }
    }
}

