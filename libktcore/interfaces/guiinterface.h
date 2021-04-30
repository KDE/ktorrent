/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGUIINTERFACE_H
#define KTGUIINTERFACE_H

#include <QList>
#include <ktcore_export.h>

class QString;
class QProgressBar;
class KMainWindow;

namespace KIO
{
class Job;
}

namespace kt
{
class PrefPageInterface;
class Plugin;
class GUIInterface;
class Activity;
class TorrentActivityInterface;

/**
 * Base class for the status bar
 * */
class KTCORE_EXPORT StatusBarInterface
{
public:
    virtual ~StatusBarInterface()
    {
    }

    /// Show a message on the statusbar for some period of time
    virtual void message(const QString &msg) = 0;

    /// Create a progress bar and put it on the right side of the statusbar
    virtual QProgressBar *createProgressBar() = 0;

    /// Remove a progress bar created with createProgressBar (pb will be deleteLater'ed)
    virtual void removeProgressBar(QProgressBar *pb) = 0;
};

/**
 * @author Joris Guisson
 * @brief Interface to modify the GUI
 *
 * This interface allows plugins and others to modify the GUI.
 */
class KTCORE_EXPORT GUIInterface
{
public:
    GUIInterface();
    virtual ~GUIInterface();

    /// Get a pointer to the main window
    virtual KMainWindow *getMainWindow() = 0;

    /// Add an activity
    virtual void addActivity(Activity *act) = 0;

    /// Remove an activity
    virtual void removeActivity(Activity *act) = 0;

    /// Set the current activity
    virtual void setCurrentActivity(Activity *act) = 0;

    /**
     * Add a page to the preference dialog.
     * @param page The page
     */
    virtual void addPrefPage(PrefPageInterface *page) = 0;

    /**
     * Remove a page from the preference dialog.
     * @param page The page
     */
    virtual void removePrefPage(PrefPageInterface *page) = 0;

    /**
     * Merge the GUI of a plugin.
     * @param p The Plugin
     */
    virtual void mergePluginGui(Plugin *p) = 0;

    /**
     * Remove the GUI of a plugin.
     * @param p The Plugin
     */
    virtual void removePluginGui(Plugin *p) = 0;

    /// Show an error message box
    virtual void errorMsg(const QString &err) = 0;

    /// Show an error message for a KIO job which went wrong
    virtual void errorMsg(KIO::Job *j) = 0;

    /// Show an information dialog
    virtual void infoMsg(const QString &info) = 0;

    /// Get the status bar
    virtual StatusBarInterface *getStatusBar() = 0;

    /// Get the torrent activity
    virtual TorrentActivityInterface *getTorrentActivity() = 0;
};

}

#endif
