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
#include <Qt>

#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kstandarddirs.h>

#include <net/portlist.h>
#include <torrent/globals.h>

#include "webinterfaceprefwidget.h"
#include "webinterfacepluginsettings.h"


using namespace bt;

namespace kt
{

	WebInterfacePrefWidget::WebInterfacePrefWidget(QWidget *parent) 
		: PrefPageInterface(WebInterfacePluginSettings::self(),i18n("Web Interface"),"network-server",parent)
	{
		setupUi(this);
	
		QStringList dirList =KGlobal::dirs()->findDirs("data", "ktorrent/www");
		QDir d(*(dirList.begin()));
		
		QStringList skinList = d.entryList(QDir::Dirs);
		foreach (QString skin,skinList)
		{
			if (skin =="." || skin == "..")
				continue;
			kcfg_skin->addItem(skin);
		}

		if (WebInterfacePluginSettings::phpExecutablePath().isEmpty())
		{
			QString phpPath = KStandardDirs::findExe("php");
			if(phpPath==QString::null)
				phpPath = KStandardDirs::findExe("php-cli");

			kcfg_phpExecutablePath->setUrl(phpPath);
		}
		
		connect(kcfg_phpExecutablePath,SIGNAL(textChanged(const QString &)),this,SLOT(textChanged(const QString &)));
	}
	
	WebInterfacePrefWidget::~WebInterfacePrefWidget()
	{}

	/*
	bool WebInterfacePrefWidget::apply()
	{
		if(WebInterfacePluginSettings::port()==kcfg_port->value()){
			if(forward->isChecked())
				bt::Globals::instance().getPortList().addNewPort(kcfg_port->value(),net::TCP,true);
			else
				bt::Globals::instance().getPortList().removePort(kcfg_port->value(),net::TCP);
		}		
		WebInterfacePluginSettings::setPort(kcfg_port->value () );
		WebInterfacePluginSettings::setForward(forward->isChecked());
		WebInterfacePluginSettings::setSessionTTL(sessionTTL->value () );
		WebInterfacePluginSettings::setSkin(interfaceSkinBox->currentText());
		WebInterfacePluginSettings::setPhpExecutablePath(phpExecutablePath->url () );
		if(!username->text().isEmpty() && !password.isEmpty()){
			WebInterfacePluginSettings::setUsername(username->text() );
			KMD5 context(password);
			WebInterfacePluginSettings::setPassword(context.hexDigest().data());
		}

		WebInterfacePluginSettings::writeConfig();
		return true;
	}
*/
	
	
	void WebInterfacePrefWidget::textChanged(const QString & path)
	{
		QFileInfo fi(path);
		if(fi.isExecutable() && (fi.isFile() || fi.isSymLink()))
		{
			kled->setToolTip(i18n("%1 exists and it is executable",path));
			kled->setColor(Qt::green);
		}
		else if (!fi.exists())
		{
			kled->setToolTip(i18n("%1 does not exist",path));
			kled->setColor(Qt::red);
		}
		else if (!fi.isExecutable())
		{
			kled->setToolTip(i18n("%1 is not executable",path));
			kled->setColor(Qt::red);
		}
		else if (fi.isDir())
		{
			kled->setToolTip(i18n("%1 is a directory",path));
			kled->setColor(Qt::red);
		}
		else
		{
			kled->setToolTip(i18n("%1 is not php executable path",path));
			kled->setColor(Qt::red);
		}
	}
}

#include "webinterfaceprefwidget.moc"
			 
