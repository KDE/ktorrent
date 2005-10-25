/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <kurl.h>
#include <klocale.h>
#include <qtextbrowser.h>
#include "log.h"
#include "error.h"

namespace bt
{
	Log::Log() : out(0),to_cout(false),widget(0),wo(0)
	{
		out = new QTextStream();
	}
	
	
	Log::~Log()
	{
		delete out;
		delete wo;
	}
	
	
	void Log::setOutputFile(const QString & file)
	{
		if (fptr.isOpen())
			fptr.close();

		fptr.setName(file);
		if (!fptr.open(IO_WriteOnly))
			throw Error(i18n("Cannot open log file %1").arg(file));

		out->setDevice(&fptr);
	}

	void Log::setOutputWidget(QTextBrowser* widget)
	{
		this->widget = widget;
		if (wo)
		{
			delete wo;
			wo = 0;
		}

		if (widget)
			wo = new QTextOStream(&tmp);
	}
	
	Log & endl(Log & lg)
	{
		*lg.out << ::endl;
		if (lg.to_cout)
			std::cout << std::endl;

		if (lg.widget)
		{
			lg.widget->append(lg.tmp);
			lg.tmp = "";
			delete lg.wo;
			lg.wo = new QTextOStream(&lg.tmp);
		}
		return lg;
	}

	Log & Log::operator << (const KURL & url)
	{
		*out << url.prettyURL();
		if (to_cout)
			std::cout << url.prettyURL().local8Bit();
		
		if (widget)
		{
			*wo << url.prettyURL();
		}
		return *this;
	}

	Log & Log::operator << (const QString & s)
	{
		*out << s;
		if (to_cout)
			std::cout << s.local8Bit();

		if (widget)
		{
			*wo << s;
		}
		
		return *this;
	}

	Log & Log::operator << (Uint64 v)
	{
		return operator << (QString::number(v));
	}

	Log & Log::operator << (Int64 v)
	{
		return operator << (QString::number(v));
	}
}	
