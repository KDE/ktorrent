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
#ifndef BTOLDCHOKEALGORITHM_H
#define BTOLDCHOKEALGORITHM_H

#include <choker.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 * 
	 * The old choking algorithm as it is described on wiki.theory.org.
	*/
	class OldChokeAlgorithm : public ChokeAlgorithm
	{
		int opt_unchoke_index;
		int opt_unchoke;
		
		PeerPtrList downloaders,interested,not_interested;
	public:
		OldChokeAlgorithm();
		virtual ~OldChokeAlgorithm();

		virtual void doChoking(PeerManager& pman, bool have_all);
	private:
		void updateInterested(PeerManager& pman);
		void updateDownloaders();
		void sendInterested(PeerManager& pman,bool have_all);
		void sendUnchokes(bool have_all);
		void optimisticUnchoke(PeerManager& pman);
	};

}

#endif
