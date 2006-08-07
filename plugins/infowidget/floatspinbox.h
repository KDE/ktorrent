/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Vincent Wagelaar <vincent@ricardis.tudelft.nl>                        *
 *   Jonas Widarsson <jonas@widarsson.com>                                 *
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
#ifndef FLOATSPINBOX_H
#define FLOATSPINBOX_H

#include <qspinbox.h>

namespace kt
{
	
	/**
	* @author Jonas Widarsson
	*
	* A Spinbox for float values which respects KGlobal::locale().
	* QSpinBox's internal integer value is only
	* used for step detection.
	*/
	
	class FloatSpinBox : public QSpinBox
	{
		Q_OBJECT
	public:
		FloatSpinBox( QWidget* parent=0, const char* name=0 );
		FloatSpinBox( int precision = 2, QWidget* parent=0, const char* name=0 );
		FloatSpinBox( float minValue, float maxValue, float step = 0.5,  int precision = 2,
			QWidget* parent=0, const char* name=0 );
		virtual ~FloatSpinBox();
		
		QString mapValueToText( int value );
		
		int mapTextToValue( bool *ok );
		
		float minValue () const;
		float maxValue () const;
		float value () const;
		int precision() const;
	
	public slots:
		virtual void setMinValue ( float minValue );
		virtual void setMaxValue ( float maxValue );
		virtual void setValue ( float value );
		virtual void setStep ( float step );
		virtual void setPrecision( int value );
		virtual void stepUp();
		virtual void stepDown();
		
	signals:
		void valueChanged ( float value );
		void valueHasChanged ();
	
	private slots:
		void internalValueChanged( int value );
		
	private:
		int m_precision_digits;
		float m_value;
		float m_minValue;
		float m_maxValue;
		bool m_useRange;
		float m_step;
		int m_oldIntVal;
		float min( float a, float b) const
		{
			return (a < b) ? a : b;
		}
		
		float max( float a, float b) const
		{
			return (a > b) ? a : b;
		}
	};
}

#endif
