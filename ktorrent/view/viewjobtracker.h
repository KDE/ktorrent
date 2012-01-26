/***************************************************************************
 *   Copyright (C) 2011 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#ifndef KT_VIEWJOBTRACKER_H
#define KT_VIEWJOBTRACKER_H

#include <torrent/jobtracker.h>


namespace kt
{

	class ScanExtender;
	class View;

	/**
		JobTracker for the View
	 */
	class ViewJobTracker : public kt::JobTracker
	{
		Q_OBJECT
	public:
		ViewJobTracker(View* parent);
		virtual ~ViewJobTracker();

		virtual void jobUnregistered(bt::Job* j);
		virtual void jobRegistered(bt::Job* j);
		virtual kt::JobProgressWidget* createJobWidget(bt::Job* job);
		
	private:
		View* view;
	};

}

#endif // KT_VIEWJOBTRACKER_H
