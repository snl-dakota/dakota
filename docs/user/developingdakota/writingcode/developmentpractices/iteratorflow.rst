.. _developer-understanding-iterator-flow:

"""""""""""""""""""""""""""
Understanding Iterator Flow
"""""""""""""""""""""""""""

This page explains the various phases comprising Iterator::run_iterator(). Prior to `Iterator <https://dakota.sandia.gov//sites/default/files/docs/6.16.0/html-dev/classDakota_1_1Iterator.html>`_ construction,
when command-line options are parsed, Boolean run mode flags corresponding to PRERUN, RUN, and POSTRUN are set in
`ParallelLibrary <https://dakota.sandia.gov//sites/default/files/docs/6.16.0/html-dev/classDakota_1_1ParallelLibrary.html>`_.
If the user didn't specify any specific run modes, the default is for all three to be true (all phases will execute).

`Iterator <https://dakota.sandia.gov//sites/default/files/docs/6.16.0/html-dev/classDakota_1_1Iterator.html>`_ is constructed.

When called, ``run_iterator()`` sequences:

 - ``initialize_run()``: unconditionally called, virtual. Performs common initialization such as allocating workspaces, setting communicators and evaluation counts. When re-implementing this virtual,
   a derived class must call its nearest parent's initialize_run(), typically *before* performing its own implementation steps.
 - *Not implemented*: pre-run input
 - IF PRERUN, invoke ``pre_run()``: virtual function; default no-op. Purpose: derived classes should implement pre_run() if they are able to generate all parameter sets (variables) at once,
   separate from ``run()``. Derived implementations should call their nearest parent's pre_run(), typically *before* performing their own steps.
 - IF PRERUN, invoke ``pre_output()``: non-virtual function; if user requested, output variables to file.
 - *Not implemented*: run input
 - IF RUN, invoke virtual function run(). Purpose: at a minimum, evaluate parameter sets through computing responses; for iterators without pre/post capability, their entire implementation is in``run()`` and
   this is a reasonable default for new Iterators.
 - *Not implemented*: run output
 - IF POSTRUN, invoke ``post_input()``: virtual function, default only print helpful message on mode. Purpose: derived iterators supporting post-run input from file must implement to read file and populate
   variables/responses (and possibly best points) appropriately. Implementations must check if the user requested file input.
 - IF POSTRUN, invoke ``post_run()``: virtual function. Purpose: generate statistics / final results. Any analysis that can be done solely on tabular data read by ``post_input()`` can be done here. Derived
   re-implementations should call their nearest parent's ``post-run()``, typically after performing their specific post-run activities.
 - *Not implemented*: post-run output
 - ``finalize_run()``: unconditionally called, virtual. Purpose: free workspaces. Default base class behavior is no-op, however, derived implementations should call their nearest parent's finalize_run
   after performing their specialized portions.

`Iterator <https://dakota.sandia.gov//sites/default/files/docs/6.16.0/html-dev/classDakota_1_1Iterator.html>`_ is destructed.
