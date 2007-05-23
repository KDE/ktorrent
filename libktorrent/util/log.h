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

#ifndef JORISLOG_H
#define JORISLOG_H


#include "constants.h"
#include <qstring.h>

// LOG MESSAGES CONSTANTS
#define LOG_NONE 0x00
#define LOG_IMPORTANT 0x01
#define LOG_NOTICE 0x03
#define LOG_DEBUG 0x07
#define LOG_ALL 0x0F

#define SYS_GEN 0x0010 // Genereral info messages
#define SYS_CON 0x0020 // Connections
#define SYS_TRK 0x0040 // Tracker
#define SYS_DHT 0x0080 // DHT
#define SYS_DIO 0x0100 // Disk IO related stuff, saving and loading of chunks ...

//plugins
#define SYS_IPF 0x1000  // IPFilter
#define SYS_SRC 0x2000  // Search plugin
#define SYS_PNP 0x4000  // UPnP plugin
#define SYS_INW 0x8000  // InfoWidget
#define SYS_SNF 0x10000 // ScanFolder plugin
#define SYS_PFI 0x20000 // Part file import
#define SYS_SCD 0x40000 // Scheduler plugin
#define SYS_RSS 0x80000 // RSS plugin
#define SYS_WEB 0x100000 // WebInterface plugin
#define SYS_ZCO 0x200000 // ZeroConf plugin

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
		
		/**
		 * Sets a filter for log messages. Applies only to listeners via LogMonitorInterface!
		 * @param filter SYS & LOG flags combined with bitwise OR.
		 */
		void setFilter(unsigned int filter);
		
		/// Lock the mutex of the log, should be called in Out()
		void lock();
	
		/// Called by the auto log rotate job when it has finished
		void logRotateDone();
	};

	Log & endl(Log & lg);
	
	
	Log & Out(unsigned int arg = 0x00);
	inline Log & GenOut(unsigned int arg) {return Out(SYS_GEN|arg);}
	inline Log & DHTOut(unsigned int arg) {return Out(SYS_DHT|arg);}
	inline Log & ConOut(unsigned int arg) {return Out(SYS_CON|arg);}
	inline Log & TrkOut(unsigned int arg) {return Out(SYS_TRK|arg);}

}

#endif
