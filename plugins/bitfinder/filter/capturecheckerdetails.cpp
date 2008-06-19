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

#include <QHeaderView>

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
		captureRemove->setEnabled(false);
		capturesToolbar->addAction(captureAdd);
		capturesToolbar->addAction(captureRemove);
		connect(captureAdd, SIGNAL(triggered( bool )), this, SLOT(addNewCapture()));
		connect(captureRemove, SIGNAL(triggered( bool )), this, SLOT(removeCapture()));

		//variablesToolbar
		variablesToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		variablesToolbar->setOrientation(Qt::Vertical);
		variablesToolbar->setIconDimensions(16);
		variableAdd = new KAction(KIcon("list-add"),i18n("Add Variable"),this);
		variableRemove = new KAction(KIcon("list-remove"),i18n("Remove Variable"),this);
		variableRemove->setEnabled(false);
		variableUp = new KAction(KIcon("arrow-up"), i18n("Move Variable Up"), this);
		variableUp->setEnabled(false);
		variableDown = new KAction(KIcon("arrow-down"), i18n("Move Variable Down"), this);
		variableDown->setEnabled(false);
		variablesToolbar->addAction(variableAdd);
		variablesToolbar->addAction(variableRemove);
		variablesToolbar->addAction(variableUp);
		variablesToolbar->addAction(variableDown);
		connect(variableAdd, SIGNAL(triggered( bool )), this, SLOT(addNewVariable()));
		connect(variableRemove, SIGNAL(triggered( bool )), this, SLOT(removeVariable()));
		connect(variableUp, SIGNAL(triggered( bool )), this, SLOT(moveVariableUp()));
		connect(variableDown, SIGNAL(triggered( bool )), this, SLOT(moveVariableDown()));
		
		//captures
		captures->setColumnCount(2);
		QStringList captureHeaders;
		captureHeaders << "Name" << "Expression";
		captures->setHorizontalHeaderLabels(captureHeaders);
		captures->verticalHeader()->hide();
		connect(captures, SIGNAL(itemSelectionChanged()), this, SLOT(captureSelectionChanged()));
		
		//variables
		variables->setColumnCount(3);
		QStringList variableHeaders;
		variableHeaders << "Name" << "Min" << "Max";
		variables->setHorizontalHeaderLabels(variableHeaders);
		variables->verticalHeader()->hide();
		connect(variables, SIGNAL(itemSelectionChanged()), this, SLOT(variableSelectionChanged()));
		
		//mappings
		mappings->setColumnCount(4);
		QStringList mappingHeaders;
		mappingHeaders << "Capture Name" << "Variable" << "Index" << "Test Value";
		mappings->setHorizontalHeaderLabels(mappingHeaders);
		mappings->verticalHeader()->hide();
		
		}
	
	void CaptureCheckerDetails::resizeColumns()
		{
		if (captures->columnWidth(1) != 65 && captures->width()!=100)
			return;
		
		int captureWidth = captures->width();
		captures->setColumnWidth(CAPTURE_NAME, int(captureWidth * 0.35 - 5));
		captures->setColumnWidth(CAPTURE_VALUE, int(captureWidth * 0.65));
		
		int variableWidth = variables->width();
		variables->setColumnWidth(VARIABLE_NAME, int(variableWidth * 0.5 - 5));
		variables->setColumnWidth(VARIABLE_MIN, int(variableWidth * 0.25));
		variables->setColumnWidth(VARIABLE_MAX, int(variableWidth * 0.25));
		
		int mappingWidth = mappings->width();
		mappings->setColumnWidth(MAP_CAPTURE, int(mappingWidth * 0.2));
		mappings->setColumnWidth(MAP_VARIABLE, int(mappingWidth * 0.2));
		mappings->setColumnWidth(MAP_INDEX, int(mappingWidth * 0.1));
		mappings->setColumnWidth(MAP_TEST, int(mappingWidth * 0.5 - 5));
		}
	
	void CaptureCheckerDetails::connectCaptureChecker(CaptureChecker* value)
		{
		if (!value)
			return;
		
		setCaptures(value->getCaptures());
		setVariables(value->getVariables());
		setMappings(value->getMappings());
		
		connect(value, SIGNAL(capturesChanged(QMap< QString, QString >)), 
				this, SLOT(setCaptures(QMap< QString, QString >)));
		
		connect(this, SIGNAL(capturesChanged(QMap<QString, QString>)), 
				value, SLOT(setCaptures(QMap<QString, QString>)));
		
		connect(value, SIGNAL(variablesChanged(QList< Variable >)),
				this, SLOT(setVariables(QList< Variable >)));
				
		connect(this, SIGNAL(variablesChanged(QList< Variable >)),
				value, SLOT(setVariables(QList< Variable >)));
				
		connect(value, SIGNAL(mappingsChanged(QMap< QPair < QString , QString >, int >)),
				this, SLOT(setMappings(QMap< QPair < QString , QString >, int >)));
		
		//not actually CaptureChecker signals, but should stop data from getting pushed out 
		connect(captures, SIGNAL(cellChanged(int, int)), this, SLOT(emitCaptures()));
		connect(variables, SIGNAL(cellChanged(int, int)), this, SLOT(emitVariables()));
		connect(mappings, SIGNAL(cellChanged(int, int)), this, SLOT(verifyMappingInput(int, int)));

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
	
	void CaptureCheckerDetails::removeCapture()
		{
		disconnect(captureChecker);
		
		QList<QTableWidgetItem *> items = captures->selectedItems();
		
		captureChecker->removeCapture(captures->item(items.at(0)->row(), CAPTURE_NAME)->text());
		
		connectCaptureChecker(captureChecker);
		setCaptures(captureChecker->getCaptures());
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
	
	void CaptureCheckerDetails::removeVariable()
		{
		disconnect(captureChecker);
		
		QList<QTableWidgetItem *> items = variables->selectedItems();
		
		captureChecker->removeVariable(variables->item(items.at(0)->row(), VARIABLE_NAME)->text());
		
		connectCaptureChecker(captureChecker);
		setVariables(captureChecker->getVariables());
		}
	
	void CaptureCheckerDetails::moveVariableUp()
		{
		disconnect(captureChecker);
		
		QList<QTableWidgetItem *> items = variables->selectedItems();
		
		if (!items.count())
			return;
		
		captureChecker->moveVariableUp(items.at(0)->row());
		
		connectCaptureChecker(captureChecker);
		setVariables(captureChecker->getVariables());
		}
		
	void CaptureCheckerDetails::moveVariableDown()
		{
		disconnect(captureChecker);
		
		QList<QTableWidgetItem *> items = variables->selectedItems();
		
		if (!items.count())
			return;
		
		captureChecker->moveVariableDown(items.at(0)->row());
		
		connectCaptureChecker(captureChecker);
		setVariables(captureChecker->getVariables());
		}
	
	void CaptureCheckerDetails::captureSelectionChanged()
		{
		captureRemove->setEnabled(captures->selectedItems().count());
		}
		
	void CaptureCheckerDetails::variableSelectionChanged()
		{
		bool varSel = variables->selectedItems().count();
		variableRemove->setEnabled(varSel);
		
		if (!varSel)
			{
			variableUp->setEnabled(false);
			variableDown->setEnabled(false);
			return;
			}
		
		//at the top it can't shift up at the bottom it can't shift down
		variableUp->setEnabled(variables->selectedItems().at(0)->row() != 0);
		variableDown->setEnabled(variables->selectedItems().at(0)->row() != variables->rowCount()-1);
		
		}
	
	void CaptureCheckerDetails::setTestString(const QString& value)
		{
		testString = value;
		
		updateMappingTest();
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
		QString selectedVariable;
		int selectRow = -1;
		
		if (variables->selectedItems().count())
			{
			selectedVariable = variables->item(variables->selectedItems().at(0)->row(), VARIABLE_NAME)->text();
			}
		
		variables->clearContents();
		variables->setRowCount(value.count());
		
		QTableWidgetItem *newItem;
		
		for (int i=0; i<value.count(); i++)
			{
			if (value.at(i).name == selectedVariable)
				{
				selectRow = i;
				}
			
			newItem = new QTableWidgetItem(value.at(i).name);
			variables->setItem(i, VARIABLE_NAME, newItem);
			
			newItem = new QTableWidgetItem(value.at(i).min);
			variables->setItem(i, VARIABLE_MIN, newItem);
			
			newItem = new QTableWidgetItem(value.at(i).max);
			variables->setItem(i, VARIABLE_MAX, newItem);
			}
		
		if (selectRow >=0)
			{
			variables->selectRow(selectRow);
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
		
		updateMappingTest();
		emit mappingsChanged();
		}
	
	void CaptureCheckerDetails::updateMappingTest()
		{
		//first clear the text on them all
		for (int i=0; i<mappings->rowCount(); i++)
			{
			mappings->item(i, MAP_TEST)->setText(i18n("No Capture"));
			//lazy way to get the default background hehe
			mappings->item(i, MAP_TEST)->setBackground(mappings->item(i, MAP_CAPTURE)->background());
			}
		
		//if there's no test string we can skip doing anything
		if (testString.isEmpty())
			return;
		
		QStringList captureList = captureChecker->getCaptures().keys();
		for (int i=0; i<captureList.count(); i++)
			{
			//first run through each of the captures getting their variable values
			Capture curCap = captureChecker->findCapture(testString, captureList.at(i));
			
			if (curCap.isEmpty())
				continue;
				
			for (int j=0; j<mappings->rowCount(); j++)
				{
				//go through each row on the mapping
				//if we're not looking at the current capture - skip it and move on to the next row
				if (mappings->item(j, MAP_CAPTURE)->text() != captureList.at(i))
					continue;
				
				if (curCap.getValue(mappings->item(j, MAP_VARIABLE)->text()).isEmpty())
					return;
				
				mappings->item(j, MAP_TEST)->setText(curCap.getValue(mappings->item(j, MAP_VARIABLE)->text()));
				
				if (curCap.isInRange(captureChecker->getMinCapture(), captureChecker->getMaxCapture()))
					{
					//this is an in range Match so colour the item green
					mappings->item(j, MAP_TEST)->setBackground(QBrush(QColor(0,255,0,128)));
					}
				else
					{
					//this match is out of range so colour the item red
					mappings->item(j, MAP_TEST)->setBackground(QBrush(QColor(255,0,0,128)));
					}
				}
			
			}
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
		for (int i=0; i<variables->rowCount(); i++)
			{
			curVar.name = variables->item(i, VARIABLE_NAME)->text();
			curVar.min = variables->item(i, VARIABLE_MIN)->text();
			curVar.max = variables->item(i, VARIABLE_MAX)->text();
			
			value.append(curVar);
			}
		
		emit variablesChanged(value);
		}
		
	}
