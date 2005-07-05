## Thomas Nagy, 2005

"""
Detect and store the most common options
* kdecxxflags  : debug=1 (-g) or debug=full (-g3, slower)
  else use the user CXXFLAGS if any, - or -O2 by default
* prefix : the installation path
* extraincludes : a list of paths separated by ':'
ie: scons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local
"""

BOLD   ="\033[1m"
RED    ="\033[91m"
GREEN  ="\033[92m"
YELLOW ="\033[1m" #"\033[93m" # unreadable on white backgrounds
CYAN   ="\033[96m"
NORMAL ="\033[0m"

import os, re, types, sys, string, shutil, stat

import SCons.Defaults
import SCons.Tool
import SCons.Util
from SCons.Script.SConscript import SConsEnvironment
from SCons.Options import Options, PathOption

class genobj:
	def __init__(self, val, env):
		if not val in "program shlib kioslave staticlib".split():
			print "unknown genobj given: "+val
			env.Exit(1)

		self.type = val
		self.orenv = env
		self.env   = None
		self.executed = 0

		self.target=''
		self.src=None

		self.cxxflags=''
		self.cflags=''
		self.includes=''

		self.linkflags=''
		self.libpaths=''
		self.libs=''

		# vars used by shlibs
		self.vnum=''
		self.libprefix=''

		# a directory where to install the targets (optional)
		self.instdir=''
		# ignore the DESTDIR (optional)
		self.nodestdir=''

		# change the working directory before reading the targets
		self.chdir=''

		# these members are private
		self.chdir_lock=None
		self.old_os_dir=''
		self.old_fs_dir=''
		self.p_local_shlibs=[]
		self.p_local_staticlibs=[]
		self.p_global_shlibs=[]

		#if not env.has_key('USE_THE_FORCE_LUKE'): env['USE_THE_FORCE_LUKE']=[self]
		#else: env['USE_THE_FORCE_LUKE'].append(self)

	def lockchdir(self):
		if not self.chdir: return
		self.chdir_lock=1
		SConfFS=SCons.Node.FS.default_fs
		self.old_fs_dir=SConfFS.getcwd()
		self.old_os_dir=os.getcwd()
		#os.chdir(old_os_dir+'/'+self.chdir)
		SConfFS.chdir( SConfFS.Dir('#/'+self.chdir), change_os_dir=1)

	def unlockchdir(self):
		if not self.chdir: return
		if self.chdir_lock:
			#os.chdir(self.old_os_dir)
			SCons.Node.FS.default_fs.chdir(self.old_fs_dir, change_os_dir=0)
			self.chdir_lock=None

	def execute(self):
		if self.orenv.has_key('DUMPCONFIG'):
			print self.xml()
			return

		self.lockchdir()

		self.env = self.orenv.Copy()

		if not self.src or len(self.src) == 0:
			print RED+"no source file given to object - self.src"+NORMAL
			self.env.Exit(1)
		if not self.env.has_key('nosmart_includes'): self.env.AppendUnique(CPPPATH=['./'])
		if self.type == "kioslave": self.libprefix=''

		if len(self.includes)>0: self.env.AppendUnique(CPPPATH=self.env.make_list(self.includes))
		if len(self.cxxflags)>0: self.env.AppendUnique(CXXFLAGS=self.env.make_list(self.cxxflags))
		if len(self.cflags)>0: self.env.AppendUnique(CCFLAGS=self.env.make_list(self.cflags))

		llist=self.env.make_list(self.libs)
		lext='.so .la'.split()
		sext='.a'.split()
		for l in llist:
			sal=SCons.Util.splitext(l)
			if len(sal)>1:
				if sal[1] in lext: self.p_local_shlibs.append(sal[0]+'.so')
				elif sal[1] in sext: self.p_local_staticlibs.append(sal[0]+'.a')
				else: self.p_global_shlibs.append(l)

		if len(self.p_global_shlibs)>0: self.env.AppendUnique(LIBS=self.p_global_shlibs)
		if len(self.libpaths)>0:   self.env.PrependUnique(LIBPATH=self.env.make_list(self.libpaths))
		if len(self.linkflags)>0:  self.env.PrependUnique(LINKFLAGS=self.env.make_list(self.linkflags))

		# the target to return
		ret=None
		if self.type=='shlib' or self.type=='kioslave':
			ret=self.env.bksys_shlib(self.target, self.src, self.instdir, 
				self.libprefix, self.vnum, nodestdir=self.nodestdir)
		elif self.type=='program':
			ret=self.env.Program(self.target, self.src)
			if not self.env.has_key('NOAUTOINSTALL'):
				self.env.bksys_install(self.instdir, ret, nodestdir=self.nodestdir)
		elif self.type=='staticlib':
			ret=self.env.StaticLibrary(self.target, self.src)

		# we link the program against a shared library made locally, add the dependency
		if len(self.p_local_shlibs)>0:
			self.env.link_local_shlib(self.p_local_shlibs)
			if ret: self.env.Depends( ret, self.p_local_shlibs )
		if len(self.p_local_staticlibs)>0:
			self.env.link_local_staticlib(self.p_local_staticlibs)
			if ret: self.env.Depends( ret, self.p_local_staticlibs )

		self.unlockchdir()

