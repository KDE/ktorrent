/***************************************************************************
 *   Copyright (C) 2005 by Adam Treat                                      *
 *   treat@kde.org                                                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kurl.h>
#include <kglobal.h>
#include <kstartupinfo.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <util/functions.h>
#include <util/error.h>
#include <util/log.h>
#include <interfaces/functions.h>
#include <utp/connection.h>
#include "app.h"
#include "gui.h"


using namespace bt;

namespace kt
{
    GUI* App::main_widget = 0;

    App::App() : KUniqueApplication()
    {
    }

    App::~App()
    {
    }


    int App::newInstance()
    {
        // Add libktorrent catalog
        KGlobal::locale()->insertCatalog("libktorrent");
        KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
        kt::GUI* widget = 0;
        if (!main_widget)
        {
            bt::InitLog(kt::DataDir() + "log", true);
            widget = new kt::GUI();
            setTopWidget(widget);
            main_widget = widget;
        }
        else
        {
            widget = main_widget;
            widget->show();
        }

        if (widget)
        {
            for (int i = 0; i < args->count(); i++)
            {
                if (args->isSet("silent"))
                    widget->loadSilently(args->url(i));
                else
                    widget->load(args->url(i));
            }
        }
        args->clear();
        return 0;
    }

    bool App::notify(QObject* receiver, QEvent* event)
    {
        // This function is overridden so that we can catch our own exceptions
        // If they are uncaught, you get a crash.
        try
        {
            // We use QApplication::notify here because libkdeui is not compiled with exception support
            // Thus any uncaught exception leads to a crash.
            QEvent::Type t = event->type();

            // KApplication does special stuff in these circumstances, so best to keep doing that
            // there should be no chance of exceptions with show events of widgets
            if (t == QEvent::Show && receiver->isWidgetType())
                return KApplication::notify(receiver, event);
            else
                return QApplication::notify(receiver, event);
        }
        catch (bt::Error& err)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.toString() << endl;
        }
        catch (utp::Connection::TransmissionError& err)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.location << endl;
        }
        catch (std::exception& err)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught exception: " << err.what() << endl;
            throw; // Best to exit for std::bad_alloc and other standard exceptions
        }
        catch (...)
        {
            Out(SYS_GEN | LOG_IMPORTANT) << "Uncaught unknown exception " << endl;
        }

        return false;
    }

}

#include "app.moc"

