  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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
#ifndef PHP_HANDLER_H
#define PHP_HANDLER_H
#include <qprocess.h>
#include <qtimer.h>
#include <torrent/queuemanager.h>
#include <interfaces/torrentinterface.h>
#include "php_interface.h"

class PhpHandler{
	public:
		PhpHandler(PhpInterface *php);
		~PhpHandler();
		bool executeScript(QString, QString, QMap<QString, QString> );
		QString getOutput(){return output;};
	private:
		void preParse(QString *d, QMap<QString, QString> requestVars);
		QString output;
		PhpInterface *php_i;
		QFileInfo fi;
};
#endif
