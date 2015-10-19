#  _________________________________________________________________________
#
#  PyUtilib: A Python utility library.
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  _________________________________________________________________________
#
#
# This script was created with the virtualenv_install script.
#

import subprocess
import re
try:
    import urllib2
except ImportError:
    import urllib.request as urllib2
try:
    import StringIO
except ImportError:
    import io as StringIO
import zipfile
import shutil
import string
import textwrap
import sys
import glob
import errno
import stat
import os

using_subversion = True
virtualenv_pypi_string = "virtualenv"
setuptools_string = "setuptools"

#
# Working around error with PYTHONHOME
#
if 'PYTHONHOME' in os.environ:
    del os.environ['PYTHONHOME']
    print("WARNING: ignoring the value of the PYTHONHOME environment variable!  This value can corrupt the virtual python installation.")

print("\nNOTE: this Python executable used to create virtual environment:\n\t%s\n" % sys.executable)
#
# The following taken from PyUtilib
#
if sys.platform.startswith('win'): #pragma:nocover
    executable_extension=".exe"
else:                            #pragma:nocover
    executable_extension=""


def search_file(filename, search_path=None, implicitExt=executable_extension, executable=False, isfile=True):
    if search_path is None:
        #
        # Use the PATH environment if it is defined and not empty
        #
        if "PATH" in os.environ and os.environ["PATH"] != "":
            search_path = string.split(os.environ["PATH"], os.pathsep)
        else:
            search_path = os.defpath.split(os.pathsep)
    for path in search_path:
        if os.path.exists(os.path.join(path, filename)) and \
           (not isfile or os.path.isfile(os.path.join(path, filename))):
            if not executable or os.access(os.path.join(path,filename),os.X_OK):
                return os.path.abspath(os.path.join(path, filename))
        if os.path.exists(os.path.join(path, filename+implicitExt)) and \
           (not isfile or os.path.isfile(os.path.join(path, filename+implicitExt))):
            if not executable or os.access(os.path.join(path,filename+implicitExt),os.X_OK):
                return os.path.abspath(os.path.join(path, filename+implicitExt))
    return None

#
# PyUtilib Ends
#


#
# The following taken from pkg_resources
#
component_re = re.compile(r'(\d+ | [a-z]+ | \.| -)', re.VERBOSE)
replace = {'pre':'c', 'preview':'c','-':'final-','rc':'c','dev':'@'}.get

def _parse_version_parts(s):
    for part in component_re.split(s):
        part = replace(part,part)
        if not part or part=='.':
            continue
        if part[:1] in '0123456789':
            yield part.zfill(8)    # pad for numeric comparison
        else:
            yield '*'+part

    yield '*final'  # ensure that alpha/beta/candidate are before final

def parse_version(s):
    parts = []
    for part in _parse_version_parts(s.lower()):
        if part.startswith('*'):
            if part<'*final':   # remove '-' before a prerelease tag
                while parts and parts[-1]=='*final-': parts.pop()
            # remove trailing zeros from each series of numeric parts
            while parts and parts[-1]=='00000000':
                parts.pop()
        parts.append(part)
    return tuple(parts)
#
# pkg_resources Ends
#

#
# Use pkg_resources to guess version.
# This allows for parsing version with the syntax:
#   9.3.2
#   8.28.rc1
#
def guess_release(svndir):
    if using_subversion and not sys.platform.startswith('win'):
        output = subprocess.Popen(['svn','ls',svndir], stdout=subprocess.PIPE).communicate()[0]
        if sys.version_info[:2] >= (3,0):
            if sys.stdout.encoding is None:
                output = output.decode('utf-8')
            else:
                output = output.decode(sys.stdout.encoding)
        if output=="":
            return None
        versions = []
        for link in re.split('/',output.strip()):
            tmp = link.strip()
            if tmp != '':
                versions.append( tmp )
    else:
        if sys.version_info[:2] <= (2,5):
            output = urllib2.urlopen(svndir).read()
        else:
            output = urllib2.urlopen(svndir, timeout=30).read()
            if sys.stdout.encoding is None:
                output = output.decode('utf-8')
            else:
                output = output.decode(sys.stdout.encoding)
        if output=="":
            return None
        links = re.findall('\<li>\<a href[^>]+>[^\<]+\</a>',output)
        versions = []
        for link in links:
            versions.append( re.split('>', link[:-5])[-1] )
    latest = None
    latest_str = None
    for version in versions:
        if version == '.':
            continue
        v = parse_version(version)
        if latest is None or latest < v:
            latest = v
            latest_str = version
    if latest_str is None:
        return None
    if not latest_str[0] in '0123456789':
        return svndir
    return svndir+"/"+latest_str


def zip_file(filename,fdlist):
    zf = zipfile.ZipFile(filename, 'w')
    for file in fdlist:
        if os.path.isdir(file):
            for root, dirs, files in os.walk(file):
                if root.startswith(file+os.sep+'lib') or root.startswith(file+os.sep+'bin'):
                    continue
                for fname in files:
                    if fname.endswith('pyc') or fname.endswith('pyo') or fname.endswith('zip'):
                        continue
                    try:
                        long16 = long(16)
                        long28 = long(28)
                    except NameError:
                        long16 = 16
                        long28 = 28
                    if fname.endswith('exe'):
                        # Octal shifts
                        #zf.external_attr = (0777 << 16L) | (010 << 28L)
                        # Decimal shifts
                        zf.external_attr = (511 << long16) | (8 << long28)
                    else:
                        # Octal shifts
                        #zf.external_attr = (0660 << 16L) | (010 << 28L)
                        # Decimal shifts
                        zf.external_attr = (432 << long16) | (8 << long28)
                    zf.write(join(root,fname))
        else:
            zf.write(file)
    zf.close()


