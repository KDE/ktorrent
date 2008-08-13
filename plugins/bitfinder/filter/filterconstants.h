/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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
 
#ifndef KTFILTERCONSTANTS_H
#define KTFILTERCONSTANTS_H

#include <QStringList>

enum FilterType
	{
	FT_REJECT,
	FT_ACCEPT
	};

const QStringList FilterTypeText = QStringList() << "Reject" << "Accept";

enum MultiMatch
	{
	MM_ONCE_ONLY,
	MM_ALWAYS_MATCH,
	MM_CAPTURE_CHECKING
	};
	
const QStringList MultiMatchText = QStringList() << "Once" << "Always" << "CaptureCheck";

enum Rerelease
	{
	RR_IGNORE,
	RR_DOWNLOAD_ALL,
	RR_DOWNLOAD_FIRST
	};
	
const QStringList RereleaseText = QStringList() << "Ignore" << "All" << "First";
	
enum SourceListType
	{
	SL_EXCLUSIVE,
	SL_INCLUSIVE
	};

const QStringList SourceListTypeText = QStringList() << "Exclusive" << "Inclusive";

#endif
