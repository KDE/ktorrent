/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@pandora.be                                              *
 *                                                                         *
 ***************************************************************************/
#ifndef JORISLOG_H
#define JORISLOG_H

#include <qstring.h>
#include <iostream>
#include <fstream>

class KURL;

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
	*/
	class Log
	{
		std::ofstream out;
		bool to_cout;
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
		void setOutputToConsole(bool on) {to_cout = on;}
		
		/**
		* Set the output logfile.
		* @param file The name of the file
		* @throw Exception if the file can't be opened
		*/
		void setOutputFile(const QString & file);
		
		/**
		* Write something to the log file.
		* Anything which can be passed to std::ostream using
		* the << operator is allowed.
		* @param val The value
		* @return This Log
		*/
		template <class T>
		Log & operator << (T val)
		{
			out << val;
			if (to_cout)
				std::cout << val;
			return *this;
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
		Log & operator << (const QString & s)
		{
			out << s.latin1();
			if (to_cout)
				std::cout << s.latin1();
			return *this;
		}
		
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
}

#endif
