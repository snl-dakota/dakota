# Dakota Sphinx extensions

from docutils import nodes

from sphinx.roles import XRefRole
from docutils.nodes import Element, TextElement
from docutils.utils import unescape
from typing import Optional, Tuple, Type

import pathlib
import re

# References
# https://doughellmann.com/posts/defining-custom-roles-in-sphinx/
# https://protips.readthedocs.io/link-roles.html
# https://stackoverflow.com/questions/50937109/custom-sphinx-role-for-references

def setup(app):
    app.add_role('dakkw', dakota_keyword_role)
    # app.add_role('dakkw2', KwRefRole())
    # TODO: app.add_role('dakex', dakota_example_role)

    # https://www.sphinx-doc.org/en/master/extdev/index.html#extension-metadata
    metadata = { 'parallel_read_safe': True }
    return metadata


def dakota_keyword_role(name, rawtext, text, lineno, inliner, options={}, content=[]):

    # TODO: Separate role or options for 
    #  Linking full text, e.g., variables-normal_uncertain vs. normal_uncertain

    # Lifted this title parsing from sphinx/util/docutil.py as can't
    # get inhertiance approach below to work...

    # \x00 means the "<" was backslash-escaped
    explicit_title_re = re.compile(r'^(.+?)\s*(?<!\x00)<(.*?)>$', re.DOTALL)

    matched = explicit_title_re.match(text)
    if matched:
        has_explicit_title = True
        title = unescape(matched.group(1))
        kw_full = unescape(matched.group(2))
    else:
        has_explicit_title = False
        # Get the trailing keyword name that will be the hyper-linked text
        kw_name = text.split('-')[-1]
        title = unescape(kw_name)
        kw_full = unescape(text)

    # TODO: only works in usingdakota section and only for HTML output
    # Find the document property that other cross-refs use for base URL
    src_file = inliner.document.attributes['source']
    src_path = pathlib.Path(src_file)
    src_parts = src_path.parts
    # src_parts will typically include source/docs/user/usingdakota/.../file.rst

    if (not 'user' in src_parts):
        msg = inliner.reporter.error(
            f'Dakota keyword reference "{rawtext}" can only be used within source/docs/user, not in file {src_file}', line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    # TODO: resolve and validate the reference?
    levels = len(src_parts) - src_parts.index('user') - 2
    rel_uri = ('../' * levels +
              'usingdakota/reference/' + kw_full + '.html')
    ref = nodes.reference(rawtext, title, refuri=rel_uri, **options)
    # Wrap in a literal node for formatting
    literal_ref = nodes.literal('', '', ref)

    return [literal_ref], []


# This approach should let Sphinx resolve the reference, but then
# Dakota customize the title and formatting, but not yet working...
class KwRefRole(XRefRole):
    
    def __init__(self, fix_parens: bool = False, lowercase: bool = False,
                 nodeclass: Optional[Type[Element]] = None,
                 innernodeclass: Optional[Type[TextElement]] = None,
                 warn_dangling: bool = False) -> None:
        super().__init__(fix_parens, lowercase, nodeclass,
                         innernodeclass, warn_dangling)
    
    def process_link(self, env: "BuildEnvironment", refnode: Element, has_explicit_title: bool, title: str, target: str) -> Tuple[str, str]:
        tmp_title, tmp_target = super().process_link(env, refnode, has_explicit_title, title, target)
        abbrev_title = tmp_title.split('-')[-1]
        return abbrev_title, tmp_target
