
def unzip_wheels():
    if not os.path.exists(Installer.abshome_dir):
        os.mkdir(Installer.abshome_dir)
    fname = os.path.join(Installer.abshome_dir, 'virtualenv_support.zip')
    f = open(fname, 'wb')
    f.write(VIRTUALENV_SUPPORT_ZIP)
    f.close()
    unzip_file(fname, dir=Installer.abshome_dir)
    os.remove(fname)

def convert(s):
    import base64
    import zlib
    b = base64.b64decode(s.encode('ascii'))
    return zlib.decompress(b)
    #
    ##return s.decode('base64').decode('zlib')
    #b = base64.b64decode(s.encode('ascii'))
    #return zlib.decompress(b).decode('utf-8')

##file virtualenv_support.zip
VIRTUALENV_SUPPORT_ZIP = convert("""
""")
































