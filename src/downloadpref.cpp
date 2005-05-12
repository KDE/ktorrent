#include <kdialog.h>
#include <klocale.h>
/****************************************************************************
** Form implementation generated from reading ui file '/home/joris/Projects/ktorrent/src/downloadpref.ui'
**
** Created: Thu May 12 19:25:55 2005
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.4   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "downloadpref.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <knuminput.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a DownloadPref as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
DownloadPref::DownloadPref( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "DownloadPref" );
    DownloadPrefLayout = new QHBoxLayout( this, 11, 6, "DownloadPrefLayout"); 

    layout6 = new QVBoxLayout( 0, 0, 6, "layout6"); 

    layout13 = new QHBoxLayout( 0, 0, 6, "layout13"); 

    layout11 = new QVBoxLayout( 0, 0, 6, "layout11"); 

    textLabel1 = new QLabel( this, "textLabel1" );
    layout11->addWidget( textLabel1 );

    textLabel2 = new QLabel( this, "textLabel2" );
    layout11->addWidget( textLabel2 );

    textLabel3 = new QLabel( this, "textLabel3" );
    layout11->addWidget( textLabel3 );

    textLabel5 = new QLabel( this, "textLabel5" );
    layout11->addWidget( textLabel5 );
    layout13->addLayout( layout11 );

    layout10 = new QVBoxLayout( 0, 0, 6, "layout10"); 

    max_downloads = new KIntNumInput( this, "max_downloads" );
    max_downloads->setValue( 2 );
    max_downloads->setMinValue( 0 );
    layout10->addWidget( max_downloads );

    max_conns = new KIntNumInput( this, "max_conns" );
    max_conns->setValue( 100 );
    max_conns->setMinValue( 0 );
    layout10->addWidget( max_conns );

    max_upload_rate = new KIntNumInput( this, "max_upload_rate" );
    max_upload_rate->setValue( 5 );
    max_upload_rate->setMinValue( 0 );
    layout10->addWidget( max_upload_rate );

    port = new KIntNumInput( this, "port" );
    port->setValue( 6881 );
    port->setMinValue( 1024 );
    port->setMaxValue( 65535 );
    layout10->addWidget( port );
    layout13->addLayout( layout10 );

    layout12 = new QVBoxLayout( 0, 0, 6, "layout12"); 

    textLabel1_2 = new QLabel( this, "textLabel1_2" );
    layout12->addWidget( textLabel1_2 );

    textLabel1_2_2 = new QLabel( this, "textLabel1_2_2" );
    layout12->addWidget( textLabel1_2_2 );

    textLabel4 = new QLabel( this, "textLabel4" );
    layout12->addWidget( textLabel4 );
    spacer4 = new QSpacerItem( 61, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout12->addItem( spacer4 );
    layout13->addLayout( layout12 );
    layout6->addLayout( layout13 );

    keep_seeding = new QCheckBox( this, "keep_seeding" );
    keep_seeding->setChecked( TRUE );
    layout6->addWidget( keep_seeding );

    show_systray_icon = new QCheckBox( this, "show_systray_icon" );
    show_systray_icon->setChecked( TRUE );
    layout6->addWidget( show_systray_icon );
    spacer6 = new QSpacerItem( 20, 57, QSizePolicy::Minimum, QSizePolicy::Expanding );
    layout6->addItem( spacer6 );
    DownloadPrefLayout->addLayout( layout6 );
    languageChange();
    resize( QSize(511, 270).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
DownloadPref::~DownloadPref()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void DownloadPref::languageChange()
{
    setCaption( tr2i18n( "Form1" ) );
    textLabel1->setText( tr2i18n( "Maximum downloads" ) );
    textLabel2->setText( tr2i18n( "Maximum connections per download" ) );
    textLabel3->setText( tr2i18n( "Maximum upload rate" ) );
    textLabel5->setText( tr2i18n( "Port" ) );
    textLabel1_2->setText( tr2i18n( "(0 is no limit)" ) );
    textLabel1_2_2->setText( tr2i18n( "(0 is no limit)" ) );
    textLabel4->setText( tr2i18n( "KB/sec (0 is no limit)" ) );
    keep_seeding->setText( tr2i18n( "&Keep seeding after download is finished" ) );
    keep_seeding->setAccel( QKeySequence( tr2i18n( "Alt+K" ) ) );
    show_systray_icon->setText( tr2i18n( "Show system tray icon" ) );
}

#include "downloadpref.moc"
