/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@pandora.be                                              *
 *                                                                         *
 ***************************************************************************/
#include <kurl.h>
#include <qtextbrowser.h>
#include "log.h"
#include "error.h"

namespace bt
{
	Log::Log() : to_cout(false),widget(0),wo(0)
	{}
	
	
	Log::~Log()
	{
		delete wo;
	}
	
	
	void Log::setOutputFile(const QString & file)
	{
		if (fptr.isOpen())
			fptr.close();

		fptr.setName(file);
		if (!fptr.open(IO_WriteOnly))
			throw Error("Cannot open file " + file);

		out.setDevice(&fptr);
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
		lg.out << ::endl;
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
		out << url.prettyURL();
		if (to_cout)
			std::cout << url.prettyURL().latin1();
		
		if (widget)
		{
			*wo << url.prettyURL();
		}
		return *this;
	}

	Log & Log::operator << (const QString & s)
	{
		out << s;
		if (to_cout)
			std::cout << s.latin1();

		if (widget)
		{
			*wo << s;
		}
		
		return *this;
	}
}	
