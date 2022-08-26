from pygments.lexer import RegexLexer
from pygments.token import *
from sphinx.highlighting import lexers

class DakotaLexer(RegexLexer):
    name = 'Dakota'
    aliases = ['dakota']
    filenames = ['*.in']

    tokens = {
        'root': [
            (r' .*', Text),
            (r'(environment|method|model|variables|interface|responses)', Keyword),
            (r'#.*\n', Comment),
            (r'((\'.+\')|(\".+\")|(’.+’))', String),
            (r'.*\n', Text)
        ]
    }
    
lexers['dakota'] = DakotaLexer(startinline=True)