def unzip_file(filename, dir=None, verbose=False):
    fname = os.path.abspath(filename)
    logger.info("Unzipping from file '%s'" % fname)
    zf = zipfile.ZipFile(fname, 'r')
    if dir is None:
        dir = os.getcwd()
    for file in zf.infolist():
        name = file.filename
        if name.endswith('/') or name.endswith('\\'):
            outfile = os.path.join(dir, name)
            if not os.path.exists(outfile):
                os.makedirs(outfile)
        else:
            outfile = os.path.join(dir, name)
            parent = os.path.dirname(outfile)
            if not os.path.exists(parent):
                os.makedirs(parent)
            OUTPUT = open(outfile, 'wb')
            OUTPUT.write(zf.read(name))
            OUTPUT.close()
    zf.close()
    logger.info("Unzipping done.")



class Repository(object):

    svn_get='checkout'
    useEasyInstall = False
    easy_install_path = None #["easy_install"]
    easy_install_flags = ['-q']
    pip_path = None #["pip"]
    pip_flags = ['-q']
    python = None #"python"
    svn = "svn" + executable_extension
    dev = []

    def __init__(self, name, **kwds):
        class _TEMP_(object):
            def __init__( self, root=None, trunk=None, 
                          release=None, tag=None, pyname=None, pypi=None, 
                          dev=False, username=None, install=True, rev=None, 
                          local=None, platform=None, version=None, 
                          branch=None, exit=True ):
                import inspect
                args, varargs, varkw, defaults = inspect.getargspec(self.__init__)
                for i in range(len(args)-1):
                    kwd = args[i+1]
                    setattr(self, kwd, defaults[i])
        self.config = _TEMP_()
        self.config.name = name
        for kwd in kwds:
            if kwd == 'dev':
                if kwds[kwd] == 'True' or kwds[kwd] is True:
                    self.config.dev = True
                else:
                    self.config.dev = False
            elif kwd == 'install':
                if kwds[kwd] == 'False' or kwds[kwd] is False:
                    self.config.install = False
                else:
                    self.config.install = True
            elif kwd == 'exit':
                if kwds[kwd] == 'False' or kwds[kwd] is False:
                    self.config.exit = False
                else:
                    self.config.exit = True
            else:
                setattr(self.config, kwd, kwds[kwd])
        #
        self.offline=False
        tmp = None
        if self.config.local:
            tmp = self.config.local
        elif self.config.dev:
            tmp = join(Installer.abshome_dir,'src', name)
        else:
            tmp = join(Installer.abshome_dir,'dist',name)
        if tmp and os.path.exists(tmp):
            self.offline=True
        print("Repository %s %s %s" % (str(self.offline),tmp,name))
        #
        self.initialize(self.config)

    @staticmethod
    def _configureExecutables(installer=None):
        #
        # Set the bin directory
        #
        if installer is not None:
            if os.path.exists(installer.abshome_dir+os.sep+"Scripts"):
                Repository.bindir = join(installer.abshome_dir,"Scripts")
            else:
                Repository.bindir = join(installer.abshome_dir,"bin")

        bindir = Repository.bindir
        if bindir is None:
            print("(ERROR) installer called _configureExecutables() "
                  "before _configureExecutables(installer)")
            return

        if Repository.python is None:
            _path = os.path.abspath(join(bindir, 'jython.bat'))
            if is_jython:
                if os.path.exists(_path):
                    Repository.python = _path
            else:
                _path = os.path.abspath(join(bindir, 'python'))
                if sys.platform.startswith('win'):
                    _path += 'w.exe'
                if os.path.exists(_path):
                    Repository.python = _path
            if Repository.python is None:
                print("(INFO) Virtualenv 'python' executable not found")
        if Repository.easy_install_path is None:
            _path = os.path.abspath(join(bindir, 'easy_install'))
            if os.path.exists(_path):
                Repository.easy_install_path = [Repository.python, _path]
            elif os.path.exists(_path + '.exe'):
                Repository.easy_install_path = [_path + '.exe']
            else:
                print("(INFO) Virtualenv 'easy_install' executable not found")
        if Repository.pip_path is None:
            _path = os.path.abspath(join(bindir, 'pip'))
            if os.path.exists(_path):
                Repository.pip_path = [Repository.python, _path]
            elif os.path.exists(_path + '.exe'):
                Repository.pip_path = [_path + '.exe']
            else:
                print("(INFO) Virtualenv 'pip' executable not found")
 

    def initialize(self, config):
        self.name = config.name
        self.root = config.root
        self.trunk = None
        self.trunk_root = None
        self.branch = None
        self.release = None
        self.tag = None
        self.release_root = None
        #
        self.pypi = config.pypi
        self.local = config.local
        self.platform = config.platform
        if config.platform is None:
            self.platform_re = None
        else:
            self.platform_re = re.compile(config.platform)
        self.version = config.version
        if not config.pypi is None:
            self.pyname=config.pypi
        else:
            self.pyname=config.pyname
        self.dev = config.dev
        if config.dev:
            Repository.dev.append(config.name)
        self.pkgdir = None
        self.pkgroot = None
        if config.username is None or '$' in config.username:
            self.svn_username = []
        else:
            self.svn_username = ['--username', config.username]
        if config.rev is None:
            self.rev=''
            self.revarg=[]
        else:
            self.rev='@'+config.rev
            self.revarg=['-r',config.rev]
        self.install = config.install

    def guess_versions(self):
        if not self.config.root is None:
            if not self.offline:
                if using_subversion and not sys.platform.startswith('win'):
                    rootdir_output = subprocess.Popen(['svn','ls',self.config.root], stdout=subprocess.PIPE).communicate()[0]
                else:
                    if sys.version_info[:2] <= (2,5):
                        rootdir_output = urllib2.urlopen(self.config.root).read()
                    else:
                        rootdir_output = urllib2.urlopen(self.config.root, timeout=30).read()
                if sys.version_info[:2] >= (3,0):
                    if sys.stdout.encoding is None:
                        rootdir_output = rootdir_output.decode('utf-8')
                    else:
                        rootdir_output = rootdir_output.decode(sys.stdout.encoding)
            if self.config.branch:
                self.trunk = self.config.root+'/branches/'+self.config.branch
            else:
                self.trunk = self.config.root+'/trunk'
            self.trunk_root = self.trunk
            try:
                if self.offline or not 'releases' in rootdir_output:
                    raise IOError
                self.release = guess_release(self.config.root+'/releases')
                self.tag = None
                self.release_root = self.release
            except (urllib2.HTTPError,IOError):
                try:
                    if self.offline or not 'tags' in rootdir_output:
                        raise IOError
                    self.release = guess_release(self.config.root+'/tags')
                    self.tag = self.release
                    self.release_root = self.release
                except (urllib2.HTTPError,IOError):
                    self.release = None
                    self.release_root = None
        if not self.config.trunk is None:
            if self.trunk is None:
                self.trunk = self.config.trunk
            else:
                self.trunk += self.config.trunk
        if not self.config.release is None:
            if self.release is None:
                self.release = self.config.release
            else:
                self.release += self.config.release
        if not self.config.tag is None:
            if self.release is None:
                self.release = self.config.tag
            else:
                self.release += self.config.tag


    def write_config(self, OUTPUT):
        config = self.config
        sys.stdout = OUTPUT
        print('[%s]' % config.name)
        if not config.root is None:
            print('root=%s' % config.root)
        if not config.trunk is None:
            print('trunk=%s' % config.trunk)
        if not config.tag is None:
            print('tag=%s' % config.tag)
        elif not config.release is None:
            print('release=%s' % config.release)
        if not config.local is None:
            print('local=%s' % config.local)
        if not config.pypi is None:
            print('pypi=%s' % config.pypi)
        elif not config.pyname is None:
            print('pypi=%s' % config.pyname)
        print('dev=%s' % str(config.dev))
        if not config.branch is None:
            print('branch=%s' % str(config.branch))
        print('install=%s' % str(config.install))
        if not config.rev is None:
            print('rev=%s' % str(config.rev))
        if not config.username is None:
            print('username=%s' % str(config.username))
        if not config.platform is None:
            print('platform=%s' % config.platform)
        if not config.version is None:
            print('version=%s' % config.version)
        print('exit=%s' % config.exit)
        sys.stdout = sys.__stdout__


    def find_pkgroot(self, trunk=False, release=False):
        if trunk:
            if self.trunk is None:
                if self.pypi is None and self.local is None:
                    self.find_pkgroot(release=True)
                else:
                    # use easy_install
                    self.pkgdir = None
                    self.pkgroot = None
                    return
            else:
                self.pkgdir = self.trunk
                self.pkgroot = self.trunk_root
                return

        elif release:
            if self.release is None:
                if self.pypi is None and self.local is None:
                    self.find_pkgroot(trunk=True)
                else:
                    # use easy_install
                    self.pkgdir = None
                    self.pkgroot = None
                    return
            else:
                self.pkgdir = self.release
                self.pkgroot = self.release_root

        else:
            raise IOError("Must have one of trunk or release specified: %s" % self.name)


    def install_trunk(self, dir=None, install=True, preinstall=False):
        self.find_pkgroot(trunk=True)
        self.perform_install(dir=dir, install=install, preinstall=preinstall)

    def install_release(self, dir=None, install=True, preinstall=False):
        self.find_pkgroot(release=True)
        self.perform_install(dir=dir, install=install, preinstall=preinstall)

    def perform_install(self, dir=None, install=True, preinstall=False):
        if not self.platform_re is None and not self.platform_re.match(sys.platform):
            return
        if not self.version is None and not eval(self.version):
            return
        if self.pkgdir is None and self.local is None:
            self.install_package(install, preinstall, dir)
            return
        if self.local:
            install = True
        print("-----------------------------------------------------------------")
        print("  Installing branch")
        print("  Checking out source for package "+self.name)
        if self.local:
            print("     Package dir: "+self.local)
        else:
            print("     Subversion dir: "+self.pkgdir)
        if os.path.exists(dir):
            print("     No checkout required")
            print("-----------------------------------------------------------------")
        elif not using_subversion:
            print("")
            print("Error: Cannot checkout software %s with subversion." % self.name)
            print("A problem was detected executing subversion commands.")
            if self.config.exit:
                print("Aborting installer!")
                sys.exit(1)
            print("Not aborting installer...")
            return
        else:
            print("-----------------------------------------------------------------")
            try:
                self.run([self.svn]+self.svn_username+[Repository.svn_get,'-q',self.pkgdir+self.rev, dir])
            except OSError:
                err,tb = sys.exc_info()[1:3] # BUG?
                print("")
                print("Error checkout software %s with subversion at %s" % (self.name,self.pkgdir+self.rev))
                print(str(err))
                print("Traceback:")
                import traceback
                traceback.print_stack(f=tb.tb_frame.f_back,file=sys.stdout)
                traceback.print_tb(tb, file=sys.stdout)
                if self.config.exit:
                    print("Aborting installer!")
                    sys.exit(1)
                print("Not aborting installer...")
                return
        if install:
            try:
                if self.dev:
                    if self.offline:
                        self.run([self.python, 'setup.py', 'develop', '--no-deps'], dir=dir)
                    else:
                        self.run([self.python, 'setup.py', 'develop'], dir=dir)
                else:
                    self.run([self.python, 'setup.py', 'install'], dir=dir)
            except OSError:
                err,tb = sys.exc_info()[1:3] # BUG?
                print("")
                print("Error installing software %s from source using the setup.py file." % self.name)
                print("This is probably due to a syntax or configuration error in this package.")
                print(str(err))
                print("Traceback:")
                import traceback
                traceback.print_stack(f=tb.tb_frame.f_back,file=sys.stdout)
                traceback.print_tb(tb, file=sys.stdout)
                if self.config.exit:
                    print("Aborting installer!")
                    sys.exit(1)
                print("Not aborting installer...")

    def install_package(self, install, preinstall, dir):
        Repository._configureExecutables()
        extra_env = {}
        if os.path.basename(dir) == 'distribute':
            extra_env['DONT_PATCH_SETUPTOOLS'] = 'True'
        if Repository.useEasyInstall:
            try:
                return self.easy_install(install, preinstall, dir, extra_env)
            except:
                print("ERROR installing with easy_install; falling back on PIP")
            return self.pip_install(install, preinstall, dir, extra_env)
        else:
            try:
                return self.pip_install(install, preinstall, dir, extra_env)
            except:
                print("ERROR installing with PIP; falling back on easy_install")
            return self.easy_install(install, preinstall, dir, extra_env)

    def easy_install(self, install, preinstall, dir, extra_env=None):
        try:
            if install:
                if self.offline:
                    self.run([self.python, 'setup.py', 'install'], 
                             dir=dir, extra_env=extra_env)
                else:
                    self.run( self.easy_install_path 
                              + Repository.easy_install_flags + [self.pypi], 
                              dir=os.path.dirname(dir), extra_env=extra_env)
            elif preinstall:
                if not os.path.exists(dir):
                    self.run(self.easy_install_path 
                             + Repository.easy_install_flags 
                             + [ '--exclude-scripts', '--always-copy', 
                                 '--editable', '--build-directory', '.', 
                                 self.pypi], 
                             dir=os.path.dirname(dir), extra_env=extra_env)
        except OSError:
            err,tb = sys.exc_info()[1:3] # BUG?
            print("")
            print("Error installing package %s with easy_install" % self.name)
            print(str(err))
            print("Traceback:")
            import traceback
            traceback.print_stack(f=tb.tb_frame.f_back,file=sys.stdout)
            traceback.print_tb(tb, file=sys.stdout)
            if self.config.exit:
                print("Aborting installer!")
                sys.exit(1)
            print("Not aborting installer...")

    def pip_install(self, install, preinstall, dir, extra_env=None):
        try:
            if install:
                if self.offline:
                    self.run([self.python, 'setup.py', 'install'], 
                             dir=dir, extra_env=extra_env)
                else:
                    self.run( self.pip_path + ['install'] 
                              + Repository.pip_flags + [self.pypi],
                              dir=os.path.dirname(dir), extra_env=extra_env)
            elif preinstall:
                if not os.path.exists(dir):
                    self.run( 
                        self.pip_path + ['install'] 
                        + Repository.pip_flags 
                        + [ #'--no-install', '--ignore-installed',
                            #'--build', '.', 
                            '--no-binary', '--download', '.', 
                            self.pypi ],
                        dir=os.path.dirname(dir), extra_env=extra_env)
        except OSError:
            err,tb = sys.exc_info()[1:3] # BUG?
            print("")
            print("Error installing package %s with pip" % self.name)
            print(str(err))
            print("Traceback:")
            import traceback
            traceback.print_stack(f=tb.tb_frame.f_back,file=sys.stdout)
            traceback.print_tb(tb, file=sys.stdout)
            if self.config.exit:
                print("Aborting installer!")
                sys.exit(1)
            print("Not aborting installer...")

    def upgrade_packages(self):
        if Repository.useEasyInstall:
            return self.easy_upgrade()
        else:
            return self.pip_upgrade()

    def easy_upgrade(self):
        self.run( self.easy_install_path + Repository.easy_install_flags 
                  + ['--upgrade', self.pypi] )

    def pip_upgrade(self):
        self.run( self.pip_path + ['install'] + Repository.pip_flags 
                  + ['--upgrade', self.pypi] )

    def run(self, cmd, dir=None, extra_env=None):
        cwd=os.getcwd()
        if not dir is None:
            os.chdir(dir)
            cwd=dir
        print( "\nRunning command '%s'\n\tin directory %s" 
               % (" ".join(cmd), cwd) )
        sys.stdout.flush()
        call_subprocess(cmd, filter_stdout=filter_python_develop, show_stdout=True, extra_env=extra_env)
        if not dir is None:
            os.chdir(cwd)


