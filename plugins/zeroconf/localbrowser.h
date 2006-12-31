/***************************************************************************
 *   Copyright (C) 2006 by Lesly Weyts and Kevin Andre                     *
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
#ifndef LOCAL_BROWSER_HHHH
#define LOCAL_BROWSER_HHHH

/**
 * @author Lesly Weyts and Kevin Andre
 * @brief Keep track of local peers
 */


 
namespace bt { class PeerID; }
 
namespace LocalBrowser {
 
	void remove(bt::PeerID);
	void insert(bt::PeerID);
	bool check (bt::PeerID);
 
	
}
#endif