## Copy function that honors symlinks
def copy_bksys(dest, source, env):
        if os.path.islink(source):
		#print "symlinking "+source+" "+dest
		if os.path.islink(dest):
			os.unlink(dest)
		os.symlink(os.readlink(source), dest)
	else:
		shutil.copy2(source, dest)
		st=os.stat(source)
		os.chmod(dest, stat.S_IMODE(st[stat.ST_MODE]) | stat.S_IWRITE)
	return 0

## Return a list of things
def make_list(env, s):
	if type(s) is types.ListType:
		return s
	else:
		return s.split()

def exists(env):
	return true

def generate(env):
	## Bksys requires scons 0.96
	env.EnsureSConsVersion(0, 96)

	SConsEnvironment.make_list = make_list

	env['HELP']=0
	if '--help' in sys.argv or '-h' in sys.argv or 'help' in sys.argv:
		env['HELP']=1
	
	if env['HELP']:
		print """
\033[1m*** Instructions ***
--------------------\033[0m
\033[1m* scons \033[0m:            to compile
\033[1m* scons -j4 \033[0m:        to compile with several instances
\033[1m* scons install \033[0m:    to compile and install
\033[1m* scons -c install \033[0m: to uninstall

\033[1m*** Generic options ***
-----------------------\033[0m
\033[1m* debug  \033[0m: debug=1 (-g) or debug=full (-g3, slower) else use environment CXXFLAGS, or -O2 by default
\033[1m* prefix \033[0m: the installation path
\033[1m* extraincludes \033[0m: a list of paths separated by ':'
\033[1mscons configure debug=full prefix=/usr/local extraincludes=/tmp/include:/usr/local\033[0m
"""
		return
	
	## Global cache directory
	# Put all project files in it so a rm -rf cache will clean up the config
	if not env.has_key('CACHEDIR'):
		env['CACHEDIR'] = os.getcwd()+'/cache/'
	if not os.path.isdir(env['CACHEDIR']):
		os.mkdir(env['CACHEDIR'])
	
	## SCons cache directory
	# This avoids recompiling the same files over and over again: 
	# very handy when working with cvs
	if os.getuid() != 0:
		env.CacheDir(os.getcwd()+'/cache/objects')

	#  Avoid spreading .sconsign files everywhere - keep this line
	env.SConsignFile(env['CACHEDIR']+'/scons_signatures')
	
	def makeHashTable(args):
		table = { }
		for arg in args:
			if len(arg) > 1:
				lst=arg.split('=')
				if len(lst) < 2:
					continue
				key=lst[0]
				value=lst[1]
				if len(key) > 0 and len(value) >0:
					table[key] = value
 		return table

	env['ARGS']=makeHashTable(sys.argv)

	## Special trick for installing rpms ...
	env['DESTDIR']=''
	if 'install' in sys.argv:
		dd=''
		if os.environ.has_key('DESTDIR'):
			dd=os.environ['DESTDIR']
		if not dd:
			if env['ARGS']: dd=env['ARGS']['DESTDIR']
		if dd:
			env['DESTDIR']=dd+'/'
			print CYAN+'** Enabling DESTDIR for the project ** ' + NORMAL + env['DESTDIR']

	## install symlinks for shared libraries properly
	env['INSTALL'] = copy_bksys

	## Use the same extension .o for all object files
	env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

	## load the options
	cachefile=env['CACHEDIR']+'generic.cache.py'
	opts = Options(cachefile)
	opts.AddOptions(
		( 'GENCCFLAGS', 'C flags' ),
		( 'GENCXXFLAGS', 'debug level for the project : full or just anything' ),
		( 'GENLINKFLAGS', 'additional link flags' ),
		( 'PREFIX', 'prefix for installation' ),
		( 'EXTRAINCLUDES', 'extra include paths for the project' ),
		( 'ISCONFIGURED', 'is the project configured' ),
	)
	opts.Update(env)
	
	# Use this to avoid an error message 'how to make target configure ?'
	env.Alias('configure', None)

	# Check if the following command line arguments have been given
	# and set a flag in the environment to show whether or not it was
	# given.
	if 'install' in sys.argv:
		env['_INSTALL']=1
	else:
		env['_INSTALL']=0
	if 'configure' in sys.argv:
		env['_CONFIGURE']=1
	else:
		env['_CONFIGURE']=0

	# Configure the environment if needed
	if not env['HELP'] and (env['_CONFIGURE'] or not env.has_key('ISCONFIGURED')):

		# be paranoid, unset existing variables
		for var in "GENCXXFLAGS GENCCFLAGS GENLINKFLAGS PREFIX EXTRAINCLUDES ISCONFIGURED EXTRAINCLUDES".split():
			if env.has_key(var): env.__delitem__(var)

		if env['ARGS'].get('debug', None):
			debuglevel = env['ARGS'].get('debug', None)
			print CYAN+'** Enabling debug for the project **' + NORMAL
			if (debuglevel == "full"):
				env['GENCXXFLAGS'] = ['-DDEBUG', '-g3', '-Wall']
			else:
				env['GENCXXFLAGS'] = ['-DDEBUG', '-g', '-Wall']
		else:
			if os.environ.has_key('CXXFLAGS'):
				# user-defined flags (gentooers will be elighted)
				env['GENCXXFLAGS'] = SCons.Util.CLVar( os.environ['CXXFLAGS'] )
				env.Append( GENCXXFLAGS = ['-DNDEBUG', '-DNO_DEBUG'] )
			else:
				env.Append(GENCXXFLAGS = ['-O2', '-DNDEBUG', '-DNO_DEBUG'])

		if os.environ.has_key('CFLAGS'):
			env['GENCCFLAGS'] = SCons.Util.CLVar( os.environ['CFLAGS'] )

		## FreeBSD settings (contributed by will at freebsd dot org)
		if os.uname()[0] == "FreeBSD":
			if os.environ.has_key('PTHREAD_LIBS'):
				env.AppendUnique( GENLINKFLAGS = SCons.Util.CLVar( os.environ['PTHREAD_LIBS'] ) )
		        else:
				syspf = os.popen('/sbin/sysctl kern.osreldate')
				osreldate = int(syspf.read().split()[1])
				syspf.close()
				if osreldate < 500016:
					env.AppendUnique( GENLINKFLAGS = ['-pthread'])
					env.AppendUnique( GENCXXFLAGS = ['-D_THREAD_SAFE'])
				elif osreldate < 502102:
					env.AppendUnique( GENLINKFLAGS = ['-lc_r'])
					env.AppendUnique( GENCXXFLAGS = ['-D_THREAD_SAFE'])
				else:
					env.AppendUnique( GENLINKFLAGS = ['-pthread'])

		# User-specified prefix
		if env['ARGS'].has_key('prefix'):
			env['PREFIX'] = os.path.abspath( env['ARGS'].get('prefix', '') )
			print (CYAN+'** installation prefix for the project set to : ' +
			       env['PREFIX'] +' **'+ NORMAL)

		# User-specified include paths
		env['EXTRAINCLUDES'] = env['ARGS'].get('extraincludes', None)
		if env['EXTRAINCLUDES']:
			print (CYAN+'** extra include paths for the project set to: ' +
			       env['EXTRAINCLUDES'] +' **'+ NORMAL)

		env['ISCONFIGURED']=1

		# And finally save the options in the cache
		opts.Save(cachefile, env)

	def bksys_install(lenv, subdir, files, destfile=None, nodestdir=None):
		""" Install files on "scons install"
		If the DESTDIR env variable has been set, (e.g. by 
		"scons install DESTDIR=$CURDIR/debian) then install files to that
		directory, regardless of where the configure stage showed that
		files should be installed.
		This feature is useful for packagers, and users of GNU stow.
		
		NB. The DESTDIR will be ignored if NODESTDIR is also set, although
		the same effect can be acheived by not setting DESTDIR in the first
		place."""

		if not env['_INSTALL']:
			return
		basedir = env['DESTDIR']
		if nodestdir or env.has_key('NODESTDIR') : basedir = "/"
		install_list = None
		if not destfile:
			install_list = env.Install(basedir+subdir+'/', files)
		else:
			if subdir:
				install_list = env.InstallAs(basedir+subdir+'/'+destfile, files)
			else:
				install_list = env.InstallAs(basedir+'/'+destfile, files)
		env.Alias('install', install_list)
		return install_list

	def build_la_file(target, source, env):
		""" Action for building libtool files.
		Writes a .la file, as used by libtool."""
		dest=open(target[0].path, 'w')
		sname=source[0].name
		dest.write("dlname='%s'\n" % sname)
		if len(env['BKSYS_VNUM'])>0:
			vnum=env['BKSYS_VNUM']
			nums=vnum.split('.')
			src=source[0].name
			name = src.split('so.')[0] + 'so'
			strn = src+" "+name+"."+str(nums[0])+" "+name
			dest.write("library_names='%s'\n" % (strn) )
		else:
			dest.write("library_names='%s %s %s'\n" % (sname, sname, sname) )
		dest.write("old_library=''\ndependency_libs=''\ncurrent=0\n")
		dest.write("age=0\nrevision=0\ninstalled=yes\nshouldnotlink=no\n")
		dest.write("dlopen=''\ndlpreopen=''\n")
		dest.write("libdir='%s'" % env['BKSYS_DESTDIR'])
		dest.close()
		return 0

	def string_la_file(target, source, env):
		print "building '%s' from '%s'" % (target[0].name, source[0].name)
	la_file = env.Action(build_la_file, string_la_file, ['BKSYS_VNUM', 'BKSYS_DESTDIR'])
	env['BUILDERS']['LaFile'] = env.Builder(action=la_file,suffix='.la',src_suffix=env['SHLIBSUFFIX'])

	## Function for building shared libraries
	def bksys_shlib(lenv, target, source, libdir, libprefix='lib', vnum='', noinst=None, nodestdir=None):
		""" Install a shared library.
		
		Installs a shared library, with or without a version number, and create a
		.la file for use by libtool.
		
		If library version numbering is to be used, the version number
		should be passed as a period-delimited version number (e.g.
		vnum = '1.2.3').  This causes the library to be installed
		with its full version number, and with symlinks pointing to it.
		
		For example, for libfoo version 1.2.3, install the file
		libfoo.so.1.2.3, and create symlinks libfoo.so and
		libfoo.so.1 that point to it.
		"""
		thisenv = lenv.Copy() # copying an existing environment is cheap
		thisenv['BKSYS_DESTDIR']=libdir
		thisenv['BKSYS_VNUM']=vnum
		thisenv['SHLIBPREFIX']=libprefix

		if len(vnum)>0:
			thisenv['SHLIBSUFFIX']='.so.'+vnum
			thisenv.Depends(target, thisenv.Value(vnum))

		# Fix against a scons bug - shared libs and ordinal out of range(128)
		if type(source) is types.ListType:
			src2=[]
			for i in source:
				src2.append( str(i) )
			source=src2

		library_list = thisenv.SharedLibrary(target, source)
		lafile_list  = thisenv.LaFile(target, library_list)

		## Install the libraries automatically
		if not thisenv.has_key('NOAUTOINSTALL') and not noinst:
			thisenv.bksys_install(libdir, library_list, nodestdir=nodestdir)
			thisenv.bksys_install(libdir, lafile_list, nodestdir=nodestdir)	

		## Handle the versioning
		if len(vnum)>0:
			nums=vnum.split('.')
			symlinkcom = ('cd $TARGET.dir && ' +
			              'rm -f $TARGET.name && ' +
			              'ln -s $SOURCE.name $TARGET.name')
			tg = target+'.so.'+vnum
			nm1 = target+'.so'
			nm2 = target+'.so.'+nums[0]
			
			thisenv.Command(nm1, tg, symlinkcom)
			thisenv.Command(nm2, tg, symlinkcom)

			#base=env['DESTDIR']+libdir+'/'
			thisenv.bksys_install(libdir, nm1, nodestdir=nodestdir)
			thisenv.bksys_install(libdir, nm2, nodestdir=nodestdir)

	# Declare scons scripts to process
	def subdirs(lenv, folderlist):
		flist=[]
		if type(folderlist) is types.ListType: flist = folderlist
		else: flist = folderlist.split()
		for i in flist:
			lenv.SConscript(i+"/SConscript")

        def link_local_shlib(lenv, str):
                """ Links against a shared library made in the project """
                lst = lenv.make_list(str)
		for file in lst:
			import re
			reg = re.compile("(.*)/lib(.*).(la|so)")
			result = reg.match(file)
			if not result:
				print "Unknown la file given "+file
				continue
			dir  = result.group(1)
			link = result.group(2)
			lenv.AppendUnique(LIBS = [link])
			lenv.PrependUnique(LIBPATH = [dir])

        def link_local_staticlib(lenv, str):
                """ Links against a shared library made in the project """
                lst = lenv.make_list(str)
		for file in lst:
			import re
			reg = re.compile("(.*)/(lib.*.a)")
			result = reg.match(file)
			if not result:
				print "Unknown archive file given "+file
				continue

			f=SCons.Node.FS.default_fs.File(file)
			lenv.Append(LINKFLAGS=[f.path])

	#valid_targets = "program shlib kioslave staticlib".split()
        SConsEnvironment.bksys_install = bksys_install
	SConsEnvironment.bksys_shlib   = bksys_shlib
	SConsEnvironment.subdirs       = subdirs
	SConsEnvironment.link_local_shlib = link_local_shlib
	SConsEnvironment.link_local_staticlib = link_local_staticlib

	SConsEnvironment.genobj=genobj

	if env.has_key('GENCXXFLAGS'):
		env.AppendUnique( CPPFLAGS = env['GENCXXFLAGS'] )

	if env.has_key('GENCCFLAGS'):
		env.AppendUnique( CCFLAGS = env['GENCCFLAGS'] )

	if env.has_key('GENLINKFLAGS'):
		env.AppendUnique( LINKFLAGS = env['GENLINKFLAGS'] )

	if env.has_key('EXTRAINCLUDES'):
		if env['EXTRAINCLUDES']:
			incpaths = []
			for dir in str(env['EXTRAINCLUDES']).split(':'):
				incpaths.append( dir )
			env.Append(CPPPATH = incpaths)

	env.Export('env')
