/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@pandora.be                                              *
 *                                                                         *
 ***************************************************************************/
#include <kurl.h>
#include "log.h"
#include "error.h"

namespace bt
{
	Log::Log() : to_cout(true)
	{}
	
	
	Log::~Log()
	{}
	
	
	void Log::setOutputFile(const QString & file)
	{
		if (out.is_open())
			out.close();
		
		out.open(file.latin1());
		if (!out.is_open())
			throw Error("Cannot open file " + file);
	}
	
	Log & endl(Log & lg)
	{
		lg << "\n";
		lg.out.flush();
		return lg;
	}

	Log & Log::operator << (const KURL & url)
	{
		out << url.prettyURL().latin1();
		if (to_cout)
			std::cout << url.prettyURL().latin1();
		return *this;
	}
}	
