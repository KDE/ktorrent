/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QFocusEvent>
#include "hintlineedit.h"

namespace kt
{
	HintLineEdit::HintLineEdit(QWidget *parent) :
			KLineEdit(parent),
			default_focus_policy(focusPolicy()),
			hint_color(QColor(0xbbbbbb)),
			showing_hint_text(false)
	{
	}
	
	HintLineEdit::~HintLineEdit()
	{
	}
	
	void HintLineEdit::mousePressEvent(QMouseEvent *e)
	{
		KLineEdit::mousePressEvent(e);
	}
	
	void HintLineEdit::focusInEvent(QFocusEvent *e)
	{
		hideHintText();
		KLineEdit::focusInEvent(e);
	}
	
	void HintLineEdit::focusOutEvent(QFocusEvent *e)
	{
		// Focus out: Switch to displaying the hint text unless there is user input
		showHintText();
		KLineEdit::focusOutEvent(e);
	}
	
	QString HintLineEdit::hintText() const
	{
		return hint_text;
	}
	
	void HintLineEdit::setHintText(const QString &ht)
	{
		if (ht == hint_text)
			return;
		
		hideHintText();
		hint_text = ht;
		if (!hasFocus() && !ht.isEmpty())
			showHintText();
	}
	
	void HintLineEdit::showHintText(bool force)
	{
		if (showing_hint_text || hint_text.isEmpty())
			return;
		
		if (force || text().isEmpty()) 
		{
			showing_hint_text = true;
			setText(hint_text);
			setTextColor(hint_color, &text_color);
		}
	}
	void HintLineEdit::hideHintText()
	{
		if (showing_hint_text && !hint_text.isEmpty()) 
		{
			showing_hint_text = false;
			setText(QString());
			setTextColor(text_color);
		}
	}
	
	bool HintLineEdit::isShowingHintText() const
	{
		return showing_hint_text;
	}
	
	QString HintLineEdit::typedText() const
	{
		return showing_hint_text ? QString() : text();
	}
	
	void HintLineEdit::setTextColor(const QColor &newColor, QColor *oldColor)
	{
		QPalette pal = palette();
		if (oldColor)
			*oldColor = pal.color(QPalette::Text);
		pal.setColor(QPalette::Text, newColor);
		setPalette(pal);
	}
}

