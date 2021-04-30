/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_EXTENDER_H
#define KT_EXTENDER_H

#include <QWidget>
#include <ktcore_export.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
 * Base class for all extender widgets
 */
class KTCORE_EXPORT Extender : public QWidget
{
    Q_OBJECT
public:
    Extender(bt::TorrentInterface *tc, QWidget *parent);
    ~Extender() override;

    /// Get the torrent of this extender
    bt::TorrentInterface *torrent()
    {
        return tc;
    }

    /// Is this similar to another extender
    virtual bool similar(Extender *ext) const = 0;

Q_SIGNALS:
    /// Should be emitted by an extender when it wants to close itself
    void closeRequest(Extender *ext);

    /// Should be emitted when an extender is resized
    void resized(Extender *ext);

protected:
    bt::TorrentInterface *tc;
};

}

#endif // KT_EXTENDER_H
