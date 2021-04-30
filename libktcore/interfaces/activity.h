/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <KParts/Part>
#include <KSharedConfig>
#include <QWidget>

#include <ktcore_export.h>

class QMenu;

namespace kt
{
class Activity;

/**
    Part of an Activity
*/
class KTCORE_EXPORT ActivityPart : public KParts::Part
{
    Q_OBJECT
public:
    ActivityPart(Activity *parent);
    ~ActivityPart() override;

    /// Set the XML GUI file of the part
    void setXMLGUIFile(const QString &xml_gui);

    /// Get a menu described in the XML of the part
    QMenu *menu(const QString &name);
};

/**
 * Base class for all activities.
 */
class KTCORE_EXPORT Activity : public QWidget
{
    Q_OBJECT
public:
    Activity(const QString &name, const QString &icon, int weight, QWidget *parent);
    ~Activity() override;

    /// Get the name of the activity
    const QString &name() const
    {
        return activity_name;
    }

    /// Get the icon name
    const QString &icon() const
    {
        return activity_icon;
    }

    /// Get the part
    ActivityPart *part() const
    {
        return activity_part;
    }

    /// Set the name
    void setName(const QString &name);

    /// Set the icon
    void setIcon(const QString &icon);

    /// Get the weight
    int weight() const
    {
        return activity_weight;
    }

    static bool lessThan(Activity *l, Activity *r);

protected:
    /**
        Activities wishing to provide toolbar and menu entries, should
        call this function to set the XML GUI description.
        @param xml_file The XMLGUI file
    */
    void setXMLGUIFile(const QString &xml_file);

Q_SIGNALS:
    void nameChanged(Activity *a, const QString &name);
    void iconChanged(Activity *a, const QString &icon);

private:
    QString activity_name;
    QString activity_icon;
    int activity_weight;
    ActivityPart *activity_part;
};
}

#endif // ACTIVITY_H
