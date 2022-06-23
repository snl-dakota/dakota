from pygments.style import Style
from pygments.token import Keyword, Name, Comment, String, Error, \
     Number, Operator, Generic

class DakotaStyle(Style):
    default_style = ""
    styles = {
        Comment:                'italic #888',
        Keyword:                'bold #005',
        String:                 'bg:#eee #111'
    }