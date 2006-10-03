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
#ifndef BTTIMEESTIMATOR_H
#define BTTIMEESTIMATOR_H

#include <util/constants.h>

namespace bt
{
	class TorrentControl;
	
	/**
	 * Simple queue class for samples. Optimized for speed and size
	 * without posibility to dynamically resize itself.
	 * @author Ivan Vasic <ivasic@gmail.com>
	 */
	class SampleQueue
	{
		public:
			SampleQueue(int max);
			~SampleQueue();
			
			/**
			 * Inserts new sample into the queue. The oldest sample is overwritten.
			 */
			void push(Uint32 sample);
			
			Uint32 first();
			Uint32 last();
			
			bool isFull();
			
			/**
			 * This function will return the number of samples in queue until it counts m_size number of elements.
			 * After this point it will always return m_size since no samples are being deleted.
			 */
			int count();
			
			/**
			 * Returns the sum of all samples.
			 */
			Uint32 sum();
		
		private:
			int m_size;
			int m_count;
			
			int m_start;
			int m_end;
			
			Uint32* m_samples;
	};

	/**
	 * ETA estimator class. It will use different algorithms for different download phases.	
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class TimeEstimator
	{
		public:
			TimeEstimator(TorrentControl* tc);
			~TimeEstimator();
			
			///Returns ETA for m_tc torrent.
			Uint32 estimate();
			
		private:
			
			Uint32 estimateCSA();
			Uint32 estimateGASA();
			Uint32 estimateWINX();
			Uint32 estimateMAVG();
			
			TorrentControl* m_tc;
			SampleQueue* m_samples;

			Uint32 m_lastAvg;
			Uint32 m_lastETA;
			
			//last percentage
			double m_perc;
	};

}

#endif
