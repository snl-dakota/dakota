# Dakota Sphinx extensions

from docutils import nodes

from sphinx.roles import XRefRole
from docutils.nodes import Element, TextElement
from typing import Optional, Tuple, Type

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
    #  Custom title, e.g., Normal Uncertain vs. normal_uncertain

    # Get the trailing keyword name that will be the hyper-linked text
    kw_full = text
    kw_name = kw_full.split('-')[-1]

    # TODO: only works in usingdakota section and only for HTML output
    # Find the document property that other cross-refs use for base URL
    src_file = inliner.document.attributes['source']
    if (not '/usingdakota/' in src_file):
        msg = inliner.reporter.error(
            'Dakota keyword reference "%s" can only be used in usingdakota chapter' % text, line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]

    # TODO: resolve and validate the reference?
    rel_source = src_file.split('/usingdakota/', 1)[1]
    levels = rel_source.count('/')
    rel_uri = ('../' * levels +
              'reference/' + kw_full + '.html')

    ref = nodes.reference(rawtext, kw_name, refuri=rel_uri, **options)
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
