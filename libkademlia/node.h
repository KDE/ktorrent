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
#ifndef DHTNODE_H
#define DHTNODE_H

#include <qobject.h>
#include "key.h"
#include "kbucket.h"


namespace dht
{

	/**
	 * @author Joris Guisson
	 *
	 * A Node represents us in the kademlia network. It contains
	 * our id and 160 KBucket's.
	 * A KBucketEntry is in node i, when the difference between our id and
	 * the KBucketEntry's id is between 2 to the power i and 2 to the power i+1.
	*/
	class Node : public QObject
	{
		Q_OBJECT
	public:
		Node();
		virtual ~Node();

	private:
		Key id;
		KBucket* bucket[160];
	};

}

#endif
