## Makefile.am for ktorrent
SUBDIRS = groups newui
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktorrent



# set the include path for X, qt and KDE
INCLUDES = -I$(top_builddir)/ktorrent/libktorrent -I$(top_builddir)/libktorrent/ -I$(srcdir)/../../libktorrent \
	$(all_includes)

# the library search path. 
ktorrent_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktorrent_LDADD = ../../apps/ktorrent/newui/libnewui.la $(LIB_KFILE) \
	$(LIB_KIO) $(LIB_KPARTS) ../../apps/ktorrent/groups/libgroups.la \
	../../libktorrent/libktorrent.la

# which sources should be compiled for ktorrent
ktorrent_SOURCES = addpeerwidget.cpp addpeerwidget.h addpeerwidgetbase.ui \
	advancedpref.ui dcopinterface.skel downloadpref.ui fileselectdlg.cpp \
	fileselectdlgbase.ui generalpref.ui ipfilterwidget.cpp ipfilterwidgetbase.ui ktorrent.cpp \
	ktorrentapp.cpp ktorrentcore.cpp ktorrentdcop.cpp ktorrentview.cpp ktorrentviewitem.cpp \
	ktorrentviewmenu.cpp leaktrace.cpp main.cpp pastedialog.cpp pastedlgbase.ui pref.cpp \
	queuedialog.cpp queuedialog.h queuedlg.ui scandialog.cpp scandlgbase.ui \
	speedlimitsdlg.cpp speedlimitsdlgbase.ui torrentcreatordlg.cpp torrentcreatordlg.h \
	torrentcreatordlgbase.ui trayhoverpopup.cpp trayicon.cpp viewmanager.cpp filterbar.cpp

xdg_apps_DATA = ktorrent.desktop


# these are the headers for your project
noinst_HEADERS = dcopinterface.h fileselectdlg.h ipfilterwidget.h ktorrent.h \
	ktorrentcore.h ktorrentdcop.h ktorrentview.h ktorrentviewitem.h ktorrentviewmenu.h \
	pastedialog.h pref.h scandialog.h speedlimitsdlg.h trayhoverpopup.h trayicon.h \
	viewmanager.h

# client stuff

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktorrent.pot

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/ktorrent
rc_DATA = ktorrentui.rc

KDE_ICON= torrent ktorrent

appicondir = $(kde_datadir)/ktorrent/icons
appicon_ICON = ktencrypted ktremove ktstart_all ktstart ktstop_all ktstop ktplugins ktinfowidget ktqueuemanager ktupnp ktprefdownloads

kde_servicetypes_DATA = ktorrentplugin.desktop
EXTRA_DIST = ktorrentplugin.desktop

if ENABLE_TORRENT_MIMETYPE
mimedir = $(kde_mimedir)/application
mime_DATA = x-bittorrent.desktop
endif

KDE_CXXFLAGS = $(USE_EXCEPTIONS) $(USE_RTTI)
