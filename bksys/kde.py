# Made from scons qt.py and (heavily) modified into kde.py
# Thomas Nagy, 2004, 2005 <tnagy2^8@yahoo.fr>

"""
Run scons -h to display the associated help, or look below ..
"""

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[1m" #"\033[93m" # unreadable on white backgrounds
CYAN   ="\033[96m"
NORMAL ="\033[0m"

import os, re, types
from SCons.Script.SConscript import SConsEnvironment

# Returns the name of the shared object (i.e. libkdeui.so.4)
# referenced by a libtool archive (like libkdeui.la)
def getSOfromLA(lafile):
	contents = open(lafile, 'r').read()
	match = re.search("^dlname='([^']*)'$", contents, re.M)
	if match:
		return match.group(1)
	return None

# A helper, needed .. everywhere
def KDEuse(lenv, flags):
	if lenv['HELP']: lenv.Exit(0)

	_flags=lenv.make_list(flags)
	if 'environ' in _flags:
		## The scons developers advise against using this but it is mostly innocuous :)
		lenv.AppendUnique( ENV = os.environ )
	if not 'lang_qt' in _flags:
		## Use this define if you are using the kde translation scheme (.po files)
		lenv.Append( CPPFLAGS = '-DQT_NO_TRANSLATION' )
	if 'rpath' in _flags:
		## Use this to set rpath - this may cause trouble if folders are moved (chrpath)
		lenv.Append( RPATH = [lenv['QTLIBPATH'], lenv['KDELIBPATH'], lenv['KDEMODULE']] )
		kdelibpaths=[]
		if lenv['KDELIBPATH'] == lenv['KDELIB']:
			kdelibpaths = [lenv['KDELIB']]
		else:
			kdelibpaths = [lenv['KDELIBPATH'],  lenv['KDELIB']]
		lenv.Append( RPATH = [lenv['QTLIBPATH'], lenv['KDEMODULE']]+kdelibpaths )
	if 'thread' in _flags:
		## Uncomment the following if you need threading support
		lenv.KDEaddflags_cxx( ['-DQT_THREAD_SUPPORT', '-D_REENTRANT'] )
	if 'fastmoc' in _flags:
		lenv['BKSYS_FASTMOC']=1
	if not 'nohelp' in _flags:
		if lenv['_CONFIGURE'] or lenv['HELP']:
			lenv.Exit(0)
	if not 'nosmart' or not lenv.has_key('nosmart_includes'):
		lenv.AppendUnique(CPPPATH=['#/'])
		lst=[]
		if lenv.has_key('USE_THE_FORCE_LUKE'):
			lst=lenv['USE_THE_FORCE_LUKE']
			lenv.__delitem__('USE_THE_FORCE_LUKE')
		for v in lst:
			v.execute()
	else:
			lenv['nosmart_includes']=1

	## To use kdDebug(intvalue)<<"some trace"<<endl; you need to define -DDEBUG
	## it is done in admin/generic.py automatically when you do scons configure debug=1

def exists(env):
	return True

def detect_kde(env):
	""" Detect the qt and kde environment using kde-config mostly """
        def getpath(varname):
                if not env.has_key('ARGS'): return None
                v=env['ARGS'].get(varname, None)
                if v: v=os.path.abspath(v)
                return v

        prefix      = getpath('prefix')
        execprefix  = getpath('execprefix')
        datadir     = getpath('datadir')
        libdir      = getpath('libdir')
        kdeincludes = getpath('kdeincludes')
        kdelibs     = getpath('kdelibs')
        qtincludes  = getpath('qtincludes')
        qtlibs      = getpath('qtlibs')
        libsuffix   = ''
        if env.has_key('ARGS'): libsuffix=env['ARGS'].get('libsuffix', '')

	if libdir: libdir = libdir+libsuffix

	## Detect the kde libraries
	print "Checking for kde-config           : ",
	kde_config = os.popen("which kde-config 2>/dev/null").read().strip()
	if len(kde_config):
		print GREEN+"kde-config was found"+NORMAL
	else:
		print RED+"kde-config was NOT found in your PATH"+NORMAL
		print "Make sure kde is installed properly"
		print "(missing package kdebase-devel?)"
		env.Exit(1)
	env['KDEDIR'] = os.popen('kde-config -prefix').read().strip()

	print "Checking for kde version          : ",
	kde_version = os.popen("kde-config --version|grep KDE").read().strip().split()[1]
	if int(kde_version[0]) != 3 or int(kde_version[2]) < 2:
		print RED+kde_version
		print RED+"Your kde version can be too old"+NORMAL
		print RED+"Please make sure kde is at least 3.2"+NORMAL
	else:
		print GREEN+kde_version+NORMAL

	## Detect the qt library
	print "Checking for the qt library       : ",
	qtdir = os.getenv("QTDIR")
	if qtdir:
		print GREEN+"qt is in "+qtdir+NORMAL
	else:
		try:
			tmplibdir = os.popen('kde-config --expandvars --install lib').read().strip()
			libkdeuiSO = tmplibdir+'/'+getSOfromLA(tmplibdir+'/libkdeui.la')
			m = re.search('(.*)/lib/libqt.*', os.popen('ldd ' + libkdeuiSO + ' | grep libqt').read().strip().split()[2])
		except:
			m=None
		if m:
			qtdir = m.group(1)
			print YELLOW+"qt was found as "+m.group(1)+NORMAL
		else:
			print RED+"qt was not found"+NORMAL
			print RED+"Please set QTDIR first (/usr/lib/qt3?) or try scons -h for more options"+NORMAL
			env.Exit(1)
	env['QTDIR'] = qtdir.strip()

	## Find the necessary programs uic and moc
	print "Checking for uic                  : ",
	uic = qtdir + "/bin/uic"
	if os.path.isfile(uic):
		print GREEN+"uic was found as "+uic+NORMAL
	else:
		uic = os.popen("which uic 2>/dev/null").read().strip()
		if len(uic):
			print YELLOW+"uic was found as "+uic+NORMAL
		else:
			uic = os.popen("which uic 2>/dev/null").read().strip()
			if len(uic):
				print YELLOW+"uic was found as "+uic+NORMAL
			else:
				print RED+"uic was not found - set QTDIR put it in your PATH ?"+NORMAL
				env.Exit(1)
	env['QT_UIC'] = uic

	print "Checking for moc                  : ",
	moc = qtdir + "/bin/moc"
	if os.path.isfile(moc):
		print GREEN + "moc was found as " + moc + NORMAL
	else:
		moc = os.popen("which moc 2>/dev/null").read().strip()
		if len(moc):
			print YELLOW + "moc was found as " + moc + NORMAL
		elif os.path.isfile("/usr/share/qt3/bin/moc"):
			moc = "/usr/share/qt3/bin/moc"
			print YELLOW + "moc was found as " + moc + NORMAL
		else:
			print RED + "moc was not found - set QTDIR or put it in your PATH ?" + NORMAL
			env.Exit(1)
	env['QT_MOC'] = moc

	## check for the qt and kde includes
	print "Checking for the qt includes      : ",
	if qtincludes and os.path.isfile(qtincludes + "/qlayout.h"):
		# The user told where to look for and it looks valid
		print GREEN + "ok " + qtincludes + NORMAL
	else:
		if os.path.isfile(qtdir + "/include/qlayout.h"):
			# Automatic detection
			print GREEN + "ok " + qtdir + "/include/ " + NORMAL
			qtincludes = qtdir + "/include/"
		elif os.path.isfile("/usr/include/qt3/qlayout.h"):
			# Debian probably
			print YELLOW + "the qt headers were found in /usr/include/qt3/ " + NORMAL
			qtincludes = "/usr/include/qt3"
		else:
			print RED + "the qt headers were not found" + NORMAL
			env.Exit(1)

	print "Checking for the kde includes     : ",
	kdeprefix = os.popen("kde-config --prefix").read().strip()
	if not kdeincludes:
		kdeincludes = kdeprefix+"/include/"
	if os.path.isfile(kdeincludes + "/klineedit.h"):
		print GREEN + "ok " + kdeincludes + NORMAL
	else:
		if os.path.isfile(kdeprefix+"/include/kde/klineedit.h"):
			# Debian, Fedora probably
			print YELLOW + "the kde headers were found in " + kdeprefix + "/include/kde/" + NORMAL
			kdeincludes = kdeprefix + "/include/kde/"
		else:
			print RED + "The kde includes were NOT found" + NORMAL
			env.Exit(1)

	# kde-config options
	kdec_opts = {'KDEBIN'    : 'exe',     'KDEAPPS'      : 'apps',
		     'KDEDATA'   : 'data',    'KDEICONS'     : 'icon',
		     'KDEMODULE' : 'module',  'KDELOCALE'    : 'locale',
		     'KDEKCFG'   : 'kcfg',    'KDEDOC'       : 'html',
		     'KDEMENU'   : 'apps',    'KDEXDG'       : 'xdgdata-apps',
		     'KDEMIME'   : 'mime',    'KDEXDGDIR'    : 'xdgdata-dirs',
		     'KDESERV'   : 'services','KDESERVTYPES' : 'servicetypes',
		     'KDEINCLUDE': 'include'
		     }

	if prefix:
		## use the user-specified prefix
		if not execprefix:
			execprefix = prefix
		if not datadir:
			datadir=prefix+"/share"
		if not libdir:
			libdir=execprefix+"/lib"+libsuffix

		subst_vars = lambda x: x.replace('${exec_prefix}', execprefix)\
			     .replace('${datadir}', datadir)\
			     .replace('${libdir}', libdir)
		debian_fix = lambda x: x.replace('/usr/share', '${datadir}')
		env['PREFIX'] = prefix
		env['KDELIB'] = libdir
		for (var, option) in kdec_opts.items():
			dir = os.popen('kde-config --install ' + option).read().strip()
			if var == 'KDEDOC': dir = debian_fix(dir)
			env[var] = subst_vars(dir)

	else:
		env['PREFIX'] = os.popen('kde-config --expandvars --prefix').read().strip()
		env['KDELIB'] = os.popen('kde-config --expandvars --install lib').read().strip()
		for (var, option) in kdec_opts.items():
			dir = os.popen('kde-config --expandvars --install ' + option).read().strip()
			env[var] = dir

	env['QTPLUGINS']=os.popen('kde-config --expandvars --install qtplugins').read().strip()

	## kde libs and includes
	env['KDEINCLUDEPATH']=kdeincludes
	if not kdelibs:
		kdelibs=os.popen('kde-config --expandvars --install lib').read().strip()
	env['KDELIBPATH']=kdelibs

	## qt libs and includes
	env['QTINCLUDEPATH']=qtincludes
	if not qtlibs:
		qtlibs=qtdir+"/lib"+libsuffix
	env['QTLIBPATH']=qtlibs

def generate(env):
	""""Set up the qt and kde environment and builders - the moc part is difficult to understand """

	# attach this function immediately
	SConsEnvironment.KDEuse = KDEuse

	if env['HELP']:
		print """\033[1m*** KDE options ***
-------------------\033[0m
\033[1m* prefix     \033[0m: base install path,         ie: /usr/local
\033[1m* execprefix \033[0m: install path for binaries, ie: /usr/bin
\033[1m* datadir    \033[0m: install path for the data, ie: /usr/local/share
\033[1m* libdir     \033[0m: install path for the libs, ie: /usr/lib
\033[1m* libsuffix  \033[0m: suffix of libraries on amd64, ie: 64, 32
\033[1m* kdeincludes\033[0m: path to the kde includes (/usr/include/kde on debian, ...)
\033[1m* qtincludes \033[0m: same punishment, for qt includes (/usr/include/qt on debian, ...)
\033[1m* kdelibs    \033[0m: path to the kde libs, for linking the programs
\033[1m* qtlibs     \033[0m: same punishment, for qt libraries
ie: \033[1mscons configure libdir=/usr/local/lib qtincludes=/usr/include/qt\033[0m
"""
		return

	import SCons.Defaults
	import SCons.Tool
	import SCons.Util
	import SCons.Node

	CLVar = SCons.Util.CLVar
	splitext = SCons.Util.splitext
	Builder = SCons.Builder.Builder
	
	# Detect the environment - replaces ./configure implicitely and store the options into a cache
	from SCons.Options import Options
	cachefile=env['CACHEDIR']+'kde.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		('PREFIX', 'root of the program installation'),

		('QTDIR', ''),
		('QTLIBPATH', 'path to the qt libraries'),
		('QTINCLUDEPATH', 'path to the qt includes'),
		('QT_UIC', 'uic command'),
		('QT_MOC', 'moc command'),
		('QTPLUGINS', 'uic executable command'),

		('KDEDIR', ''),
		('KDELIBPATH', 'path to the installed kde libs'),
		('KDEINCLUDEPATH', 'path to the installed kde includes'),

		('KDEBIN', 'inst path of the kde binaries'),
		('KDEINCLUDE', 'inst path of the kde include files'),
		('KDELIB', 'inst path of the kde libraries'),
		('KDEMODULE', 'inst path of the parts and libs'),
		('KDEDATA', 'inst path of the application data'),
		('KDELOCALE', ''), ('KDEDOC', ''), ('KDEKCFG', ''),
		('KDEXDG', ''), ('KDEXDGDIR', ''), ('KDEMENU', ''),
		('KDEMIME', ''), ('KDEICONS', ''), ('KDESERV', ''),
		('KDESERVTYPES', ''), ('KDEAPPS', ''),
	)
	opts.Update(env)

	def getInstDirForResType(lenv,restype):
		if len(restype) == 0 or not lenv.has_key(restype):
			print RED+"unknown resource type "+restype+NORMAL
			lenv.Exit(1)
		else:
			instdir = lenv[restype]
		basedir=lenv['DESTDIR']
		## support for installing into other folders when PREFIX is set - used by gnu stow
		if basedir: instdir = instdir.replace(lenv['PREFIX'], basedir)
		return instdir

	# reconfigure when things are missing
	if not env['HELP'] and (env['_CONFIGURE'] or not env.has_key('QTDIR') or not env.has_key('KDEDIR')):
		detect_kde(env)
		opts.Save(cachefile, env)

	## set default variables, one can override them in sconscript files
	env.Append(CXXFLAGS = ['-I'+env['KDEINCLUDEPATH'], '-I'+env['QTINCLUDEPATH'] ],
			LIBPATH = [env['KDELIBPATH'], env['QTLIBPATH'] ])
	
	env['QT_AUTOSCAN'] = 1
	env['QT_DEBUG']    = 0

	env['MEINPROC'] = 'meinproc'
	env['MSGFMT']   = 'msgfmt'

	## ui file processing
	def uic_processing(target, source, env):
		inc_kde  ='#include <klocale.h>\n#include <kdialog.h>\n'
		inc_moc  ='#include "%s"\n' % target[2].name
		comp_h   ='$QT_UIC -L $QTPLUGINS -nounload -o %s %s' % (target[0].path, source[0].path)
		comp_c   ='$QT_UIC -L $QTPLUGINS -nounload -tr tr2i18n -impl %s %s' % (target[0].path, source[0].path)
		comp_moc ='$QT_MOC -o %s %s' % (target[2].path, target[0].path)
		if env.Execute(comp_h):
			return ret
		dest = open( target[1].path, "w" )
		dest.write(inc_kde)
		dest.close()
		if env.Execute( comp_c+" >> "+target[1].path ):
			return ret
		dest = open( target[1].path, "a" )
		dest.write(inc_moc)
		dest.close()
		ret = env.Execute( comp_moc )
		return ret
	def uicEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		target.append(bs+'.cpp')
		target.append(bs+'.moc')
		return target, source
	env['BUILDERS']['Uic']=Builder(action=uic_processing,emitter=uicEmitter,suffix='.h',src_suffix='.ui')

	def kcfg_buildit(target, source, env):
		comp='kconfig_compiler -d%s %s %s' % (str(source[0].get_dir()), source[1].path, source[0].path)
		return env.Execute(comp)
	def kcfg_stringit(target, source, env):
		print "processing %s to get %s and %s" % (source[0].name, target[0].name, target[1].name)
	def kcfgEmitter(target, source, env):
		adjustixes = SCons.Util.adjustixes
		bs = SCons.Util.splitext(str(source[0].name))[0]
		bs = os.path.join(str(target[0].get_dir()),bs)
		# .h file is already there
		target.append(bs+'.cpp')

		if not os.path.isfile(str(source[0])):
			print RED+'kcfg file given '+str(source[0])+' does not exist !'+NORMAL
			print os.popen('pwd').read()
			return target, source
		kfcgfilename=""
		kcfgFileDeclRx = re.compile("^[fF]ile\s*=\s*(.+)\s*$")
		for line in file(str(source[0]), "r").readlines():
			match = kcfgFileDeclRx.match(line.strip())
			if match:
				kcfgfilename = match.group(1)
				break
		if not kcfgfilename:
			print 'invalid kcfgc file'
			return 0
		source.append(str(source[0].get_dir())+'/'+kcfgfilename)
		return target, source

	env['BUILDERS']['Kcfg']=Builder(action=env.Action(kcfg_buildit, kcfg_stringit),
			emitter=kcfgEmitter, suffix='.h', src_suffix='.kcfgc')
	
	## MOC processing
	env['BUILDERS']['Moc']=Builder(action='$QT_MOC -o $TARGET $SOURCE',suffix='.moc',src_suffix='.h')
	env['BUILDERS']['Moccpp']=Builder(action='$QT_MOC -o $TARGET $SOURCE',suffix='_moc.cpp',src_suffix='.h')

	## KIDL file
	env['BUILDERS']['Kidl']=Builder(action= 'dcopidl $SOURCE > $TARGET || (rm -f $TARGET ; false)',
			suffix='.kidl', src_suffix='.h')
	## DCOP
	env['BUILDERS']['Dcop']=Builder(action='dcopidl2cpp --c++-suffix cpp --no-signals --no-stub $SOURCE',
			suffix='_skel.cpp', src_suffix='.kidl')
	## STUB
	env['BUILDERS']['Stub']=Builder(action= 'dcopidl2cpp --c++-suffix cpp --no-signals --no-skel $SOURCE',
			suffix='_stub.cpp', src_suffix='.kidl')
	## DOCUMENTATION
	env['BUILDERS']['Meinproc']=Builder(action='$MEINPROC --check --cache $TARGET $SOURCE',suffix='.cache.bz2')
	## TRANSLATIONS
	env['BUILDERS']['Transfiles']=Builder(action='$MSGFMT $SOURCE -o $TARGET',suffix='.gmo',src_suffix='.po')

	## Handy helpers for building kde programs
	## You should not have to modify them ..

	ui_ext = [".ui"]
	kcfg_ext = ['.kcfgc']
	header_ext = [".h", ".hxx", ".hpp", ".hh"]
	cpp_ext = [".cpp", ".cxx", ".cc"]
	skel_ext = [".skel", ".SKEL"]
	stub_ext = [".stub", ".STUB"]

	def KDEfiles(lenv, target, source):
		""" Returns a list of files for scons (handles kde tricks like .skel) 
		It also makes custom checks against double includes like : ['file.ui', 'file.cpp']
		(file.cpp is already included because of file.ui) """

		q_object_search = re.compile(r'[^A-Za-z0-9]Q_OBJECT[^A-Za-z0-9]')
		def scan_moc(bs, file_cpp):
			addfile=None
			# try to find the header
			h_ext=''
			for n_h_ext in header_ext:
				if os.path.isfile(bs+n_h_ext):
					h_ext=n_h_ext
					break
			# We have the header corresponding to the cpp file
			if h_ext:
				needscan=0
				# User asked for fastmoc, try to avoid scanning
				if env.has_key('BKSYS_FASTMOC'):
					if os.path.isfile(bs+'.moc'):
						lenv.Moc(bs+h_ext)
					elif os.path.isfile(bs+'_moc.cpp'):
						lenv.Moccpp(bs+h_ext)
						addfile=bs+'_moc.cpp'
					else:
						#print "need scanning "+os.getcwd()+'/'+bs+".moc"
						needscan=1
				else:
					needscan=1
				# We cannot avoid scanning the files ...
				if needscan:
					file_h=bs+h_ext
					h_contents = open(file_h, 'rb').read()
					if q_object_search.search(h_contents):
						# we know now there is Q_OBJECT macro
						lst = bs.split('/')
						val = lst[ len(lst) - 1 ]
						reg = '\n\s*#include\s*("|<)'+val+'.moc("|>)'
						meta_object_search = re.compile(reg)
						cpp_contents = open(file_cpp, 'rb').read()
						if meta_object_search.search(cpp_contents):
							lenv.Moc(file_h)
						else:
							lenv.Moccpp(file_h)
							addfile=bs+'_moc.cpp'
							print "WARNING: moc.cpp for "+bs+h_ext+" consider using #include <file.moc> instead"
			return addfile

		src=[]
		ui_files=[]
		kcfg_files=[]
		other_files=[]
		kidl=[]

		source_=lenv.make_list(source)

		# For each file, check wether it is a dcop file or not, and create the complete list of sources
		for file in source_:
			bs  = SCons.Util.splitext(file)[0]
			ext = SCons.Util.splitext(file)[1]
			if ext in skel_ext:
				if not bs in kidl:
					kidl.append(bs)
				lenv.Dcop(bs+'.kidl')
				src.append(bs+'_skel.cpp')
			elif ext in stub_ext:
				if not bs in kidl:
					kidl.append(bs)
				lenv.Stub(bs+'.kidl')
				src.append(bs+'_stub.cpp')
			elif ext == ".moch":
				lenv.Moccpp(bs+'.h')
				src.append(bs+'_moc.cpp')
			elif ext in cpp_ext:
				src.append(file)
				if not env.has_key('NOMOCFILE'):
					ret = scan_moc(bs, file)
					if ret:
						src.append( ret )
			elif ext in ui_ext:
				lenv.Uic(file)
				src.append(bs+'.cpp')
			elif ext in kcfg_ext:
				lenv.Kcfg(file)
				src.append(bs+'.cpp')
			else:
				src.append(file)

		for base in kidl:
			lenv.Kidl(base+'.h')
		
		# Now check against typical newbie errors
		for file in ui_files:
			for ofile in other_files:
				if ofile == file:
					print RED+"WARNING: You have included "+file+".ui and another file of the same prefix"+NORMAL
					print "Files generated by uic (file.h, file.cpp must not be included"
		for file in kcfg_files:
			for ofile in other_files:
				if ofile == file:
					print RED+"WARNING: You have included "+file+".kcfg and another file of the same prefix"+NORMAL
					print "Files generated by kconfig_compiler (settings.h, settings.cpp) must not be included"
		return src


	""" In the future, these functions will contain the code that will dump the
	configuration for re-use from an IDE """
	import glob
	def KDEinstall(lenv, restype, subdir, files):
		if env.has_key('DUMPCONFIG'):
			print "<install type=\"%s\" subdir=\"%s\">" % (restype, subdir)
			for i in lenv.make_list(files):
				print "    <file name=\"%s\"/>" % i
			print "</install>"
			return

		if not env['_INSTALL']:
			return
		dir = getInstDirForResType(lenv, restype)
		install_list = lenv.bksys_install(dir+'/'+subdir, files, nodestdir=1)
		return install_list

	def KDEinstallas(lenv, restype, destfile, file):
		if not env['_INSTALL']:
			return
		dir = getInstDirForResType(lenv, restype)
		install_list = lenv.InstallAs(dir+'/'+destfile, file)
                env.Alias('install', install_list)
		return install_list

	def KDEprogram(lenv, target, source, 
			includes='', localshlibs='', globallibs='', globalcxxflags=''):
		""" Makes a kde program 
		The program is installed except if one sets env['NOAUTOINSTALL'] """
		src = KDEfiles(lenv, target, source)
		program_list = lenv.Program(target, src)

		# we link the program against a shared library done locally, add the dependency
		if not lenv.has_key('nosmart_includes'):
			lenv.AppendUnique(CPPPATH=['./'])
		if len(localshlibs)>0:
			lst=lenv.make_list(localshlibs)
			lenv.link_local_shlib(lst)
			lenv.Depends( program_list, lst )
			
		if len(includes)>0:
			lenv.KDEaddpaths_includes(includes)
		if len(globallibs)>0:
			lenv.KDEaddlibs(globallibs)
		if len(globalcxxflags)>0:
			lenv.KDEaddflags_cxx(globalcxxflags)
		
		if not lenv.has_key('NOAUTOINSTALL'):
			KDEinstall(lenv, 'KDEBIN', '', target)
		return program_list

	def KDEshlib(lenv, target, source, kdelib=0, libprefix='lib', 
			includes='', localshlibs='', globallibs='', globalcxxflags='', vnum=''):
		""" Makes a shared library for kde (.la file for klibloader)
		The library is installed except if one sets env['NOAUTOINSTALL'] """
		src = KDEfiles(lenv, target, source)

		if not lenv.has_key('nosmart_includes'):
			lenv.AppendUnique(CPPPATH=['./'])
		# we link the program against a shared library done locally, add the dependency
		lst=[]
		if len(localshlibs)>0:
			lst=lenv.make_list(localshlibs)
			lenv.link_local_shlib(lst)
		if len(includes)>0:
			lenv.KDEaddpaths_includes(includes)
		if len(globallibs)>0:
			lenv.KDEaddlibs(globallibs)
		if len(globalcxxflags)>0:
			lenv.KDEaddflags_cxx(globalcxxflags)

		restype = 'KDEMODULE'
		if kdelib==1:
			restype = 'KDELIB'

		library_list = lenv.bksys_shlib(target, src, getInstDirForResType(lenv, restype), libprefix, vnum, nodestdir=1)
		if len(lst)>0: lenv.Depends( library_list, lst )

		return library_list

	def KDEstaticlib(lenv, target, source):
		""" Makes a static library for kde - in practice you should not use static libraries 
		1. they take more memory than shared ones
		2. makefile.am needed it because of limitations
		(cannot handle sources in separate folders - takes extra processing) """
		if not lenv.has_key('nosmart_includes'):
			lenv.AppendUnique(CPPPATH=['./'])
		src = KDEfiles(lenv, target, source)
		return lenv.StaticLibrary(target, src)
		# do not install static libraries by default

	def KDEaddflags_cxx(lenv, fl):
		""" Compilation flags for C++ programs """
		lenv.AppendUnique(CXXFLAGS = lenv.make_list(fl))
	
	def KDEaddflags_c(lenv, fl):
		""" Compilation flags for C programs """
		lenv.AppendUnique(CFLAGS = lenv.make_list(fl))

	def KDEaddflags_link(lenv, fl):
		""" Add link flags - Use this if KDEaddlibs below is not enough """
		lenv.PrependUnique(LINKFLAGS = lenv.make_list(fl))

	def KDEaddlibs(lenv, libs):
		""" Helper function """
		lenv.AppendUnique(LIBS = lenv.make_list(libs))

	def KDEaddpaths_includes(lenv, paths):
		""" Add new include paths """
		lenv.AppendUnique(CPPPATH = lenv.make_list(paths))

	def KDEaddpaths_libs(lenv, paths):
		""" Add paths to libraries """
		lenv.PrependUnique(LIBPATH = lenv.make_list(paths))

	def KDElang(lenv, folder, appname):
		""" Process translations (.po files) in a po/ dir """
		transfiles = glob.glob(folder+'/*.po')
		for lang in transfiles:
			result = lenv.Transfiles(lang)
			country = SCons.Util.splitext(result[0].name)[0]
			KDEinstallas(lenv, 'KDELOCALE', country+'/LC_MESSAGES/'+appname+'.mo', result)

	def KDEicon(lenv, icname='*', path='./', restype='KDEICONS', subdir=''):
		"""Contributed by: "Andrey Golovizin" <grooz()gorodok()net>
		modified by "Martin Ellis" <m.a.ellis()ncl()ac()uk>

		Installs icons with filenames such as cr22-action-frame.png into 
		KDE icon hierachy with names like icons/crystalsvg/22x22/actions/frame.png.
		
		Global KDE icons can be installed simply using env.KDEicon('name').
		The second parameter, path, is optional, and specifies the icons
		location in the source, relative to the SConscript file.

		To install icons that need to go under an applications directory (to
		avoid name conflicts, for example), use e.g.
		env.KDEicon('name', './', 'KDEDATA', 'appname/icons')"""

		if env.has_key('DUMPCONFIG'):
			print "<icondirent subdir=\"%s\">" % (path+subdir)
			return

		type_dic = { 'action' : 'actions', 'app' : 'apps', 'device' : 
			'devices', 'filesys' : 'filesystems', 'mime' : 'mimetypes' } 
		dir_dic = {
		'los'  :'locolor/16x16',
		'lom'  :'locolor/32x32',
		'him'  :'hicolor/32x32',
		'hil'  :'hicolor/48x48',
		'lo16' :'locolor/16x16',
		'lo22' :'locolor/22x22',
		'lo32' :'locolor/32x32',
		'hi16' :'hicolor/16x16',
		'hi22' :'hicolor/22x22',
		'hi32' :'hicolor/32x32',
		'hi48' :'hicolor/48x48',
		'hi64' :'hicolor/64x64',
		'hi128':'hicolor/128x128',
		'hisc' :'hicolor/scalable',
		'cr16' :'crystalsvg/16x16',
		'cr22' :'crystalsvg/22x22',
		'cr32' :'crystalsvg/32x32',
		'cr48' :'crystalsvg/48x48',
		'cr64' :'crystalsvg/64x64',
		'cr128':'crystalsvg/128x128',
		'crsc' :'crystalsvg/scalable'
		}

		iconfiles = []
		for ext in "png xpm mng svg svgz".split():
			files = glob.glob(path+'/'+'*-*-%s.%s' % (icname, ext))
			iconfiles += files
		for iconfile in iconfiles:
			lst = iconfile.split('/')
			filename = lst[ len(lst) - 1 ]
			tmp = filename.split('-')
			if len(tmp)!=3:
				print RED+'WARNING: icon filename has unknown format: '+iconfile+NORMAL
				continue
			[icon_dir, icon_type, icon_filename]=tmp
			try:
				basedir=getInstDirForResType(lenv, restype)
				destfile = '%s/%s/%s/%s/%s' % (basedir, subdir, dir_dic[icon_dir], type_dic[icon_type], icon_filename)
			except KeyError:
				print RED+'WARNING: unknown icon type: '+iconfile+NORMAL
				continue
			## Do not use KDEinstallas here, as parsing from an ide will be necessary
			if env['_INSTALL']: 
				env.Alias('install', env.InstallAs( destfile, iconfile ) )

        ## This function uses env imported above
        def docfolder(lenv, folder, lang, destination=""):
                # folder is the folder to process
                # lang is the language
                # destination is the subdirectory in KDEDOC
                docfiles = glob.glob(folder+"/???*.*") # file files that are at least 4 chars wide :)
                # warn about errors
                #if len(lang) != 2:
                #       print "error, lang must be a two-letter string, like 'en'"

                # when the destination is not given, use the folder
                if len(destination) == 0:
                        destination=folder
                docbook_list = []
                for file in docfiles:
                        # do not process folders
                        if not os.path.isfile(file):
                                continue
                        # do not process the cache file
                        if file == 'index.cache.bz2':
                                continue
                        # ignore invalid files (TODO??)
                        if len( SCons.Util.splitext( file ) ) <= 1 :
                                continue

                        ext = SCons.Util.splitext( file )[1]

                        # docbook files are processed by meinproc
                        if ext != '.docbook':
				continue
                        docbook_list.append( file )
                        lenv.KDEinstall('KDEDOC', lang+'/'+destination, file)
                # Now process the index.docbook files ..
                if len(docbook_list) == 0:
                        return
                if not os.path.isfile( folder+'/index.docbook' ):
                        print "Error, index.docbook was not found in "+folder+'/index.docbook'
                        return
	        ## Define this to 1 if you are writing documentation else to 0 :)
                if env.has_key('i_am_a_documentation_writer'):
                        for file in docbook_list:
                                lenv.Depends( folder+'index.cache.bz2', file )
                lenv.Meinproc( folder+'/index.cache.bz2', folder+'/index.docbook' )
                lenv.KDEinstall( 'KDEDOC', lang+'/'+destination, folder+'/index.cache.bz2' )

	#valid_targets = "program shlib kioslave staticlib".split()
	import generic
	class kobject(generic.genobj):
		def __init__(self, val, senv=None):
			if senv: generic.genobj.__init__(self, val, senv)
			else: generic.genobj.__init__(self, val, env)
			self.iskdelib=0
		def it_is_a_kdelib(self): self.iskdelib=1
		def execute(self):
			self.lockchdir()
			if self.orenv.has_key('DUMPCONFIG'):
				print self.xml()
				return
			if (self.type=='shlib' or self.type=='kioslave'):
				install_dir = 'KDEMODULE'
				if self.iskdelib==1: install_dir = 'KDELIB'
				self.instdir=getInstDirForResType(self.orenv, install_dir)
				self.nodestdir=1
			elif self.type=='program':
				self.instdir=getInstDirForResType(self.orenv, 'KDEBIN')
				self.nodestdir=1

			self.src=KDEfiles(env, self.target, self.source)
			generic.genobj.execute(self)
			self.unlockchdir()

                def xml(self):
                        ret= '<compile type="%s" chdir="%s" target="%s" cxxflags="%s" cflags="%s" includes="%s" linkflags="%s" libpaths="%s" libs="%s" vnum="%s" iskdelib="%s" libprefix="%s">\n' % (self.type, self.chdir, self.target, self.cxxflags, self.cflags, self.includes, self.linkflags, self.libpaths, self.libs, self.vnum, self.iskdelib, self.libprefix)
                        if self.source:
                                for i in self.orenv.make_list(self.source):
                                        ret += '  <source file="%s"/>\n' % i
                        ret += "</compile>"
                        return ret

	# Attach the functions to the environment so that SConscripts can use them
	SConsEnvironment.KDEprogram = KDEprogram
	SConsEnvironment.KDEshlib = KDEshlib
	SConsEnvironment.KDEstaticlib = KDEstaticlib
	SConsEnvironment.KDEinstall = KDEinstall
	SConsEnvironment.KDEinstallas = KDEinstallas
	SConsEnvironment.KDElang = KDElang
	SConsEnvironment.KDEicon = KDEicon

	SConsEnvironment.KDEaddflags_cxx = KDEaddflags_cxx
	SConsEnvironment.KDEaddflags_c = KDEaddflags_c
	SConsEnvironment.KDEaddflags_link = KDEaddflags_link
	SConsEnvironment.KDEaddlibs = KDEaddlibs
	SConsEnvironment.KDEaddpaths_includes = KDEaddpaths_includes
	SConsEnvironment.KDEaddpaths_libs = KDEaddpaths_libs

	SConsEnvironment.docfolder = docfolder
	SConsEnvironment.kobject = kobject

