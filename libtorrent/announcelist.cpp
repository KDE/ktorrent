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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "announcelist.h"
#include "bnode.h"
#include "error.h"

namespace bt
{

	AnnounceList::AnnounceList()
	{
		curr = 0;
	}


	AnnounceList::~AnnounceList()
	{}

	void AnnounceList::load(BNode* node)
	{
		BListNode* ml = dynamic_cast<BListNode*>(node);
		if (!ml)
			return;
		
		//ml->printDebugInfo();
		for (Uint32 i = 0;i < ml->getNumChildren();i++)
		{
			BListNode* url = dynamic_cast<BListNode*>(ml->getChild(i));
			if (!url)
				throw Error("Parse Error");
			
			for (Uint32 j = 0;j < url->getNumChildren();j++)
			{
				BValueNode* vn = dynamic_cast<BValueNode*>(url->getChild(j));
				if (!vn)
					throw Error("Parse Error");
				
				trackers.append(KURL(vn->data().toString()));
			}
		}
	}
	
	KURL AnnounceList::getTrackerURL(bool last_was_succesfull) const
	{
		if (last_was_succesfull)
			return *trackers.at(curr);
		
		curr = (curr + 1) % trackers.count();
		return *trackers.at(curr);
	}

}
