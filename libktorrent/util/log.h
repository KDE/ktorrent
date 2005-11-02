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

#ifndef JORISLOG_H
#define JORISLOG_H


#include "constants.h"
#include <qstring.h>

class KURL;


namespace kt
{
	class LogMonitorInterface;
}

namespace bt
{
	
	
	/**
	* @author Joris Guisson
	* @brief Class which writes messages to a logfile
	*
	* This class writes messages to a logfile. To use it, create an instance,
	* set the output file and write stuff with the << operator.
	*
	* By default all messages will also be printed on the standard output. This
	* can be turned down using the @a setOutputToConsole function.
	*
	* There is also the possibility to monitor what is written to the log using
	* the LogMonitorInterface class.
	*/
	class Log 
	{
		class Private;
		
		Private* priv;
	public:
		/**
		* Constructor.
		*/
		Log();
		
		/**
		* Destructor, closes the file.
		*/
		virtual ~Log();
	
		/**
		* Enable or disable the printing of log messages to the standard
		* output.
		* @param on Enable or disable
		*/
		void setOutputToConsole(bool on);

		/**
		 * Add a log monitor.
		 * @param m The log monitor
		 */
		void addMonitor(kt::LogMonitorInterface* m);

		/**
		 * Remove a log monitor.
		 * @param m The log monitor
		 */
		void removeMonitor(kt::LogMonitorInterface* m);
		
		/**
		* Set the output logfile.
		* @param file The name of the file
		* @throw Exception if the file can't be opened
		*/
		void setOutputFile(const QString & file);
		
		/**
		* Write a number to the log file.
		* Anything which can be passed to QString::number will do.
		* @param val The value
		* @return This Log
		*/
		template <class T>
		Log & operator << (T val)
		{
			return operator << (QString::number(val));
		}
		
		/**
		* Apply a function to the Log.
		* @param func The function
		* @return This Log
		*/
		Log & operator << (Log & (*func)(Log & ))
		{
			return func(*this);
		}

		
		/**
		 * Output a QString to the log.
		 * @param s The QString
		 * @return This Log
		 */
		Log & operator << (const char* s);

		/**
		 * Output a QString to the log.
		 * @param s The QString
		 * @return This Log
		 */
		Log & operator << (const QString & s);

		/**
		 * Output a 64 bit integer to the log.
		 * @param v The integer
		 * @return This Log
		 */
		Log & operator << (Uint64 v);

		/**
		 * Output a 64 bit integer to the log.
		 * @param v The integer
		 * @return This Log
		 */
		Log & operator << (Int64 v);
		
		/**
		* Prints and endline character to the Log and flushes it.
		* @param lg The Log
		* @return @a lg
		*/
		friend Log & endl(Log & lg);

		/**
		 * Write an URL to the file.
		 * @param text The KURL
		 * @return This Log
		 */
		Log & operator << (const KURL & url);
	};

	Log & endl(Log & lg);

}

#endif
