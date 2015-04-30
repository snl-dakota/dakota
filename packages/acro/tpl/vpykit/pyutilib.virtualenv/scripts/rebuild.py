#!/usr/bin/env python
"""
Helper script to rebuild wheels.py using a preinstall ZIP file.
"""

# This script is adapted from the virtualenv rebuild-script.py file.
# That script is distributed with the following license:
"""
Copyright (c) 2007 Ian Bicking and Contributors
Copyright (c) 2009 Ian Bicking, The Open Planning Project
Copyright (c) 2011 The virtualenv developers

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
"""

import re
import os
import sys

here = os.path.dirname(__file__)
script = os.path.join(here, sys.argv[1])
target = sys.argv[2]

file_regex = re.compile(
    r'##file (.*?)\n([a-zA-Z][a-zA-Z0-9_]+)\s*=\s*convert\("""\n(.*?)"""\)',
    re.S)
file_template = '##file %(filename)s\n%(varname)s = convert("""\n%(data)s""")\n'

def rebuild():
    f = open(script, 'rb')
    content = f.read()
    f.close()
    parts = []
    last_pos = 0
    match = None
    for match in file_regex.finditer(content):
        parts.append(content[last_pos:match.start()])
        last_pos = match.end()
        filename = match.group(1)
        varname = match.group(2)
        data = match.group(3)
        print('Found reference to file %s' % filename)
        try:
            # Check the current working directory first
            f = open(filename, 'rb')
        except IOError:
            # fallback on the script directory 
            f = open(os.path.join(here, filename), 'rb')
        c = f.read()
        f.close()
        new_data = c.encode('zlib').encode('base64')
        if new_data == data:
            print('  Reference up to date (%s bytes)' % len(c))
            parts.append(match.group(0))
            continue
        print('  Content changed (%s bytes -> %s bytes)' % (
            zipped_len(data), zipped_len(new_data)))
        new_match = file_template % dict(
            filename=filename,
            varname=varname,
            data=new_data)
        parts.append(new_match)
    parts.append(content[last_pos:])
    new_content = ''.join(parts)
    if new_content != content:
        sys.stdout.write('Content updated; overwriting... ')
        f = open(target, 'wb')
        f.write(new_content)
        f.close()
        print('done.')
    else:
        print('No changes in content')
    if match is None:
        print('No variables were matched/found')

def zipped_len(data):
    if not data:
        return 'no data'
    try:
        return len(data.decode('base64').decode('zlib'))
    except:
        return 'unknown'

if __name__ == '__main__':
    rebuild()
    

