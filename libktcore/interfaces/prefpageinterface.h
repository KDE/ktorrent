/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PREFPAGEINTERFACE_H
#define PREFPAGEINTERFACE_H

#include <QWidget>
#include <ktcore_export.h>

class KConfigSkeleton;

namespace kt
{
/**
 * @author Ivan Vasic
 * @brief Interface to add configuration dialog page.
 *
 * This interface allows plugins and others to add their own pages in Configuration dialog.
 */
class KTCORE_EXPORT PrefPageInterface : public QWidget
{
    Q_OBJECT
public:
    PrefPageInterface(KConfigSkeleton *cfg, const QString &name, const QString &icon, QWidget *parent);
    ~PrefPageInterface() override;

    /**
     * Initialize the settings.
     * Called by the settings dialog when it is created.
     */
    virtual void loadSettings();

    /**
     * Load default settings.
     * Called when the defaults button is pressed in the settings dialog.
     */
    virtual void loadDefaults();

    /**
     * Called when user presses OK or apply.
     */
    virtual void updateSettings();

    KConfigSkeleton *config()
    {
        return cfg;
    }
    const QString &pageName()
    {
        return name;
    }
    const QString &pageIcon()
    {
        return icon;
    }

    /// Override if there are custom widgets outside which have changed
    virtual bool customWidgetsChanged()
    {
        return false;
    }

Q_SIGNALS:
    /// Emitted when buttons need to be updated
    void updateButtons();

private:
    KConfigSkeleton *cfg;
    QString name;
    QString icon;
};
}
#endif
