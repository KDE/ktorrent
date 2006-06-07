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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include "testrunner.h"

using namespace bt;

namespace utest
{

	TestRunner::TestRunner()
	{
		tests.setAutoDelete(true);
	}


	TestRunner::~TestRunner()
	{}

	void TestRunner::addTest(UnitTest* ut)
	{
		tests.append(ut);
	}
	
	void TestRunner::doAllTests()
	{
		int succes = 0;
		int failure = 0;
		QPtrList<UnitTest>::iterator i = tests.begin();
		while (i != tests.end())
		{
			Out() << "======================" << endl;
			UnitTest* t = *i;
			bool res = false;
			try
			{
				res = t->doTest();
			}
			catch (bt::Error & err)
			{
				Out() << "Caught Error : " << err.toString() << endl;
				res = false;
			}
			bt::Out() << "Doing test " << t->getName() << " : " << (res ? "SUCCES" : "FAILURE") << endl;
			if (res)
				succes++;
			else
				failure++;
			i++;
		}
		
		Out() << "======================" << endl;
		Out() << "Summary : " << endl;
		Out() << "\t" << succes << " succesfull tests" << endl;
		Out() << "\t" << failure << " failed tests" << endl;
	}
}
