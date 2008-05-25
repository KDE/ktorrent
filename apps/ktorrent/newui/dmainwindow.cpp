/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "dmainwindow.h"

#include <kdebug.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kiconloader.h>

#include <qtoolbutton.h>

#include "dtabwidget.h"
#include "docksplitter.h"

DMainWindow::DMainWindow(QWidget *parent, const char *name)
    :KParts::MainWindow(parent, name), m_firstRemoved(false), m_currentWidget(0)
{
    loadSettings();
    createToolWindows();
    m_central = new Ideal::DockSplitter(Qt::Horizontal, this);
    m_activeTabWidget = createTab();
    m_central->addDock(0, 0, m_activeTabWidget);
    setCentralWidget(m_central);
}

void DMainWindow::loadSettings()
{
    KConfig *config = kapp->config();
    config->setGroup("UI");
    m_openTabAfterCurrent = config->readBoolEntry("OpenNewTabAfterCurrent", true);
    m_showIconsOnTabs = config->readBoolEntry("ShowTabIcons", false);
}

void DMainWindow::saveSettings()
{
    m_leftDock->saveSettings();
    m_rightDock->saveSettings();
    m_bottomDock->saveSettings();
}

DMainWindow::~DMainWindow()
{
/*    for (QValueList<QWidget*>::iterator it = m_widgets.begin(); it != m_widgets.end(); ++it)
        removeWidget(*it);*/
}

DDockWindow *DMainWindow::toolWindow(DDockWindow::Position position) const
{
    switch (position) {
        case DDockWindow::Bottom: return m_bottomDock;
        case DDockWindow::Left: return m_leftDock;
        case DDockWindow::Right: return m_rightDock;
    }
    return 0;
}

void DMainWindow::createToolWindows()
{
    m_bottomDock = new DDockWindow(this, DDockWindow::Bottom);
    moveDockWindow(m_bottomDock, Qt::DockBottom);
    m_leftDock = new DDockWindow(this, DDockWindow::Left);
    moveDockWindow(m_leftDock, Qt::DockLeft);
    m_rightDock = new DDockWindow(this, DDockWindow::Right);
    moveDockWindow(m_rightDock, Qt::DockRight);
    
    // hide all docks until we add stuff to it
    m_bottomDock->hide();
    m_leftDock->hide();
    m_rightDock->hide();
}

void DMainWindow::addWidget(QWidget *widget, const QString &title)
{
//     invalidateActiveTabWidget();
    if (m_firstRemoved && m_activeTabWidget == m_tabs.first())
    {
        m_central->addDock(0, 0, m_activeTabWidget);
        m_firstRemoved = false;
    }

    addWidget(m_activeTabWidget, widget, title);
}

void DMainWindow::addWidget(DTabWidget *tab, QWidget *widget, const QString &title)
{
    static QPixmap emptyPixmap;

    int idx = -1;
    if (m_openTabAfterCurrent && (tab->count() > 0))
        idx = tab->currentPageIndex() + 1;
    if (m_showIconsOnTabs)
    {
        const QPixmap *pixmap = widget->icon();
        const QIconSet &icons = (pixmap && (pixmap->size().height() <= 16)) ? *(pixmap) : SmallIcon("kdevelop");
        tab->insertTab(widget, icons, title, idx);
    }
    else
        tab->insertTab(widget, emptyPixmap, title, idx);
    m_widgets.append(widget);
    m_widgetTabs[widget] = tab;
    widget->installEventFilter(this);
    tab->showPage(widget);
}

void DMainWindow::removeWidget(QWidget *widget)
{
    if (!m_widgets.contains(widget))
        return; //not a widget in main window

    if (m_widgetTabs.contains(widget))
    {
        DTabWidget *tab = m_widgetTabs[widget];
        if (tab->indexOf(widget) >= 0)
        {
            tab->removePage(widget);
            widget->reparent(0,QPoint(0,0),false);
            if (tab->count() == 0)
            {
                if (tab->closeButton())
                    tab->closeButton()->hide();
                //remove and delete tabwidget if it is not the first one
                if (tab != m_tabs.first())
                {
                    QPair<uint, uint> idx = m_central->indexOf(tab);
                    m_tabs.remove(tab);
                    m_activeTabWidget = m_tabs.first();
                    m_central->removeDock(idx.first, idx.second, true);
                }
                //only temporarily remove the first tabwidget
                else
                {
                    m_central->removeDock(0, 0, false);
                    m_firstRemoved = true;
                }
                //focus smth in m_activeTabWidget
                if (m_activeTabWidget)
                {
                    if (m_activeTabWidget->currentPage())
                    {
                        m_activeTabWidget->currentPage()->setFocus();
                    }
                }
            }
        }
    }

    m_widgets.remove(widget);
    m_widgetTabs.remove(widget);
    if (m_activeTabWidget && m_activeTabWidget->currentPage())
    {
        //a hack to please multibuffer and actually switch the active part
        QFocusEvent ev(QEvent::FocusIn);
        QApplication::sendEvent(m_activeTabWidget->currentPage(), &ev);
    }
}

