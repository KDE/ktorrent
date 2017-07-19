/***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                                     *
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
#include <KGlobal>
#include <KIconLoader>
#include <KStandardDirs>

#include <net/portlist.h>
#include <torrent/globals.h>

#include "webinterfaceprefwidget.h"
#include "webinterfacepluginsettings.h"


using namespace bt;

namespace kt
{

    WebInterfacePrefWidget::WebInterfacePrefWidget(QWidget* parent)
        : PrefPageInterface(WebInterfacePluginSettings::self(), i18n("Web Interface"), "network-server", parent)
    {
        setupUi(this);
        connect(kcfg_authentication, &QCheckBox::toggled, this, &WebInterfacePrefWidget::authenticationToggled);

        QStringList dirList = KGlobal::dirs()->findDirs("data", "ktorrent/www");
        if (!dirList.isEmpty())
        {
            QDir d(*(dirList.begin()));

            QStringList skinList = d.entryList(QDir::Dirs);
            foreach (const QString& skin, skinList)
            {
                if (skin == "." || skin == ".." || skin == "common")
                    continue;
                kcfg_skin->addItem(skin);
            }
        }
        kcfg_username->setEnabled(WebInterfacePluginSettings::authentication());
        kcfg_password->setEnabled(WebInterfacePluginSettings::authentication());
    }

    WebInterfacePrefWidget::~WebInterfacePrefWidget()
    {}

    void WebInterfacePrefWidget::authenticationToggled(bool on)
    {
        kcfg_username->setEnabled(on);
        kcfg_password->setEnabled(on);
    }


}

#include "webinterfaceprefwidget.moc"

