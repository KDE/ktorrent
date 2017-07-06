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

#ifndef KTSYNDICATIONPLUGIN_H
#define KTSYNDICATIONPLUGIN_H

#include <interfaces/plugin.h>

class QAction;

namespace kt
{
    class SyndicationActivity;


    /**
        @author
    */
    class SyndicationPlugin : public Plugin
    {

    public:
        SyndicationPlugin(QObject* parent, const QVariantList& args);
        ~SyndicationPlugin();

        bool versionCheck(const QString& version) const override;
        void load() override;
        void unload() override;

    private:
        void setupActions();

    private:
        QAction * add_feed;
        QAction * remove_feed;
        QAction * edit_feed_name;
        QAction * add_filter;
        QAction * remove_filter;
        QAction * edit_filter;
        QAction * manage_filters;
        SyndicationActivity* activity;

        friend class SyndicationActivity;
    };

}

#endif
