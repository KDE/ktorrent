/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCRIPT_H
#define KTSCRIPT_H

#include <QObject>

namespace Kross
{
class Action;
}

namespace kt
{
/**
    Keeps track of a script
*/
class Script : public QObject
{
    Q_OBJECT
public:
    Script(QObject *parent);
    Script(const QString &file, QObject *parent);
    ~Script() override;

    struct MetaInfo {
        QString name;
        QString comment;
        QString icon;
        QString author;
        QString email;
        QString website;
        QString license;

        bool valid() const
        {
            return !name.isEmpty() && !comment.isEmpty() && !icon.isEmpty() && !author.isEmpty() && !license.isEmpty();
        }
    };

    /**
     * Load the script from a desktop file
     * @param dir THe directory the desktop file is in
     * @param desktop_file The desktop file itself (relative to dir)
     * @return true upon success
     */
    bool loadFromDesktopFile(const QString &dir, const QString &desktop_file);

    /**
     * Load and execute the script
     * @return true upon success
     */
    bool execute();

    /// Is the script executeable (i.e. is the interpreter not installed)
    bool executeable() const;

    /**
     * Stop the script
     */
    void stop();

    /// Is the script running
    bool running() const
    {
        return executing;
    }

    /// Get the name of the script
    QString name() const;

    /// Get the icon name of the script
    QString iconName() const;

    /// Get the file
    QString scriptFile() const
    {
        return file;
    }

    /// Get the package directory, this returns an empty string if the script is just a file
    QString packageDirectory() const
    {
        return package_directory;
    }

    /// Set the package directory
    void setPackageDirectory(const QString &dir)
    {
        package_directory = dir;
    }

    /// Get the meta info of the script
    const MetaInfo &metaInfo() const
    {
        return info;
    }

    /// Does the script has a configure function
    bool hasConfigure() const;

    /// Call the configure function of the script
    void configure();

    /// Whether or not the script can be removed
    bool removable() const
    {
        return can_be_removed;
    }

    /// Set the script to be removable or not
    void setRemovable(bool on)
    {
        can_be_removed = on;
    }

private:
    QString file;
    Kross::Action *action;
    bool executing;
    MetaInfo info;
    bool can_be_removed;
    QString package_directory;
};

}

#endif
