#  _________________________________________________________________________
#
#  PyUtilib: A Python utility library.
#  Copyright (c) 2008 Sandia Corporation.
#  This software is distributed under the BSD License.
#  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#  the U.S. Government retains certain rights in this software.
#  _________________________________________________________________________
#

try:
    import ConfigParser
except ImportError:
    import configparser as ConfigParser

try:
    from collections import OrderedDict
except ImportError:
    if not '_OrderedDict' in dir():
        from odict import OrderedDict
    else:
        OrderedDict = _OrderedDict


class OrderedConfigParser(ConfigParser.ConfigParser):
    """
    Customization of ConfigParser to (a) use an ordered dictionary and (b)
    keep the original case of the data keys.
    """

    def __init__(self):
        if sys.version_info >= (2,6,0):
            ConfigParser.ConfigParser.__init__(self, dict_type=OrderedDict)
        else:
            ConfigParser.ConfigParser.__init__(self)
            self._defaults = OrderedDict()
            self._sections = OrderedDict()

    def _get_sections(self, fp):
        """
        In old version of Python, we prefetch the sections, to
        ensure that the data structures we are using are OrderedDict.
        """
        if sys.version_info >= (2,6,0):
            return
        while True:
            line = fp.readline()
            if not line:
                break
            line.strip()
            mo = self.SECTCRE.match(line)
            if mo:
                sectname = mo.group('header')
                if not sectname in self._sections:
                    self._sections[sectname] = OrderedDict()
                    self._sections[sectname]['__name__'] = sectname

    def _read(self, fp, fpname):
        """Parse a sectoned setup file.

        This first calls _get_sections to preparse the section info,
        and then calls the ConfigParser._read method.
        """
        self._get_sections(fp)
        fp.seek(0)
        return ConfigParser.ConfigParser._read(self, fp, fpname)

    def optionxform(self, option):
        """Do not convert to lower case"""
        return option
