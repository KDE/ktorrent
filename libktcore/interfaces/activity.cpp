/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "activity.h"

#include <KXMLGUIFactory>
#include <QCollator>
#include <QMenu>

namespace kt
{
ActivityPart::ActivityPart(Activity *parent)
    : KParts::Part(parent)
{
}

ActivityPart::~ActivityPart()
{
}

void ActivityPart::setXMLGUIFile(const QString &xml_gui)
{
    setXMLFile(xml_gui, true);
}

QMenu *ActivityPart::menu(const QString &name)
{
    return qobject_cast<QMenu *>(factory()->container(name, this));
}

Activity::Activity(const QString &name, const QString &icon, int weight, QWidget *parent)
    : QWidget(parent)
    , activity_name(name)
    , activity_icon(icon)
    , activity_weight(weight)
    , activity_part(nullptr)
{
}

Activity::~Activity()
{
}

void Activity::setXMLGUIFile(const QString &xml_file)
{
    if (!activity_part)
        activity_part = new ActivityPart(this);

    activity_part->setXMLGUIFile(xml_file);
}

void Activity::setName(const QString &name)
{
    activity_name = name;
    nameChanged(this, name);
}

void Activity::setIcon(const QString &icon)
{
    activity_icon = icon;
    iconChanged(this, icon);
}

bool Activity::lessThan(Activity *l, Activity *r)
{
    if (l->weight() == r->weight())
        return QString::compare(l->name(), r->name()) < 0; // KF5 QCollator
    else
        return l->weight() < r->weight();
}
}
