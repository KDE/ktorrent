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

#include "capturechecker.h"

namespace kt
	{
	
	CaptureChecker::CaptureChecker(QObject * parent) : QObject(parent)
		{
		}
	
	bool CaptureChecker::addNewCapture(const QString& name)
		{
		if (captures.contains(name))
			return false;
		
		captures.insert(name, "");
		return true;
		}
	
	bool CaptureChecker::setCaptureValue(const QString& name, const QString& value)
		{
		if (!captures.contains(name))
			return false;
		
		captures.insert(name, value);
		return true;
		}
	}