def filter_python_develop(line):
    if not line.strip():
        return Logger.DEBUG
    for prefix in ['Searching for', 'Reading ', 'Best match: ', 'Processing ',
                   'Moving ', 'Adding ', 'running ', 'writing ', 'Creating ',
                   'creating ', 'Copying ']:
        if line.startswith(prefix):
            return Logger.DEBUG
    return Logger.NOTIFY


def apply_template(str, d):
    t = string.Template(str)
    return t.safe_substitute(d)


wrapper = textwrap.TextWrapper(subsequent_indent="    ")


class Installer(object):

    abshome_dir = None

    def __init__(self):
        self.description="This script manages the installation of packages into a virtual Python installation."
        self.home_dir = None
        self.default_dirname='python'
        self.sw_packages = []
        self.sw_dict = {}
        self.cmd_files = []
        self.auxdir = []
        self.srcdir = None
        self.config=None
        self.config_file=None
        self.README="""
#
# Virtual Python installation generated by the %s script.
#
# This directory is managed with virtualenv, which creates a
# virtual Python installation.  If the 'bin' directory is put in
# user's PATH environment, then the bin/python command can be used
# without further installation.
#
# Directories:
#   admin      Administrative data for maintaining this distribution
#   bin        Scripts and executables
#   dist       Python packages that are not intended for development
#   include    Python header files
#   lib        Python libraries and installed packages
#   src        Python packages whose source files can be
#              modified and used directly within this virtual Python
#              installation.
#   Scripts    Python bin directory (used on MS Windows)
#
""" % sys.argv[0]

    def add_repository(self, *args, **kwds):
        if not 'root' in kwds and not 'pypi' in kwds and not 'release' in kwds and not 'trunk' in kwds:
            raise IOError("No repository info specified for repository "+args[0])
        repos = Repository( *args, **kwds)
        if repos.name in self.sw_dict:
            for i in range(len(self.sw_packages)):
                if self.sw_packages[i].name == repos.name:
                    self.sw_packages.pop(i)
                    break
        self.sw_dict[repos.name] = repos
        self.sw_packages.append( repos )

    def add_dos_cmd(self, file):
        self.cmd_files.append( file )

    def add_auxdir(self, package, todir, fromdir):
        self.auxdir.append( (todir, package, fromdir) )

    def modify_parser(self, parser):
        self.default_windir = 'C:\\'+self.default_dirname
        self.default_unixdir = './'+self.default_dirname
        #
        parser.add_option('--debug',
            help='Configure script to generate debugging IO and to raise exceptions.',
            action='store_true',
            dest='debug',
            default=False)

        parser.add_option('--release',
            help='Install release branches of Python software using subversion.',
            action='store_true',
            dest='release',
            default=False)

        parser.add_option('--trunk',
            help='Install trunk branches of Python software using subversion.',
            action='store_true',
            dest='trunk',
            default=False)

        parser.add_option('--offline',
            help='Disable the installation of offline packages',
            action='store_true',
            dest='install_offline',
            default=False)

        parser.add_option('--proxy',
            help='Set the HTTP_PROXY environment with this option.',
            action='store',
            dest='proxy',
            default=None)

        parser.add_option('--preinstall',
            help='Prepare an installation that will be used to build a MS Windows installer.',
            action='store_true',
            dest='preinstall',
            default=False)

        parser.add_option('--zip',
            help='Add ZIP files that are use define this installation.',
            action='append',
            dest='zip',
            default=[])

        parser.add_option('--source', '--src',
            help='Use packages defined in the specified source directory',
            action='store',
            dest='source',
            default=None)

        parser.add_option('--use-pythonpath',
            help="By default, the PYTHONPATH is ignored when installing.  This option allows the 'easy_install' tool to search this path for related Python packages, which are then installed.",
            action='store_true',
            dest='use_pythonpath',
            default=False)

        # Historically, virtualenv used --no-site-packages, but
        # recently, they moved to --system-site-packages /
        # --no-site-packages.  Either way, they support the
        # "--no-site-packages" option.  For portability, we will query
        # that argument and set the default to the logical negation.
        if not parser.has_option('--no-site-packages'):
            raise RuntimeError(
                "Internal VirtualEnv error: cannot determine the name of "
                "the --no-site-packages option.  "
                "Please report this to the PyUtilib Developers.")
        site_packages_opt = parser.get_option('--no-site-packages')
        if site_packages_opt.action == 'store_true':
            parser.set_defaults(**dict([tuple([site_packages_opt.dest,False])]))
        elif site_packages_opt.action == 'store_false':
            parser.set_defaults(**dict([tuple([site_packages_opt.dest,True])]))
        else:
            raise RuntimeError(
                "Internal VirtualEnv error: cannot determine the store "
                "function for the --no-site-packages option.  "
                "Please report this to the PyUtilib Developers.")

        parser.add_option(
            '-a', '--add-package',
            dest='packages',
            action='append',
            help='Specify a package that is added to the virtual Python installation.  This option can specify a directory for the Python package source or PyPI package name that is downloaded automatically.  This option can be specified multiple times to declare multiple packages.',
            default=[])

        parser.add_option('--config',
            help='Use an INI config file to specify the packages used in this installation.  Using this option clears the initial configuration, but multiple uses of this option will add package specifications.',
            action='append',
            dest='config_files',
            default=[])

        parser.add_option('--keep-config',
            help='Keep the initial configuration data that was specified if the --config option is specified.',
            action='store_true',
            dest='keep_config',
            default=False)

        parser.add_option('--without-externals',
            help="Ignore the 'externals' section of the config file.",
            action='store_false',
            dest='follow_externals',
            default=True)

        parser.add_option('--localize',
            help='Force localization of DOS scripts on Linux platforms',
            action='store_true',
            dest='localize',
            default=False)

        parser.add_option('--pypi-url',
            help='Specify the url for the PyPI package index used for online installation',
            action='store',
            dest='pypi_url',
            default=None)


        #
        # Change the virtualenv options
        #
        parser.remove_option("--python")
        parser.add_option("-p", "--python",
            dest='python',
            metavar='PYTHON_EXE',
            help="Specify the Python interpreter to use, e.g., --python=python2.5 will install with the python2.5 interpreter.")
        parser.remove_option("--relocatable")
        parser.remove_option("--version")
        parser.remove_option("--unzip-setuptools")
        parser.remove_option("--clear")
        #
        # Add description
        #
        parser.description=self.description
        parser.epilog="If DEST_DIR is not specified, then a default installation path is used:  "+self.default_windir+" on Windows and "+self.default_unixdir+" on Linux.  This command uses the Python 'pip' package to install Python packages.  This package installs packages by downloading files from the internet.  If you are running this from within a firewall, you may need to set the HTTP_PROXY environment variable to a value like 'http://<proxyhost>:<port>'."


    def adjust_options(self, options, args):
        #
        # Add the virtualenv_support directory that is installed with the virtualenv.
        #
        options.search_dirs.append(os.path.join(Installer.abshome_dir, 'virtualenv_support'))
        #
        # Force options.clear to be False.  This allows us to preserve the logic
        # associated with --clear, which we may want to use later.
        #
        options.clear=False
        #
        global vpy_main
        if options.debug:
            vpy_main.raise_exceptions=True
        #
        global logger
        if options.verbose:
            # This works because the -v will appear after (and override)
            # the default -q on the final command line.
            if '-q' in Repository.easy_install_flags:
                Repository.easy_install_flags.remove('-q')
            Repository.easy_install_flags.append('-v')

            if '-q' in Repository.pip_flags:
                Repository.pip_flags.remove('-q')
            Repository.pip_flags.append('-v')
        verbosity = options.verbose - options.quiet
        self.logger = Logger([(Logger.level_for_integer(2-verbosity), sys.stdout)])
        logger = self.logger
        #
        # Determine if the subversion command is available
        #
        global using_subversion
        try:
            sys.stdout.flush()
            call_subprocess(['svn'+executable_extension,'--version'], show_stdout=False)
        except OSError:
            print("")
            print("------------------------------------------------")
            print("WARNING: problems executing subversion commands.")
            print("Subversion is disabled.")
            print("------------------------------------------------")
            print("")
            using_subversion = False
        #
        if len(args) > 1:
            self.logger.fatal("ERROR: installer script can only have one argument")
            sys.exit(1000)
        #
        # Error checking
        #
        if not options.preinstall and os.path.exists(self.abshome_dir+os.sep+'bin'):
            self.logger.fatal(wrapper.fill("ERROR: The installation path '%s' already exists! Remove this directory to create a fresh installation." % self.home_dir))
            sys.exit(1000)
        if len(args) == 0:
            args.append(self.abshome_dir)
        #
        # Reset the config file if no options are specified
        #
        if not self.config_file is None and not (options.trunk or options.release):
            self.config_file = os.path.dirname(self.config_file)+"/pypi.ini"
        #
        # If applying preinstall, then only do subversion exports
        #
        if options.preinstall:
            Repository.svn_get='export'
        #
        # If the user requested an alternative PyPI index, add it to the
        # easy_install command line.
        #
        if options.pypi_url:
            Repository.easy_install_flags.append("--index-url")
            Repository.easy_install_flags.append(options.pypi_url)
            Repository.pip_flags.append("--index-url")
            Repository.pip_flags.append(options.pypi_url)

    def get_homedir(self, options, args):
        #
        # Figure out the installation directory
        #
        if len(args) == 0:
            path = self.guess_path()
            if path is None or options.preinstall:
                # Install in a default location.
                if sys.platform == 'win32':
                    home_dir = self.default_windir
                else:
                    home_dir = self.default_unixdir
            else:
                home_dir = os.path.dirname(os.path.dirname(path))
        else:
            home_dir = args[0]
        self.home_dir = home_dir
        Installer.abshome_dir = os.path.abspath(home_dir)
        #print "HERE", Installer.abshome_dir
        #print glob.glob(os.path.join(Installer.abshome_dir,'*'))
        #if os.path.exists(os.path.join(Installer.abshome_dir,'Scripts')):
        if sys.platform.startswith('win'):
            Installer.py_executable = os.path.join(Installer.abshome_dir,'Scripts','python')
            #print glob.glob(os.path.join(Installer.abshome_dir,'Scripts','*'))
        else:
            Installer.py_executable = os.path.join(Installer.abshome_dir,'bin','python')
            #print glob.glob(os.path.join(Installer.abshome_dir,'bin','*'))
        if sys.platform.startswith('win'):
            Installer.py_executable += '.exe'
        print("Executable: "+Installer.py_executable)
        if options.source is None:
            self.srcdir = join(self.abshome_dir,'src')
        else:
            self.srcdir = os.path.abspath(options.source)
            if not os.path.exists(self.srcdir):
                raise ValueError(
                    "Specified source directory does not exist! %s"
                    % self.srcdir )
        if os.path.abspath(sys.executable).startswith(self.abshome_dir):
            raise ValueError(
                "Python executable used to create the virtual environment:"
                "\n\t    %s\n\tfound within the target installation directory:"
                "\n\t    %s\n\tCowardly refusing to continue installation."
                % ( os.path.abspath(sys.executable), self.abshome_dir ) )

    def guess_path(self):
        return None

    def setup_installer(self, options):
        if options.preinstall:
            print("Creating preinstall zip file in '%s'" % self.home_dir)
        else:
            print("Starting fresh installation in '%s'" % self.home_dir)
        #
        # Setup HTTP proxy
        #
        proxy = ''
        if not options.proxy is None:
            proxy = options.proxy
        if proxy == '':
            proxy = os.environ.get('HTTP_PROXY', '')
        if proxy == '':
            proxy = os.environ.get('http_proxy', '')
        os.environ['HTTP_PROXY'] = proxy
        os.environ['http_proxy'] = proxy
        print("  using the HTTP_PROXY environment: %s" % proxy)
        proxy = ''
        if not options.proxy is None:
            proxy = options.proxy
        if proxy == '':
            proxy = os.environ.get('HTTPS_PROXY', '')
        if proxy == '':
            proxy = os.environ.get('https_proxy', '')
        os.environ['HTTPS_PROXY'] = proxy
        os.environ['https_proxy'] = proxy
        print("  using the HTTPS_PROXY environment: %s" % proxy)
        print("")
        #
        # Disable the PYTHONPATH, to isolate this installation from
        # other Python installations that a user may be working with.
        #
        if not options.use_pythonpath:
            try:
                del os.environ["PYTHONPATH"]
            except:
                pass
        #
        # If --preinstall is declared, then we remove the directory, and prepare a ZIP file
        # that contains the full installation.
        #
        if options.preinstall:
            print("-----------------------------------------------------------------")
            print(" STARTING preinstall in directory %s" % self.home_dir)
            print("-----------------------------------------------------------------")
            rmtree(self.abshome_dir)
            os.mkdir(self.abshome_dir)
        #
        # When preinstalling or working offline, disable the
        # default install_setuptools() function.
        #
        # JDS [3/31/14]: This logic or something like it may be needed
        # for the post-setuptools 2.0 world, but the actual
        # install_setuptools and install_pip functions that we were
        # editing have disappeared.  Commenting this out so that the
        # offline installer doesn't fail due to syntax error.
        #
        #if options.install_offline:
        #    install_setuptools.use_default=False
        #    install_pip.use_default=False

        #
        # If we're clearing the current installation, then remove a bunch of
        # directories
        #
        elif options.clear and not options.source is None:
            if os.path.exists(self.srcdir):
                rmtree(self.srcdir)
        #
        # Open up zip files
        #
        for file in options.zip:
            unzip_file(file, dir=self.abshome_dir)
        #
        # Parse config files
        #
        if options.release:
            if os.path.exists(join(self.abshome_dir, 'admin', 'config.ini')):
                self.config=None
                options.config_files.append( join(self.abshome_dir, 'admin', 'config.ini') )
        if not self.config is None and (len(options.config_files) == 0 or options.keep_config):
            fp = StringIO.StringIO(self.config)
            self.read_config_file(fp=fp, follow_externals=options.follow_externals)
            fp.close()
        if not self.config_file is None and (len(options.config_files) == 0 or options.keep_config):
            self.read_config_file(file=self.config_file, follow_externals=options.follow_externals)
        for file in options.config_files:
            self.read_config_file(file=file, follow_externals=options.follow_externals)
        print("-----------------------------------------------------------------")
        print("Finished processing configuration information.")
        print("-----------------------------------------------------------------")
        print(" START - Configuration summary")
        print("-----------------------------------------------------------------")
        self.write_config(stream=sys.stdout)
        print("-----------------------------------------------------------------")
        print(" END - Configuration summary")
        print("-----------------------------------------------------------------")
        #
        if not options.preinstall and not (options.trunk or options.release):
            ##self.sw_packages.insert( 0, Repository('virtualenv', pypi=virtualenv_pypi_string) )
            ##self.sw_packages.insert( 0, Repository('setuptools', pypi=setuptools_string) )
            ##self.sw_packages.insert( 0, Repository('pip', pypi='pip') )
            #
            # Configure the package versions, for offline installs
            #
            for pkg in self.sw_packages:
                pkg.guess_versions()

    def get_packages(self, options):
        #
        # Setup the 'admin' directory
        #
        if not os.path.exists(self.abshome_dir):
            os.mkdir(self.abshome_dir)
        if not os.path.exists(join(self.abshome_dir,'admin')):
            os.mkdir(join(self.abshome_dir,'admin'))
        sys.stdout = open(join(self.abshome_dir,'admin',"virtualenv.cfg"),'w')
        print(options.trunk)
        print(options.release)
        sys.stdout.close()
        sys.stdout = sys.__stdout__
        self.write_config( join(self.abshome_dir,'admin','config.ini') )
        #
        # Setup package directories
        #
        if not os.path.exists(join(self.abshome_dir,'dist')):
            os.mkdir(join(self.abshome_dir,'dist'))
        if not os.path.exists(self.srcdir):
            os.mkdir(self.srcdir)
        if not os.path.exists(self.abshome_dir+os.sep+"bin"):
            os.mkdir(self.abshome_dir+os.sep+"bin")
        #
        # Get source packages
        #
        ##self.sw_packages.insert( 0, Repository('virtualenv', pypi=virtualenv_pypi_string) )
        ##self.sw_packages.insert( 0, Repository('setuptools', pypi=setuptools_string) )
        ##self.sw_packages.insert( 0, Repository('pip', pypi='pip') )

        if options.preinstall:
            #
            # When preinstalling, add the setuptools package to the installation list
            #
            #self.sw_packages.insert( 0, Repository('setuptools',pypi='setuptools') )
            #self.sw_packages.insert( 0, Repository('distribute', pypi='distribute') )
            pass

        for _pkg in options.packages:
            if os.path.exists(_pkg):
                self.sw_packages.append( Repository(_pkg, local=os.path.abspath(_pkg)) )
            else:
                self.sw_packages.append( Repository(_pkg, pypi=_pkg) )
        #
        # Add Pyomo Forum packages
        #
        self.get_other_packages(options)
        #
        # Get package source
        #
        for pkg in self.sw_packages:
            pkg.guess_versions()
            if not pkg.install:
                pkg.find_pkgroot(trunk=options.trunk, release=options.release)
                continue
            if pkg.local:
                tmp = pkg.local
            elif pkg.dev:
                tmp = join(self.srcdir,pkg.name)
            else:
                tmp = join(self.abshome_dir,'dist',pkg.name)
            if options.trunk:
                pkg.install_trunk(dir=tmp, install=False, preinstall=options.preinstall)
            else:
                pkg.install_release(dir=tmp, install=False, preinstall=options.preinstall)
        if not os.path.exists(join(self.abshome_dir,'doc')):
            self.install_auxdirs(options)
        #
        # Create a README.txt file
        #
        sys.stdout = open(join(self.abshome_dir,"README.txt"),"w")
        print(self.README.strip())
        sys.stdout.close()
        sys.stdout = sys.__stdout__
        #
        # Finalize package export
        #
        self.finalize_packages(options)
        #
        # Finalize preinstall
        #
        if options.preinstall:
            print("-----------------------------------------------------------------")
            print(" FINISHED preinstall in directory %s" % self.home_dir)
            print("-----------------------------------------------------------------")
            os.chdir(self.abshome_dir)
            zip_file(self.default_dirname+'.zip', ['.'])
            sys.exit(0)

    def get_other_packages(self, options):
        #
        # Used by subclasses of Installer to
        # add packages that were requested through other means....
        #
        pass

    def finalize_packages(self, options):
        #
        # Perform final steps need to get packages
        #
        pass

    def install_packages(self, options):
        Repository._configureExecutables(self)
        #
        self.get_packages(options)
        #
        # Install the related packages
        #
        for pkg in self.sw_packages:
            if not pkg.install:
                pkg.find_pkgroot(trunk=options.trunk, release=options.release)
                continue
            if pkg.local:
                srcdir = pkg.local
            elif pkg.dev:
                srcdir = join(self.srcdir,pkg.name)
            else:
                srcdir = join(self.abshome_dir,'dist',pkg.name)
            if options.trunk:
                pkg.install_trunk(dir=srcdir, preinstall=options.preinstall)
            else:
                pkg.install_release(dir=srcdir, preinstall=options.preinstall)
        #
        # Localize DOS cmd files
        #
        self.localize_cmd_files(self.abshome_dir, options.localize)
        #
        # Copy the <env>/Scripts/* files into <env>/bin
        #
        if os.path.exists(self.abshome_dir+os.sep+"Scripts"):
            if not os.path.exists(self.abshome_dir+os.sep+"bin"):
                os.mkdir(self.abshome_dir+os.sep+"bin")
            for file in glob.glob(self.abshome_dir+os.sep+"Scripts"+os.sep+"*"):
                shutil.copy(file, self.abshome_dir+os.sep+"bin")
        #
        # Misc notifications
        #
        print("")
        print("-----------------------------------------------------------------")
        print("  Add %s to the PATH environment variable" % (self.home_dir+os.sep+"bin"))
        print("-----------------------------------------------------------------")
        print("")
        print("Finished installation in '%s'" % self.home_dir)

    def localize_cmd_files(self, dir, force_localization=False):
        """
        Hard-code the path to Python that is used in the Python CMD files that
        are installed.
        """
        if not (sys.platform.startswith('win') or force_localization):
            return
        if os.path.exists(dir+os.sep+"Scripts"):
            bindir = 'Scripts'
        else:
            bindir = 'bin'
        for file in self.cmd_files:
            fname = join(dir,bindir,file)
            if not os.path.exists(fname):
                print("WARNING: Problem while localizing file '%s'.  This file is missing" % fname)
                continue
            INPUT = open(fname, 'r')
            content = "".join(INPUT.readlines())
            INPUT.close()
            content = content.replace('__VIRTUAL_ENV__',dir)
            OUTPUT = open(fname, 'w')
            OUTPUT.write(content)
            OUTPUT.close()

    def svnjoin(*args):
        return '/'.join(args[1:])

    def install_auxdirs(self, options):
        for todir,pkg,fromdir in self.auxdir:
            pkgroot = self.sw_dict[pkg].pkgroot
            if options.clear:
                rmtree( join(self.abshome_dir,todir) )
            cmd = [Repository.svn,Repository.svn_get,'-q',self.svnjoin(pkgroot,fromdir),join(self.abshome_dir,todir)]
            print("Running command '%s'" % " ".join(cmd))
            sys.stdout.flush()
            call_subprocess(cmd, filter_stdout=filter_python_develop,show_stdout=True)

    def read_config_file(self, file=None, fp=None, follow_externals=True):
        """
        Read a config file.
        """
        parser = OrderedConfigParser()
        if not fp is None:
            parser.readfp(fp, '<default configuration>')
        elif not os.path.exists(file):
            if not '/' in file and not self.config_file is None:
                file = os.path.dirname(self.config_file)+"/"+file
            try:
                if sys.version_info[:2] <= (2,5):
                    output = urllib2.urlopen(file).read()
                else:
                    output = urllib2.urlopen(file, timeout=30).read()
                    if sys.stdout.encoding is None:
                        output = output.decode('utf-8')
                    else:
                        output = output.decode(sys.stdout.encoding)
            except Exception:
                print("Problems opening configuration url: "+file)
                raise
            fp = StringIO.StringIO(output)
            parser.readfp(fp, file)
            fp.close()
        else:
            if not file in parser.read(file):
                raise IOError("Error while parsing file %s." % file)
        sections = parser.sections()
        if 'installer' in sections:
            for option, value in parser.items('installer'):
                setattr(self, option, apply_template(value, os.environ) )
        if follow_externals and 'externals' in sections:
            for option, value in parser.items('externals'):
                self.read_config_file(file=value, follow_externals=follow_externals)
        if 'localize' in sections:
            for option, value in parser.items('localize'):
                self.add_dos_cmd(option)
        for sec in sections:
            if sec in ['installer', 'localize', 'externals']:
                continue
            if sec.endswith(':auxdir'):
                auxdir = sec[:-7]
                for option, value in parser.items(sec):
                    self.add_auxdir(auxdir, option, apply_template(value, os.environ) )
            else:
                options = {}
                for option, value in parser.items(sec):
                    # NB: option may come back unicode; if it does,
                    # convert it to a string (otherwise, **options can fail)
                    options[str(option)] = apply_template(value, os.environ)
                self.add_repository(sec, **options)

    def write_config(self, filename=None, stream=None):
        if not filename is None:
            OUTPUT=open(filename,'w')
            self.write_config(stream=OUTPUT)
            OUTPUT.close()
        else:
            sys.stdout = stream
            for repos in self.sw_packages:
                repos.write_config(stream)
                print("")
            if len(self.cmd_files) > 0:
                print("[localize]")
                for file in self.cmd_files:
                    print(file+"=")
                print("\n")
            sys.stdout = sys.__stdout__



