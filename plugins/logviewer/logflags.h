/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#ifndef KTLOGFLAGS_H
#define KTLOGFLAGS_H

class QString;

namespace kt
{
	struct _logFlags
	{
		unsigned int SYSCON;
		unsigned int SYSTRK;
		unsigned int SYSDHT;
		unsigned int SYSGEN;
		unsigned int SYSDIO;

		unsigned int SYSIPF;
		unsigned int SYSSRC;
		unsigned int SYSPNP;
		unsigned int SYSINW;
		unsigned int SYSSNF;
		unsigned int SYSPFI;
		unsigned int SYSSCD;
		unsigned int SYSRSS;
		unsigned int SYSWEB;
	};
	
	class LogViewer;

	/**
	 * Class to read/save logging messages flags.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class LogFlags
	{
		public:
			virtual ~LogFlags();
			
			static LogFlags& instance();
			
			///Checks current flags with arg. Return true if message should be shown
			bool checkFlags(unsigned int arg);
			
			///Updates flags from Settings::
			void updateFlags();
			
			///Destroys this object
			static void finalize();
			
			///Checks if LogViewer should print rich text format.
			bool useRichText();
			
			///Sets a pointer to LogViewer
			void setLog(LogViewer* log);
			
			///Makes line rich text according to arg level.
			QString& getFormattedMessage(unsigned int arg, QString& line);
			
		private:
			LogFlags();
			
			struct _logFlags m_flags;
			
			static LogFlags* self;
			
			static LogViewer* m_log;
			
			bool m_useRichText;
	};

}

#endif
