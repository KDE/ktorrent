## Makefile.am for ktorrent

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = ktorrent



# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir)/. $(all_includes)

# the library search path. 
ktorrent_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# the libraries to link against.
ktorrent_LDADD = ../libtorrent/libtorrent.la $(LIB_KFILE) \
	$(LIB_KDEPRINT)

# which sources should be compiled for ktorrent
ktorrent_SOURCES = settings.kcfgc main.cpp ktorrent.cpp ktorrentview.cpp \
		pref.cpp 		chunkdownloadview.cpp ktorrentviewitem.cpp ktorrentcore.cpp \
		peerview.cpp ktorrentmonitor.cpp 		downloadpref.ui debugview.cpp trayicon.cpp

xdg_apps_DATA = ktorrent.desktop
kde_kcfg_DATA = ktorrent.kcfg

# these are the headers for your project
noinst_HEADERS = ktorrent.h ktorrentview.h pref.h chunkdownloadview.h \
					ktorrentviewitem.h ktorrentcore.h peerview.h ktorrentmonitor.h debugview.h trayicon.h

# client stuff

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/ktorrent.pot

KDE_ICON = ktorrent

# this is where the kdelnk file will go 
kdelnkdir = $(kde_appsdir)/Internet
kdelnk_DATA = ktorrent.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/ktorrent
rc_DATA = ktorrentui.rc


