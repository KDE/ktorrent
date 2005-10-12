## Makefile.am for ktorrent

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktorrent



# set the include path for X, qt and KDE
INCLUDES = -I$(srcdir)/../../libktorrent $(all_includes)

# the library search path. 
ktorrent_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktorrent_LDADD = $(LIB_KPARTS) ../../libktorrent/libktorrent.la $(LIB_KFILE) \
	$(LIB_KIO)

# which sources should be compiled for ktorrent
ktorrent_SOURCES = dcopinterface.skel settings.kcfgc main.cpp ktorrent.cpp \
	ktorrentview.cpp pref.cpp chunkdownloadview.cpp ktorrentviewitem.cpp ktorrentcore.cpp \
	peerview.cpp ktorrentmonitor.cpp downloadpref.ui trayicon.cpp \
	logviewer.cpp ktorrentdcop.cpp ktorrentapp.cpp \
	torrentcreatordlg.cpp torrentcreatordlg.h torrentcreatordlgbase.ui infowidgetbase.ui \
	infowidget.cpp chunkbar.cpp availabilitychunkbar.cpp downloadedchunkbar.cpp \
	iwfiletreeitem.cpp fileselectdlgbase.ui fileselectdlg.cpp iwfiletreediritem.cpp \
	generalpref.ui

xdg_apps_DATA = ktorrent.desktop
kde_kcfg_DATA = ktorrent.kcfg

# these are the headers for your project
noinst_HEADERS = ktorrent.h ktorrentview.h pref.h chunkdownloadview.h \
	ktorrentviewitem.h ktorrentcore.h peerview.h ktorrentmonitor.h trayicon.h \
	logviewer.h dcopinterface.h ktorrentdcop.h infowidget.h chunkbar.h \
	availabilitychunkbar.h downloadedchunkbar.h iwfiletreeitem.h fileselectdlg.h \
	iwfiletreediritem.h

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


KDE_ICON = ktorrent
appicondir = $(kde_datadir)/ktorrent/icons
appicon_ICON = AUTO

kde_servicetypes_DATA = ktorrentplugin.desktop
EXTRA_DIST = ktorrentplugin.desktop
