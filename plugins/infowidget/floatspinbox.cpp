/***************************************************************************
 *   Copyright (C) 2006 by                                                 *
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

#include <kglobal.h>
#include <klocale.h>
#include <qlineedit.h>
#include "floatspinbox.h"
#include "localefloatvalidator.h"


kt::FloatSpinBox::FloatSpinBox( 
	QWidget * parent,
	const char * name
	)
	:QSpinBox( -100, 100, 1, parent, name), 
	m_precision_digits( 2 ),
	m_value(0.0f),
	m_minValue(0.0f),
	m_maxValue(0.0f),
	m_useRange(false),
	m_oldIntVal(0)
{
	QSpinBox::setValue(0);
	setStep( 0.25f );
	connect(this, SIGNAL(valueChanged ( int )), this, SLOT(internalValueChanged( int )));
	setValidator( new LocaleFloatValidator( dynamic_cast<QSpinBox * > (this) ));
	editor()->setAlignment(Qt::AlignRight);
}

kt::FloatSpinBox::FloatSpinBox(
	int precision,
	QWidget * parent,
	const char * name
	)
	:QSpinBox( -100, 100, 1, parent, name), 
	m_precision_digits( precision ),
	m_value(0.0f),
	m_minValue(0.0f),
	m_maxValue(0.0f),
	m_useRange(false),
	m_oldIntVal(0)
{
	QSpinBox::setValue(0);
	setStep( 0.25 );
	connect(this, SIGNAL(valueChanged ( int )), this, SLOT(internalValueChanged( int )));
	setValidator( new LocaleFloatValidator( dynamic_cast<QSpinBox * > (this) ));
}


kt::FloatSpinBox::FloatSpinBox( 
	float minValue, 
	float maxValue, 
	float step, 
	int precision,  
	QWidget * parent, 
	const char * name
	)
	:QSpinBox( -100, 100, 1, parent, name), 
	m_precision_digits( precision ),
	m_value(0.0f),
	m_minValue(minValue),
	m_maxValue(maxValue),
	m_useRange(true),
	m_oldIntVal(0)
{
	QSpinBox::setValue(0);
	setValue(0.0f);
	setStep( step );
	connect(this, SIGNAL(valueChanged ( int )), this, SLOT(internalValueChanged( int )));
	setValidator( new LocaleFloatValidator( this ));

}

QString kt::FloatSpinBox::mapValueToText( int value )
{
	/// This is called from QSpinBox, which passes an int.
	/// As we don't use the QSpinBox's internal value, we ignore it.
	QString t = KGlobal::locale()->formatNumber( m_value, m_precision_digits);
	editor()->setText(t);
	return t;
}

int kt::FloatSpinBox::mapTextToValue( bool * ok )
{
	/// This is called from QSpinBox, which needs an int for return.
	/// As we don't use the QSpinBox's internal value, we only return 0.
	
	float value = KGlobal::locale()->readNumber(text(), ok);
	if (*ok)
	{
		setValue(value);
		*ok = true;
	}
	return 1;
}

float kt::FloatSpinBox::minValue( ) const
{
	return m_minValue;
}

float kt::FloatSpinBox::maxValue( ) const
{
	return m_maxValue;
}

float kt::FloatSpinBox::value( ) const
{
	return m_value;
}

int kt::FloatSpinBox::precision( ) const
{
	return m_precision_digits;
}

void kt::FloatSpinBox::setMinValue( float minValue )
{
	if (minValue <= m_maxValue)
	{
		m_minValue = minValue;
		m_useRange = true;
	}
}

void kt::FloatSpinBox::setMaxValue( float maxValue )
{
	if (maxValue >= m_minValue)
	{
		m_maxValue = maxValue;
		m_useRange = true;
	}
}

void kt::FloatSpinBox::setValue( float value )
{
	bool changed = false;
	if (m_useRange)
	{
		float old = m_value;
		m_value = max(m_minValue, min(m_maxValue, value));
		if ( old != m_value )
			changed = true;
	}
	else
	{
		m_value = value;
		changed = true;
	}
	if (changed)
	{
		mapValueToText(0);
		emit valueChanged( m_value );
		emit valueHasChanged();
	}
}

void kt::FloatSpinBox::setStep( float step)
{
	if (step > 0)
		m_step = step;
}

void kt::FloatSpinBox::internalValueChanged( int value )
{
	/// The step buttons won't work without tracking the 
	/// QSpinbox's value changes.
	
	if ( value > m_oldIntVal)
	{	
		stepUp();
	}
	else
	{
		stepDown();
	}
	
	if (value > 10)
		value -= 20;
		
	if (value < -10)
		value += 20;
		
	m_oldIntVal = value;
}

void kt::FloatSpinBox::setPrecision( int value )
{
	m_precision_digits = value;
}

void kt::FloatSpinBox::stepUp( )
{
	setValue( m_value + m_step );
}

void kt::FloatSpinBox::stepDown( )
{
	setValue( m_value - m_step );
}

kt::FloatSpinBox::~ FloatSpinBox( )
{
}


#include "floatspinbox.moc"
