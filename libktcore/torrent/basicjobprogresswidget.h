/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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


#ifndef KT_BASICJOBPROGRESSWIDGET_H
#define KT_BASICJOBPROGRESSWIDGET_H

#include <torrent/jobprogresswidget.h>
#include "ui_basicjobprogresswidget.h"


namespace kt 
{
	/**
		Basic JobProgressWidget, showing a progress bar and the description
	 */
	class BasicJobProgressWidget : public kt::JobProgressWidget,public Ui_BasicJobProgressWidget
	{
		Q_OBJECT
	public:
		BasicJobProgressWidget(bt::Job* job,QWidget* parent);
		virtual ~BasicJobProgressWidget();
		
		virtual void description(const QString& title, const QPair< QString, QString >& field1, const QPair< QString, QString >& field2);
		virtual void infoMessage(const QString& plain, const QString& rich);
		virtual void warning(const QString& plain, const QString& rich);
		virtual void totalAmount(KJob::Unit unit, qulonglong amount);
		virtual void processedAmount(KJob::Unit unit, qulonglong amount);
		virtual void percent(long unsigned int percent);
		virtual void speed(long unsigned int value);
		
		virtual bool similar(Extender* ext) const
		{
			Q_UNUSED(ext);
			return false;
		}
	};

}

#endif // KT_BASICJOBPROGRESSWIDGET_H
