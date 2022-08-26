.. _git-dakotadescription:

""""""""""""""""""""""
Dakota Git Description 
""""""""""""""""""""""

The primary upstream Git repository for Dakota core, and its open source third-party libraries, are hosted on a public-facing machine software.sandia.gov. The top-level repository includes submodules for public TPLs external, pecos, and surfpack. It also has submodules for SNL-specific content which provide extension points for other external sites to provide their own site-specific packages.

**Dakota Repository Structure**

+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+
| Local clone path             | Upstream URL                                  | Notes                                                                         |
+==============================+===============================================+===============================================================================+
| dakota/                      | - software.sandia.gov:/git/dakota             | Public core Dakota repo                                                       |
|                              | - https://software.sandia.gov/git/dakota      |                                                                               |
+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+
| dakota/packages/external     | - ../dakota-packages                          | Public, relative to facilitate HTTPS vs. Git+SSH                              |
|                              | - (software.sandia.gov:/git/dakota-packages)  |                                                                               |
+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+
| dakota/packages/pecos        | - ../pecos                                    | Public, relative to facilitate HTTPS vs. Git+SSH                              |
|                              | - (software.sandia.gov:/git/pecos)            |                                                                               |
+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+
| dakota/packages/surfpack     | - ../surfpack                                 | Public, relative to facilitate HTTPS vs. Git+SSH                              |
|                              | - (software.sandia.gov:/git/surfpack)         |                                                                               |
+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+
| - *dakota/packages/local/\** | - *SNL Gitlab-Ex*                             | - *SNL TPLs (DOT, NLPQL, NPSOL); other sites can place their copies here too* |
| - *dakota/local*             | - *SNL CEE Gitlab*                            | - *SNL site-specific content*                                                 |
+------------------------------+-----------------------------------------------+-------------------------------------------------------------------------------+

All respoitories have a "devel" branch into which changes are integrated.  An automated process tests changes made to the devel branches before pushing these to the more stable "master" branches. 