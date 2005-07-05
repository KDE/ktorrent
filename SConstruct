#! /usr/bin/env python

"""
help       -> scons -h
compile    -> scons
clean      -> scons -c
install    -> scons install
uninstall  -> scons -c install
configure  -> scons configure prefix=/tmp/ita debug=full extraincludes=/usr/local/include:/tmp/include prefix=/usr/local

Run from a subdirectory -> scons -u
The variables are saved automatically after the first run (look at cache/kde.cache.py, ..)
"""

###################################################################
# LOAD THE ENVIRONMENT AND SET UP THE TOOLS
###################################################################

## Load the builders in config
env = Environment( tools=['default', 'generic', 'kde'], toolpath=['./' ,'./bksys'])

env.KDEuse("environ rpath")
#env.KDEuse("environ rpath lang_qt thread nohelp")

env['CACHEDIR']=None

ktorrent_sources = """

libutil/array.cpp
libutil/fileops.cpp
libutil/file.cpp
libutil/ptrmap.cpp
libutil/log.cpp
libutil/timer.cpp
libutil/error.cpp
libutil/sha1hash.cpp
libutil/urlencoder.cpp
libutil/sha1hashgen.cpp
libutil/functions.cpp

libtorrent/bencoder.cpp 
libtorrent/value.cpp 
libtorrent/bdecoder.cpp 
libtorrent/bnode.cpp 
libtorrent/torrent.cpp 
libtorrent/chunkmanager.cpp 
libtorrent/chunk.cpp 
libtorrent/peer.cpp 
libtorrent/globals.cpp 
libtorrent/bitset.cpp 
libtorrent/peermanager.cpp 
libtorrent/tracker.cpp 
libtorrent/downloader.cpp 
libtorrent/authenticate.cpp 
libtorrent/uploader.cpp 
libtorrent/torrentcontrol.cpp 
libtorrent/choker.cpp 
libtorrent/chunkdownload.cpp 
libtorrent/speedestimater.cpp 
libtorrent/peerdownloader.cpp 
libtorrent/request.cpp 
libtorrent/piece.cpp 
libtorrent/torrentmonitor.cpp 
libtorrent/peerid.cpp 
libtorrent/announcelist.cpp 
libtorrent/peeruploader.cpp 
libtorrent/uploadcap.cpp
libtorrent/packetreader.cpp 
libtorrent/packetwriter.cpp 
libtorrent/packet.cpp 
libtorrent/httptracker.cpp 
libtorrent/udptracker.cpp 
libtorrent/cache.cpp 
libtorrent/singlefilecache.cpp 
libtorrent/multifilecache.cpp
libtorrent/torrentcreator.cpp 
libtorrent/downloadcap.cpp

src/dcopinterface.skel 
src/settings.kcfgc 
src/main.cpp 
src/ktorrent.cpp 
src/ktorrentview.cpp 
src/pref.cpp 
src/chunkdownloadview.cpp 
src/ktorrentviewitem.cpp 
src/ktorrentcore.cpp
src/peerview.cpp 
src/ktorrentmonitor.cpp 
src/downloadpref.ui 
src/debugview.cpp 
src/trayicon.cpp
src/searchwidget.cpp 
src/searchbar.ui 
src/htmlpart.cpp 
src/logviewer.cpp 
src/ktorrentdcop.cpp
src/ktorrentapp.cpp 
src/torrentcreatordlg.cpp 
src/torrentcreatordlgbase.ui
"""

obj=env.kobject('program')
obj.target='ktorrent'
obj.source=ktorrent_sources
obj.cxxflags='-DQT_THREAD_SUPPORT'
obj.libs='qt-mt kio kdecore khtml'
obj.execute()

#env.KDEaddpaths_libs('#libtorrent/')

env.KDEinstall( 'KDEDATA', 'ktorrent', 'src/ktorrentui.rc' )
env.KDEinstall( 'KDEKCFG', '', 'src/ktorrent.kcfg' )
env.KDEinstall( 'KDEMENU', 'Network', 'src/ktorrent.desktop')
env.KDEicon()


###################################################################
# CONVENIENCE FUNCTIONS TO EMULATE 'make dist' and 'make distclean'
###################################################################

### To make a tarball of your masterpiece, use 'scons dist'
if 'dist' in COMMAND_LINE_TARGETS:

	## The target scons dist requires the python module shutil which is in 2.3
	env.EnsurePythonVersion(2, 3)

	import os
	APPNAME = 'bksys'
	VERSION = os.popen("cat VERSION").read().rstrip()
	FOLDER  = APPNAME+'-'+VERSION
	TMPFOLD = ".tmp"+FOLDER
	ARCHIVE = FOLDER+'.tar.bz2'

	## If your app name and version number are defined in 'version.h', use this instead:
	## (contributed by Dennis Schridde devurandom@gmx@net)
	#import re
	#INFO = dict( re.findall( '(?m)^#define\s+(\w+)\s+(.*)(?<=\S)', open(r"version.h","rb").read() ) )
	#APPNAME = INFO['APPNAME']
	#VERSION = INFO['VERSION']

	import shutil
	import glob

	## check if the temporary directory already exists
	for dir in [FOLDER, TMPFOLD, ARCHIVE]:
		if os.path.isdir(dir):
			shutil.rmtree(dir)

	## create a temporary directory
	startdir = os.getcwd()
	
	os.popen("mkdir -p "+TMPFOLD)	
	os.popen("cp -R * "+TMPFOLD)
	os.popen("mv "+TMPFOLD+" "+FOLDER)

	## remove scons-local if it is unpacked
	os.popen("rm -rf "+FOLDER+"/scons "+FOLDER+"/sconsign "+FOLDER+"/scons-local-0.96.1")

	## remove our object files first
	os.popen("find "+FOLDER+" -name \"*cache*\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \"*.pyc\" | xargs rm -f")

	## CVS cleanup
	os.popen("find "+FOLDER+" -name \"CVS\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \".cvsignore\" | xargs rm -rf")

	## Subversion cleanup
	os.popen("find %s -name .svn -type d | xargs rm -rf" % FOLDER)

	## GNU Arch cleanup
	os.popen("find "+FOLDER+" -name \"{arch}\" | xargs rm -rf")
	os.popen("find "+FOLDER+" -name \".arch-i*\" | xargs rm -rf")

	## Create the tarball (coloured output)
	print "\033[92m"+"Writing archive "+ARCHIVE+"\033[0m"
	os.popen("tar cjf "+ARCHIVE+" "+FOLDER)

	## Remove the temporary directory
	if os.path.isdir(FOLDER):
		shutil.rmtree(FOLDER)

	env.Default(None)
        env.Exit(0)

### Emulate "make distclean"
if 'distclean' in COMMAND_LINE_TARGETS:
	## Remove the cache directory
	import os, shutil
	if os.path.isdir(env['CACHEDIR']):
		shutil.rmtree(env['CACHEDIR'])
	os.popen("find . -name \"*.pyc\" | xargs rm -rf")

	env.Default(None)
	env.Exit(0)

