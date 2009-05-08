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
#include <QDBusConnection>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include "dbusgroup.h"

namespace kt
{
	static QString ValidDBusName(const QString & s,int extra_digit)
	{
		if (s.isEmpty())
			return s;
		
		QString ret = s;
		if (ret[0].isDigit())
			ret[0] = '_';
		
		for (int i = 0;i < s.length();i++)
		{
			QChar c = ret[i];
			if (!c.isLetterOrNumber() && c != '_')
				ret[i] = '_';
		}
		
		if (extra_digit > 0)
			return ret + QString::number(extra_digit);
		else
			return ret;
	}

	DBusGroup::DBusGroup(Group* g,GroupManager* gman,QObject* parent)
			: QObject(parent),group(g),gman(gman)
	{
		QString name = ValidDBusName(g->groupName(),0);
		if (name != g->groupName())
		{
			// check for dupes if the name has been changed
			int i = 2;
			while (gman->find(name) != 0)
			{
				name = ValidDBusName(g->groupName(),i);
				i++;
			}
		}
		QString path = "/group/" + name;
		QDBusConnection::sessionBus().registerObject(path, this,
									QDBusConnection::ExportScriptableSlots|QDBusConnection::ExportScriptableSignals);
	}


	DBusGroup::~DBusGroup()
	{
	}

	QString DBusGroup::name() const
	{
		return group->groupName();
	}
	
	QString DBusGroup::icon() const
	{
		return group->groupIconName();
	}
	
	QString DBusGroup::defaultSaveLocation() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.default_save_location;
	}

	void DBusGroup::setDefaultSaveLocation(const QString & dir)
	{
		Group::Policy p = group->groupPolicy();
		p.default_save_location = dir;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
	
	double DBusGroup::maxShareRatio() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.max_share_ratio;
	}
	
	void DBusGroup::setMaxShareRatio(double ratio)
	{
		Group::Policy p = group->groupPolicy();
		p.max_share_ratio = ratio;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
	
	double DBusGroup::maxSeedTime() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.max_seed_time;
	}
	
	void DBusGroup::setMaxSeedTime(double hours)
	{
		Group::Policy p = group->groupPolicy();
		p.max_seed_time = hours;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
	
	uint DBusGroup::maxUploadSpeed() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.max_upload_rate;
	}
	
	void DBusGroup::setMaxUploadSpeed(uint speed)
	{
		Group::Policy p = group->groupPolicy();
		p.max_upload_rate = speed;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
	
	uint DBusGroup::maxDownloadSpeed() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.max_download_rate;
	}
	
	void DBusGroup::setMaxDownloadSpeed(uint speed)
	{
		Group::Policy p = group->groupPolicy();
		p.max_download_rate = speed;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
	
	bool DBusGroup::onlyApplyOnNewTorrents() const
	{
		const Group::Policy & p = group->groupPolicy();
		return p.only_apply_on_new_torrents;
	}
	
	void DBusGroup::setOnlyApplyOnNewTorrents(bool on)
	{
		Group::Policy p = group->groupPolicy();
		p.only_apply_on_new_torrents = on;
		group->setGroupPolicy(p);
		gman->saveGroups();
	}
}
