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
#ifndef KT_HINTLINEEDIT_H
#define KT_HINTLINEEDIT_H

#include <ktcore_export.h>
#include <klineedit.h>


namespace kt 
{
	/**
		Line edit which shows a hint when no text is shown.
		
		This class is a modified version of Qt designer's HintLineEdit (hence the copyright above)
	*/
	class KTCORE_EXPORT HintLineEdit : public KLineEdit 
	{
		Q_OBJECT
	public:
		HintLineEdit(QWidget *parent = 0);
		virtual ~HintLineEdit();
		
		/// Hint text being used
		QString hintText() const;
		
		/// Are we showing the hint text
		bool isShowingHintText() const;
		
		/// Convenience for accessing the text that returns "" in case of isShowingHintText().
		QString typedText() const;
		
	public slots:
		void setHintText(const QString &ht);
		void showHintText(bool force = false);
		void hideHintText();
		
	protected:
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void focusInEvent(QFocusEvent *e);
		virtual void focusOutEvent(QFocusEvent *e);
		
	private:
		void setTextColor(const QColor &newColor, QColor *oldColor = 0);
		
		const Qt::FocusPolicy default_focus_policy;
		const QColor hint_color;
		QColor text_color;
		QString hint_text;
		bool showing_hint_text;
	};
}

#endif // KT_HINTLINEEDIT_H
