""""""""""""""""""
Documenting Dakota
""""""""""""""""""

======================
Documentation Overview
======================

Dakota is documented primarily through

* Dakota User's Guide (Sphinx documentation with source in
  :file:`dakota/docs/user`): primarily focuses on enabling a
  non-expert to use Dakota. Put high-level usage guidelines here, and
  at least a summary of key features and how they benefit a user. The
  Sphinx docs also include:

  - Detailed :ref:`keyword-reference-area`, which draws from metadata
    in :file:`dakota/docs/keywords`

  - :ref:`theory-main`: technical details on use or implementation

  - Developer Guide you're currently reading

* Dakota Examples Library: use case-driven runnable examples and
  tutorials; in separate peer repository ``dakota/dakota-examples``
  and corresponding site-specific SNL-internal repository.

* Dakota C++ Developer Manual: Doxygen format with front matter in
  :file:`dakota/docs/dev`

========================
Principles for Authoring
========================

**Favor flexibility over rigidity**

* This page consists of suggestions and guidelines, not laws. You can
  always deviate from this page's suggestions if there is a good
  reason for it.

**Favor brevity over verbosity**

* This cannot be overstated. Long pages with long paragraphs create
  immense cognitive load for a reader who just wants to figure out how
  to get something to work.

  - Live by the words of Cicero: "If I had more time, I would have
    written a shorter letter."

  - Cross-references are your friend, and will make pages
    shorter. When possible, split topic areas into multiple Sphinx
    pages that are grouped together.

  - As with code, if you catch yourself copy-pasting information into
    multiple pages, it probably means you need to put the information
    in a common area and reference it.

**Favor conventions over exceptions**

* If you find yourself standing on your head to import something into
  the new documentation system, consider whether there are any
  alternate approaches using existing tools/features that Sphinx or
  markdown (either ReStructuredText or Markdown) already provide.

**Favor the best version of duplicated content**

* If information appears in two or more places, favor using the
  version that is either more accurate, more clearly-stated, more
  recent, or more complete (not necessarily longer).

* Don't be afraid to delete one of the versions, but review it to see
  if there are any interesting sentences/thoughts worth adding to the
  "main" page

  - Is there different phrasing or unique perspective? Is there
    information that is missing from the main page that we're keeping?

  - Remember that everything is either under version control, or is
    still in the old manuals, so there's no danger of permanently
    losing a piece of information.

**Favor images and directed instructions over explanatory prose**

* Longer explanations should go into theory manual sections

Favor multiple mediums over text

* If content appears in multiple mediums (i.e. videos and written
  text), duplication is ok because we're offering multiple ways to
  learn something.

**Favor expertise when editing**

* Take ownership of pages you are confident about editing - no need to
  do a formal review process for small changes or changes you can
  speak with authority on.

* If in doubt about updating a page or deleting from a page, talk to
  the SME for that page's topic.






