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

#include "app.h"

#include <util/log.h>
#include <torrent/globals.h>
#include <util/functions.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include <utp/connection.h>

using namespace bt;

namespace kt
{
/*
    bool App::notify(QObject* receiver, QEvent* event)
    {
        // This function is overridden so that we can catch our own exceptions
        // If they are uncaught, you get a crash.
        try
        {
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
*/
}
