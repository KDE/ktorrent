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
#ifndef BTDATACHECKERLISTENER_H
#define BTDATACHECKERLISTENER_H

#include <util/constants.h>

namespace bt
{

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class DataCheckerListener
	{
	public:
		DataCheckerListener(bool auto_import);
		virtual ~DataCheckerListener();

		/**
		 * Called when a chunk has  been proccessed.
		 * @param num The number processed
		 * @param total The total number of pieces to process
		*/
		virtual void progress(Uint32 num,Uint32 total) = 0;
	
		/**
		 * Called when a failed or dowloaded chunk is found.
		 * @param num_failed The number of failed chunks
		 * @param num_downloaded Number of downloaded chunks
		 */
		virtual void status(Uint32 num_failed,Uint32 num_downloaded) = 0;
		
		/**
		 * Data check has been finished.
		 */
		virtual void finished() = 0;
		
		/**
		 * Test if we need to stop.
		 */
		bool needToStop() const {return stopped;}
		
		/// Check if the check has been stopped
		bool isStopped() const {return stopped;}
		
		/// Is this an auto_import
		bool isAutoImport() const {return auto_import;}
		
		/**
		 * Stop the data check.
		 */
		void stop() {stopped = true;}
	private:
		bool stopped;
	
	protected:
		bool auto_import;
	};

}

#endif