def configure(installer):
    """
    A dummy configuration function.
    """
    return installer

def create_installer():
    return Installer()

def get_installer():
    """
    Return an instance of the installer object.  If this object
    does not already exist, then create the object and use the
    configure() function to customize it based on the end-user's
    needs.

    The argument to this function is the class type that will be
    constructed if needed.
    """
    try:
        return get_installer.installer
    except:
        get_installer.installer = configure( create_installer() )
        return get_installer.installer

##
## The following functions change the behavior of the virtualenv logic
##

#
# Override the default definition of rmtree, to better handle MSWindows errors
# that are associated with read-only files
#
def handleRemoveReadonly(func, path, exc):
    excvalue = exc[1]
    if func in (os.rmdir, os.remove) and excvalue.errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU| stat.S_IRWXG| stat.S_IRWXO) # 0777
        func(path)
    else:
        raise

def rmtree(dir):
    if os.path.exists(dir):
        logger.notify('Deleting tree %s', dir)
        shutil.rmtree(dir, ignore_errors=False, onerror=handleRemoveReadonly)
    else:
        logger.info('Do not need to delete %s; already gone', dir)

#
# This is a monkey patch, to add control for exception management.
#
vpy_main = main
vpy_main.raise_exceptions=False
def main():
    if sys.platform != 'win32':
        if os.environ.get('TMPDIR','') == '.':
            os.environ['TMPDIR'] = '/tmp'
        elif os.environ.get('TEMPDIR','') == '.':
            os.environ['TEMPDIR'] = '/tmp'
    try:
        vpy_main()
    except Exception:
        err = sys.exc_info()[1] # BUG?
        if vpy_main.raise_exceptions:
            raise
        print("")
        print("ERROR: "+str(err))


