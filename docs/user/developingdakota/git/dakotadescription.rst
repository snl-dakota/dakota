.. _git-dakotadescription:

""""""""""""""""""""""
Dakota Git Description 
""""""""""""""""""""""

The authoritative upstream Git repositories for Dakota core, and its
open source third-party libraries, are hosted on SNL-internal Gitlab
repositories. They are mirrored to repositories at
https://github.com/snl-dakota, which is used here to describe the
overall structure. The top-level repository includes submodules for
public TPLs external, pecos, and surfpack. It also has submodules for
SNL-specific (private) content which provide extension points for
other external sites to provide their own site-specific packages.

**Dakota Repository Structure**

+------------------------------+--------------------------------------------------+--------------------------------------------------+
| Local clone path             | Upstream URL                                     | Notes                                            |
+==============================+==================================================+==================================================+
| dakota/                      | https://github.com/snl-dakota/dakota             |                                                  |
+------------------------------+--------------------------------------------------+--------------------------------------------------+
| dakota/packages/external     | ../dakota-packages                               | Public, relative to facilitate HTTPS vs. Git+SSH |
|                              | (https://github.com/snl-dakota/dakota-packages)  |                                                  |
+------------------------------+--------------------------------------------------+--------------------------------------------------+
| dakota/packages/pecos        | ../pecos                                         | Public, relative to facilitate HTTPS vs. Git+SSH |
|                              | (https://github.com/snl-dakota/pecos)            |                                                  |
+------------------------------+--------------------------------------------------+--------------------------------------------------+
| dakota/packages/surfpack     | ../surfpack                                      | Public, relative to facilitate HTTPS vs. Git+SSH |
|                              | (https://github.com/snl-dakota/surfpack)         |                                                  |
+------------------------------+--------------------------------------------------+--------------------------------------------------+
| dakota/dakota-examples       | ../dakota-examples                               | Public, relative to facilitate HTTPS vs. Git+SSH |
|                              | (https://github.com/snl-dakota/dakota-examples)  |                                                  |
+------------------------------+--------------------------------------------------+--------------------------------------------------+
| - *dakota/packages/local/\** | - *SNL Gitlab-Ex*                                | - *SNL TPLs (DOT, NLPQL, NPSOL);                 |
|                              |                                                  |   other sites can place their copies here too*   |
| - *dakota/local*             | - *SNL CEE Gitlab*                               | - *SNL site-specific content*                    |
+------------------------------+--------------------------------------------------+--------------------------------------------------+

Most repositories have a "devel" branch into which changes are
integrated (dakota-examples only uses master and topic branches).  An
automated process tests changes made to the devel branches before
pushing these to the more stable "master" branches.
