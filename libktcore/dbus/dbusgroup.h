/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDBUSGROUP_H
#define KTDBUSGROUP_H

#include <QObject>

namespace kt
{
class Group;
class GroupManager;

/**
    @author
*/
class DBusGroup : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ktorrent.group")
public:
    DBusGroup(Group *g, GroupManager *gman, QObject *parent);
    ~DBusGroup() override;

public Q_SLOTS:
    Q_SCRIPTABLE QString name() const;
    Q_SCRIPTABLE QString icon() const;
    Q_SCRIPTABLE QString defaultSaveLocation() const;
    Q_SCRIPTABLE void setDefaultSaveLocation(const QString &dir);
    Q_SCRIPTABLE QString defaultMoveOnCompletionLocation() const;
    Q_SCRIPTABLE void setDefaultMoveOnCompletionLocation(const QString &dir);
    Q_SCRIPTABLE double maxShareRatio() const;
    Q_SCRIPTABLE void setMaxShareRatio(double ratio);
    Q_SCRIPTABLE double maxSeedTime() const;
    Q_SCRIPTABLE void setMaxSeedTime(double hours);
    Q_SCRIPTABLE uint maxUploadSpeed() const;
    Q_SCRIPTABLE void setMaxUploadSpeed(uint speed);
    Q_SCRIPTABLE uint maxDownloadSpeed() const;
    Q_SCRIPTABLE void setMaxDownloadSpeed(uint speed);
    Q_SCRIPTABLE bool onlyApplyOnNewTorrents() const;
    Q_SCRIPTABLE void setOnlyApplyOnNewTorrents(bool on);

private:
    Group *group;
    GroupManager *gman;
};

}

#endif
