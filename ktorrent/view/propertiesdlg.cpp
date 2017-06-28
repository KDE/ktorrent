/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#include "propertiesdlg.h"
#include <QUrl>
#include <interfaces/torrentinterface.h>
#include <settings.h>

namespace kt
{
    PropertiesDlg::PropertiesDlg(bt::TorrentInterface* tc, QWidget* parent)
        : QDialog(parent),
          tc(tc)
    {
        setupUi(this);
        setWindowTitle(i18n("Torrent Settings"));

        connect(m_buttonBox, &QDialogButtonBox::accepted, this, &PropertiesDlg::accept);
        connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

        QString folder = tc->getMoveWhenCompletedDir();
        if (QFile::exists(folder))
        {
            move_on_completion_enabled->setChecked(true);
            move_on_completion_url->setUrl(QUrl::fromLocalFile(folder));
            move_on_completion_url->setEnabled(true);
        }
        else
        {
            move_on_completion_enabled->setChecked(false);
            move_on_completion_url->setEnabled(false);
        }

        // disable DHT and PEX if they are globally disabled
        const bt::TorrentStats& s = tc->getStats();
        dht->setEnabled(!s.priv_torrent);
        pex->setEnabled(!s.priv_torrent);
        dht->setChecked(!s.priv_torrent && tc->isFeatureEnabled(bt::DHT_FEATURE));
        pex->setChecked(!s.priv_torrent && tc->isFeatureEnabled(bt::UT_PEX_FEATURE));

        superseeding->setChecked(s.superseeding);
        connect(move_on_completion_enabled, &QCheckBox::toggled, this, &PropertiesDlg::moveOnCompletionEnabled);
    }

    PropertiesDlg::~PropertiesDlg()
    {
    }

    void PropertiesDlg::moveOnCompletionEnabled(bool on)
    {
        move_on_completion_url->setEnabled(on);
    }

    void PropertiesDlg::accept()
    {
        if (move_on_completion_enabled->isChecked())
        {
            tc->setMoveWhenCompletedDir(move_on_completion_url->url().toLocalFile());
        }
        else
        {
            tc->setMoveWhenCompletedDir(QString());
        }

        tc->setFeatureEnabled(bt::DHT_FEATURE, dht->isChecked());
        tc->setFeatureEnabled(bt::UT_PEX_FEATURE, pex->isChecked());
        tc->setSuperSeeding(superseeding->isChecked());
        QDialog::accept();
    }


}

