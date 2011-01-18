/*  This software is a thinlayer between KDE's IO-infrastructure and KTorrent.
    Copyright (C) 2010  Christian Weilbach < christian_weilbach 4T web D0T de >

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DBUSTHREAD_H
#define DBUSTHREAD_H

#include <QtCore/QThread>

class DBusHandler;

class DBusThread : public QThread
{
public:
    DBusThread( DBusHandler* );
    void run();
private:
    DBusHandler* m_handler;
};

#endif // DBUSTHREAD_H
