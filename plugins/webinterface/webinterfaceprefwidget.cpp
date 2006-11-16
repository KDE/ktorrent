  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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

#include "webinterfaceprefwidget.h"
#include "webinterfacepluginsettings.h"

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kpassdlg.h>
#include <kmdcodec.h>

namespace kt
{

WebInterfacePrefWidget::WebInterfacePrefWidget(QWidget *parent, const char *name):WebInterfacePreference(parent,name)
{
	port->setValue(WebInterfacePluginSettings::port());
	forward->setChecked(WebInterfacePluginSettings::forward());
	sessionTTL->setValue(WebInterfacePluginSettings::sessionTTL());
	phpExecutablePath->setURL (WebInterfacePluginSettings::phpExecutablePath());
	rootDir->setMode(KFile::Directory);
	rootDir->setURL (WebInterfacePluginSettings::rootDir());
	username->setText(WebInterfacePluginSettings::username());
	password->setText("_fakepass_");
}

bool WebInterfacePrefWidget::apply()
{
	WebInterfacePluginSettings::setPort(port->value () );
	WebInterfacePluginSettings::setForward(forward->isChecked());
	WebInterfacePluginSettings::setSessionTTL(sessionTTL->value () );
	WebInterfacePluginSettings::setPhpExecutablePath(phpExecutablePath->url () );
	WebInterfacePluginSettings::setRootDir(rootDir->url () );
	if(!username->text().isEmpty() && !QString(password->password()).isEmpty() && QString(password->password())!="_fakepass_"){
		WebInterfacePluginSettings::setUsername(username->text() );
		KMD5 context(password->password());
		WebInterfacePluginSettings::setPassword(context.hexDigest().data());
	}

	WebInterfacePluginSettings::writeConfig();
	return true;
}
}
#include "webinterfaceprefwidget.moc"
