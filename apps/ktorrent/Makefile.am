## Makefile.am for ktorrent
SUBDIRS = groups
# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktorrent



# set the include path for X, qt and KDE
INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)

# the library search path. 
ktorrent_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktorrent_LDADD = ../../apps/ktorrent/groups/libgroups.la -lkmdi $(LIB_KPARTS) \
	../../libktorrent/libktorrent.la $(LIB_KFILE) $(LIB_KIO)

# which sources should be compiled for ktorrent
ktorrent_SOURCES = dcopinterface.skel settings.kcfgc main.cpp ktorrent.cpp \
		ktorrentview.cpp pref.cpp ktorrentviewitem.cpp ktorrentcore.cpp downloadpref.ui \
		trayicon.cpp ktorrentdcop.cpp ktorrentapp.cpp torrentcreatordlg.cpp \
		torrentcreatordlg.h torrentcreatordlgbase.ui fileselectdlgbase.ui fileselectdlg.cpp \
		generalpref.ui pastedlgbase.ui pastedialog.cpp leaktrace.cpp queuedlg.ui \
		queuedialog.cpp queuedialog.h scandlgbase.ui scandialog.cpp ipfilterwidgetbase.ui \
		ipfilterwidget.cpp addpeerwidgetbase.ui addpeerwidget.cpp addpeerwidget.h

xdg_apps_DATA = ktorrent.desktop
kde_kcfg_DATA = ktorrent.kcfg

# these are the headers for your project
noinst_HEADERS = ktorrent.h ktorrentview.h pref.h ktorrentviewitem.h \
			ktorrentcore.h trayicon.h dcopinterface.h ktorrentdcop.h fileselectdlg.h pastedialog.h \
		scandialog.h ipfilterwidget.h

# client stuff

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktorrent.pot

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/Internet
kdelnk_DATA = ktorrent.desktop

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