DTabWidget *DMainWindow::splitHorizontal()
{
    m_activeTabWidget = createTab();
    m_central->addDock(m_central->numRows(), 0, m_activeTabWidget);
    return m_activeTabWidget;
}

DTabWidget *DMainWindow::splitVertical()
{
//     invalidateActiveTabWidget();
    int row = m_central->indexOf(m_activeTabWidget).first;
    m_activeTabWidget = createTab();
    m_central->addDock(row, m_central->numCols(row), m_activeTabWidget);
    return m_activeTabWidget;
}

void DMainWindow::invalidateActiveTabWidget()
{
/*    QWidget *focused = m_central->focusWidget();
    kdDebug(9000) << "invalidate: " << focused << endl;
    if (focused == 0)
        return;
    if (!m_widgets.contains(focused))
    {
        kdDebug(9000) << "    focused is not in m_widgets" << endl;
        return;
    }
    if (m_widgetTabs.contains(focused))
    {
        kdDebug(9000) << "    focused is in m_widgets and m_widgetTabs" << endl;
        DTabWidget *tab = m_widgetTabs[focused];
        if (tab->indexOf(focused) >= 0)
            m_activeTabWidget = tab;
        kdDebug(9000) << "    tab: " << tab << endl;
    }*/
}

DTabWidget *DMainWindow::createTab()
{
    DTabWidget *tab = new DTabWidget(m_central);
    m_tabs.append(tab);
    if (tab->closeButton())
        connect(tab->closeButton(), SIGNAL(clicked()), this, SLOT(closeTab()));
    connect(tab, SIGNAL(closeRequest(QWidget*)), this, SLOT(closeTab(QWidget*)));
    connect(tab, SIGNAL(contextMenu(QWidget*,const QPoint &)),
        this, SLOT(tabContext(QWidget*,const QPoint &)));
    return tab;
}

bool DMainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    QWidget *w = (QWidget*)obj;
    if (!m_widgets.contains(w))
        return KParts::MainWindow::eventFilter(obj, ev);

    if ((m_currentWidget != w) && (ev->type() == QEvent::FocusIn))
    {
        m_currentWidget = w;
        emit widgetChanged(w);
    }
    else if (ev->type() == QEvent::IconChange)
    {
        if (m_widgetTabs.contains(w))
        {
            DTabWidget *tab = m_widgetTabs[w];
            tab->setTabIconSet(w, w->icon() ? (*(w->icon())) : QPixmap());
        }
    }
    else if (ev->type() == QEvent::CaptionChange)
    {
        kdDebug(9000) << "caption change" << endl;
        DTabWidget *tab = m_widgetTabs[w];
        tab->changeTab(w, w->caption());
    }

    return KParts::MainWindow::eventFilter(obj, ev);
}

void DMainWindow::closeTab()
{
    //nothing to do here, should be reimplemented
}

void DMainWindow::tabContext(QWidget *, const QPoint &)
{
    //nothing to do here, should be reimplemented
}

void DMainWindow::closeTab(QWidget *)
{
    //nothing to do here, should be reimplemented
}

void DMainWindow::moveWidget(DDockWindow::Position position, QWidget * view, const QString & title)
{
    if (m_docks.contains(view))
    {
        toolWindow(m_docks[view])->removeWidget(view);

        toolWindow(position)->addWidget( title, view, true );
        m_docks[view] = position;
    }
}

void DMainWindow::addDockWidget(DDockWindow::Position position, QWidget *view, const QString &title)
{
    DDockWindow* dock = toolWindow(position);
    dock->addWidget(title, view);
    m_docks[view] = position;
    connect(view, SIGNAL(destroyed()), this, SLOT(widgetDestroyed()));
    if (dock->isHidden())
        dock->show();
}

void DMainWindow::removeDockWidget(QWidget *view)
{
    DDockWindow* dock = toolWindow(m_docks[view]);
    dock->removeWidget(view);
    m_docks.remove(view);
    if (!dock->hasWidgets())
        dock->hide();
}

bool DMainWindow::hasDockWidget(QWidget *view)
{
    return m_docks.contains(view);
}

DDockWindow::Position DMainWindow::dockWidgetPosition(QWidget *view)
{
    return m_docks[view];
}

void DMainWindow::widgetDestroyed()
{
    QWidget *w = static_cast<QWidget*>(const_cast<QObject*>(sender()));

    if (m_docks.contains(w))
    {
        kdError() << "Widget destroyed before being removed from UI!" << endl;
        m_docks.remove(w);
    }
}

#include "dmainwindow.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
