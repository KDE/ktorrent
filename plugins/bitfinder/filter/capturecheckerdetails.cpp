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
#include <kinputdialog.h>

#include "capturecheckerdetails.h"

#include <util/log.h>

using namespace bt;

namespace kt
	{
	
	CaptureCheckerDetails::CaptureCheckerDetails(QWidget * parent) 
			: QWidget(parent)
		{
		setupUi(this);
		
		//Let's build the toolbars we need
		//capturesToolbar
		capturesToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		capturesToolbar->setOrientation(Qt::Vertical);
		capturesToolbar->setIconDimensions(16);
		captureAdd = new KAction(KIcon("list-add"),i18n("Add Capture"),this);
		captureRemove = new KAction(KIcon("list-remove"),i18n("Remove Capture"),this);
		capturesToolbar->addAction(captureAdd);
		capturesToolbar->addAction(captureRemove);
		connect(captureAdd, SIGNAL(triggered( bool )), this, SLOT(addNewCapture()));

		//variablesToolbar
		variablesToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		variablesToolbar->setOrientation(Qt::Vertical);
		variablesToolbar->setIconDimensions(16);
		variableAdd = new KAction(KIcon("list-add"),i18n("Add Variable"),this);
		variableRemove = new KAction(KIcon("list-remove"),i18n("Remove Variable"),this);
		variableUp = new KAction(KIcon("arrow-up"), i18n("Move Variable Up"), this);
		variableDown = new KAction(KIcon("arrow-down"), i18n("Move Variable Down"), this);
		variablesToolbar->addAction(variableAdd);
		variablesToolbar->addAction(variableRemove);
		variablesToolbar->addAction(variableUp);
		variablesToolbar->addAction(variableDown);
		connect(variableAdd, SIGNAL(triggered( bool )), this, SLOT(addNewVariable()));
		
		//captures
		captures->setColumnCount(2);
		QStringList captureHeaders;
		captureHeaders << "Name" << "Expression";
		captures->setHorizontalHeaderLabels(captureHeaders);
		connect(captures, SIGNAL(cellChanged(int, int)), this, SLOT(emitCaptures()));
		
		//variables
		variables->setColumnCount(3);
		QStringList variableHeaders;
		variableHeaders << "Name" << "Min" << "Max";
		variables->setHorizontalHeaderLabels(variableHeaders);
		connect(variables, SIGNAL(cellChanged(int, int)), this, SLOT(emitVariables()));
		
		//mappings
		mappings->setColumnCount(4);
		QStringList mappingHeaders;
		mappingHeaders << "Capture Name" << "Variable" << "Index" << "Test Value";
		mappings->setHorizontalHeaderLabels(mappingHeaders);
		connect(mappings, SIGNAL(cellChanged(int, int)), this, SLOT(verifyMappingInput(int, int)));
		
		}
	
	void CaptureCheckerDetails::resizeColumns()
		{
		if (captures->columnWidth(1) != 65 && captures->width()!=100)
			return;
		
		int captureWidth = captures->width();
		captures->setColumnWidth(CAPTURE_NAME, captureWidth * 0.35 - 5);
		captures->setColumnWidth(CAPTURE_VALUE, captureWidth * 0.65);
		
		int variableWidth = variables->width();
		variables->setColumnWidth(VARIABLE_NAME, variableWidth * 0.5 - 5);
		variables->setColumnWidth(VARIABLE_MIN, variableWidth * 0.25);
		variables->setColumnWidth(VARIABLE_MAX, variableWidth * 0.25);
		
		int mappingWidth = mappings->width();
		mappings->setColumnWidth(MAP_CAPTURE, mappingWidth * 0.2);
		mappings->setColumnWidth(MAP_VARIABLE, mappingWidth * 0.2);
		mappings->setColumnWidth(MAP_INDEX, mappingWidth * 0.1);
		mappings->setColumnWidth(MAP_TEST, mappingWidth * 0.5 - 5);
		}
	
	void CaptureCheckerDetails::connectCaptureChecker(CaptureChecker* value)
		{
		if (!value)
			return;
		
		connect(value, SIGNAL(capturesChanged(QMap< QString, QString >)), 
				this, SLOT(setCaptures(QMap< QString, QString >)));
		
		connect(this, SIGNAL(capturesChanged(QMap<QString, QString>)), 
				value, SLOT(setCaptures(QMap<QString, QString>)));
		
		connect(value, SIGNAL(variablesChanged(QList< Variable >)),
				this, SLOT(setVariables(QList< Variable >)));
				
		connect(value, SIGNAL(mappingsChanged(QMap< QPair < QString , QString >, int >)),
				this, SLOT(setMappings(QMap< QPair < QString , QString >, int >)));
		}
	
	void CaptureCheckerDetails::setCaptureChecker(CaptureChecker* value)
		{
		captureChecker = value;
		
		connectCaptureChecker(value);
		}
	
	void CaptureCheckerDetails::addNewCapture()
		{
		bool ok = false;
		QString name = KInputDialog::getText(i18n("Add New Capture"), 
					i18n("Please enter the new capture name."),QString(),&ok,this);
		
		if (ok)
			{
			if (captureChecker)
				captureChecker->addNewCapture(name);
			}
		}
	
	void CaptureCheckerDetails::addNewVariable()
		{
		bool ok = false;
		QString name = KInputDialog::getText(i18n("Add New Variable"), 
					i18n("Please enter the new variable name."),QString(),&ok,this);
		
		if (ok)
			{
			if (captureChecker)
				captureChecker->addNewVariable(name);
			}
		}
	
	void CaptureCheckerDetails::verifyMappingInput(int row, int column)
		{
		if (column != MAP_INDEX)
			return;
			
		QString captureName = mappings->item(row, MAP_CAPTURE)->text();
		QString variableName = mappings->item(row, MAP_VARIABLE)->text();
		int index = mappings->item(row, MAP_INDEX)->text().toInt();
		
		captureChecker->setMappingValue(captureName, variableName, index);
		}

	void CaptureCheckerDetails::setCaptures(QMap<QString, QString> value)
		{
		disconnect(captures, SIGNAL(cellChanged(int, int)), this, SLOT(emitCaptures()));
		//thar be bugs in these waters
		captures->clearContents();
		captures->setRowCount(value.count());
		
		QTableWidgetItem *newItem;
		
		QMap<QString, QString>::const_iterator i = value.constBegin();
		int curRow = 0;
		
		while ( i != value.constEnd() )
			{
			newItem = new QTableWidgetItem(i.key());
			captures->setItem(curRow, CAPTURE_NAME, newItem);
			
			newItem = new QTableWidgetItem(i.value());
			captures->setItem(curRow, CAPTURE_VALUE, newItem);
			
			curRow++;
			i++;
			}
			
		connect(captures, SIGNAL(cellChanged(int, int)), this, SLOT(emitCaptures()));
		}
		
	void CaptureCheckerDetails::setVariables(QList<Variable> value)
		{
		disconnect(variables, SIGNAL(cellChanged(int, int)), this, SLOT(emitVariables()));
		variables->clearContents();
		variables->setRowCount(value.count());
		
		QTableWidgetItem *newItem;
		
		for (int i=0; i<value.count(); i++)
			{
			newItem = new QTableWidgetItem(value.at(i).name);
			variables->setItem(i, VARIABLE_NAME, newItem);
			
			newItem = new QTableWidgetItem(value.at(i).min);
			variables->setItem(i, VARIABLE_MIN, newItem);
			
			newItem = new QTableWidgetItem(value.at(i).max);
			variables->setItem(i, VARIABLE_MAX, newItem);
			}
		connect(variables, SIGNAL(cellChanged(int, int)), this, SLOT(emitVariables()));
		}
		
	void CaptureCheckerDetails::setMappings(QMap<QPair<QString, QString>, int> value)
		{
		disconnect(mappings, SIGNAL(cellChanged(int, int)), this, SLOT(verifyMappingInput(int, int)));
		mappings->clearContents();
		mappings->setRowCount(value.count());
		
		QTableWidgetItem *newItem;
		
		QMap<QPair<QString, QString>, int>::const_iterator i = value.constBegin();
		int curRow = 0;
		
		while ( i != value.constEnd() )
			{
			newItem = new QTableWidgetItem(i.key().first);
			newItem->setFlags(Qt::ItemIsEnabled);
			mappings->setItem(curRow, MAP_CAPTURE, newItem);
			
			newItem = new QTableWidgetItem(i.key().second);
			newItem->setFlags(Qt::ItemIsEnabled);
			mappings->setItem(curRow, MAP_VARIABLE, newItem);
			
			newItem = new QTableWidgetItem(QString::number(i.value()));
			mappings->setItem(curRow, MAP_INDEX, newItem);
			
			newItem = new QTableWidgetItem(QString());
			newItem->setFlags(Qt::ItemIsEnabled);
			mappings->setItem(curRow, MAP_TEST, newItem);
			
			curRow++;
			i++;
			}
			
		connect(mappings, SIGNAL(cellChanged(int, int)), this, SLOT(verifyMappingInput(int, int)));
		}
	
	void CaptureCheckerDetails::emitCaptures()
		{
		QMap<QString, QString> value;
		
		for (int i=0; i<captures->rowCount(); i++)
			{
			value.insert(captures->item(i, CAPTURE_NAME)->text(), captures->item(i, CAPTURE_VALUE)->text());
			}
		
		emit capturesChanged(value);
		}
		
	void CaptureCheckerDetails::emitVariables()
		{
		QList<Variable> value;
		
		Variable curVar;
		for (int i=0; i<captures->rowCount(); i++)
			{
			curVar.name = variables->item(i, VARIABLE_NAME)->text();
			curVar.min = variables->item(i, VARIABLE_MIN)->text();
			curVar.max = variables->item(i, VARIABLE_MAX)->text();
			}
		
		emit variablesChanged(value);
		}
		
	}