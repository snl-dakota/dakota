
import os
import sys
import subprocess
import signal

admin_dir = os.path.dirname(os.path.abspath(__file__))
packages=[('admin','acro-admin'), ('tpl','cxxtest','python'), ('packages','*','python')]

def signal_handler(signum, frame):
    pid=os.getpid()
    pgid=os.getpgid(pid)
    if pgid == -1:
        sys.stderr.write("  ERROR: invalid pid %s\n" % (pid,))
        sys.exit(1)
    os.killpg(pgid,signal.SIGTERM)
    sys.exit(1)

signal.signal(signal.SIGINT,signal_handler)
if sys.platform[0:3] != "win":
    signal.signal(signal.SIGHUP, signal_handler)
signal.signal(signal.SIGTERM, signal_handler)


def run():
    trunk = (len(sys.argv) > 1 and '--trunk' in sys.argv)
    if trunk:
        sys.argv.remove('--trunk')
    if '--site-packages' in sys.argv:
        sys.stdout.write("Installing with Python site packages\n")
        sys.argv.remove('--site-packages')
        site_packages=['--site-packages']
    else:
        site_packages=[]
    rc=0
    if not os.path.exists('python'):
        cmd = [ sys.executable,
            os.path.join('tpl','vpykit','bin','install_python'),
            '--logfile',os.path.join('tpl','python.log'),
            '-c', os.path.join(admin_dir,'vpy','dev.ini') ]
        if trunk:
            sys.stdout.write("Installing Python from trunk\n")
            cmd.append('--trunk')
        else:
            sys.stdout.write("Installing Python from cached packages\n")
            cmd.extend(['-z', os.path.join(admin_dir,'vpy','python.zip')])
        cmd.extend(site_packages)
        for package in packages:
            cmd.extend(['-p', os.path.join('%s', *package)])

        #print cmd
        sys.stdout.flush()
        rc = subprocess.call(cmd)
    if rc != 0:
        sys.exit(rc)
    rc = subprocess.call([os.path.join('.','bootstrap','bootstrap'), 'all'])
    if rc != 0:
        sys.exit(rc)
    if len(sys.argv) > 1:
        rc = subprocess.call([
                os.path.join('.','python','bin','python'), 
                os.path.join('python','bin','driver') ] + sys.argv[1:])
    sys.exit(rc)
    
