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
#ifndef DDOCKWINDOW_H
#define DDOCKWINDOW_H

#include <qdockwindow.h>
#include <qvaluelist.h>

class QBoxLayout;
class QToolButton;
class QWidgetStack;
class QPopupMenu;

class KComboBox;
class KAction;

class DMainWindow;

namespace Ideal {
    class Button;
    class ButtonBar;
}

class DDockWindow : public QDockWindow {
    Q_OBJECT
public:
    enum Position { Bottom, Left, Right };

    DDockWindow(DMainWindow *parent, Position position);
    virtual ~DDockWindow();

    virtual void setVisible(bool v);
    bool visible() const { return m_visible; }
    Position position() const { return m_position; }

    virtual void addWidget(const QString &title, QWidget *widget, bool skipActivation = false);
    virtual void raiseWidget(QWidget *widget);
    virtual void lowerWidget(QWidget *widget);
    /**Removes the widget from dock. Does not delete it.*/
    virtual void removeWidget(QWidget *widget);

    virtual void hideWidget(QWidget *widget);
    virtual void showWidget(QWidget *widget);

    virtual QWidget *currentWidget() const;

    virtual void setMovingEnabled(bool b);

    virtual void saveSettings();

    DMainWindow *mainWindow() const { return m_mainWindow; }

    virtual void selectLastWidget();
    virtual void selectNextWidget();
    virtual void selectPrevWidget();

    bool isActive();
	/// Check if this dock has any widgets
	bool hasWidgets() const; 

signals:
    void hidden();

private slots:
    void selectWidget();
    void selectWidget(Ideal::Button *button);
    void contextMenu(QPopupMenu*);
    void moveToDockLeft();
    void moveToDockRight();
    void moveToDockBottom();
    void moveToDock(DDockWindow::Position);

protected:
    virtual void loadSettings();

    Ideal::ButtonBar *m_bar;
    QWidgetStack *m_widgetStack;

    QMap<Ideal::Button*, QWidget*> m_widgets;
    QMap<QWidget*, Ideal::Button*> m_buttons;

private:
    Position m_position;
    bool m_visible;
    QString m_name;
    DMainWindow *m_mainWindow;
    bool m_doNotCloseActiveWidget;

    Ideal::Button *m_toggledButton;
    Ideal::Button *m_lastContextMenuButton;
    QBoxLayout *m_internalLayout;


    KAction * m_moveToDockLeft;
    KAction * m_moveToDockRight;
    KAction * m_moveToDockBottom;
};

#endif