#
# This is a monkey patch, to catch errors when a directory cannot be created
# by virtualenv.
#
def mkdir(path):
    if not os.path.exists(path):
        logger.info('Creating %s', path)
        try:
            os.makedirs(path)
        except Exception:
            print("Cannot create directory '%s'!" % path)
            print("Verify that you have write permissions to this directory.")
            sys.exit(1)
    else:
        logger.info('Directory %s already exists', path)

#
# This is a monkey patch to correct the capitalization of "true" set by
# virtualenv
#
default_call_subprocess = call_subprocess
def call_subprocess(cmd, **kwds):
    if kwds.get('extra_env',{}) and \
            kwds.get('extra_env',{}).get('DONT_PATCH_SETUPTOOLS', '') == 'true':
        kwds['extra_env']['DONT_PATCH_SETUPTOOLS'] = 'True'
    return default_call_subprocess(cmd, **kwds)

#
# The following methods will be called by virtualenv
#
def extend_parser(parser):
    installer = get_installer()
    installer.modify_parser(parser)

def adjust_options(options, args):
    installer = get_installer()
    installer.get_homedir(options, args)
    installer.adjust_options(options, args)
    installer.setup_installer(options)
    unzip_wheels()

def after_install(options, home_dir):
    install_wheel(['virtualenv'], Installer.py_executable, options.search_dirs)
    installer = get_installer()
    installer.install_packages(options)

