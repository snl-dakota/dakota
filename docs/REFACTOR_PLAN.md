# Plan: Refactor Dakota's Introductory Documentation

*Revision 5: incorporates the team's answers on terminology, driver language, running example, the install/getting-started split, and diagram format (see §3); adds a signposted mention of the in-memory Python direct interface (Steps 5 and 7); makes the intro format-neutral, with JSON input shown first and freeform second (Decision 6), and JSON no longer called "experimental"; and standardizes analysis-driver tabs as Python / Bash / Windows (PowerShell) (Decision 7).*

**Goal.** Make the first few pages of the Dakota user docs tell the same story the team tells in its standard intro talk:

1. **What it is:** Dakota runs *ensembles* of a *black-box* simulation, picking parameter values, managing the runs, and analyzing the outputs.
2. **Why use it:** *build one interface to your code, then use many methods* (sensitivity analysis, UQ, optimization, calibration, parameter studies).
3. **How a study is put together:** the six input-file blocks (environment, method, model, variables, interface, responses) and how they relate. This structure is the same whether the input file is written in the **JSON** format or the **freeform** format.
4. **How you connect your code:** the **analysis driver** (pre-process → run → post-process), sitting between Dakota's parameters file and results file. It can be a script (Python, bash, …) or built in the GUI.

Source: `docs/user/` in `snl-dakota/dakota` (`devel` branch), Sphinx + `sphinx_rtd_theme`, reStructuredText.

---

## 1. Where the docs stand now

### Reader path today
`index.rst` (splash) → `setupdakota.rst` (Getting Started = download tables + install + env) → `usingdakota/usingdakota.rst` → `introduction/aboutdakota.rst` → `introduction/helloworld.rst` → `examples.rst` → `introduction/couplingtosimulations.rst` → `inputfile.rst` → …

### Gaps compared with the slides
| Slide message | Current docs |
|---|---|
| "Dakota enhances simulations by analyzing ensembles" (slide 2: run table, SPICE/Abaqus loops) | Not stated anywhere. `aboutdakota.rst` opens with "delivers both state-of-the-art research and robust, usable software". |
| "Interface once, apply many methods" (slides 5, 9, 10) | Only implied, in one sentence on the coupling page. The 4-quadrant methods table doesn't exist. |
| Six blocks with one snippet each (slides 11–20) | Exists as a dense prose wall in `inputfile.rst`, three levels deep and *after* the tutorial and examples. The "information flow" figure (slide 20) is missing. |
| Analysis driver (pre-process / execute / post-process) | Explained in `couplingtosimulations.rst`, but it isn't linked from the splash page and there's no short Python driver to copy. |
| Input-file syntax (newest release highlights JSON input) | Every beginner page shows only freeform syntax. JSON is covered only on `inputfile/json.rst`, a reference-style page written as "how JSON differs from freeform", which assumes the reader already knows freeform. |
| Splash page | Title is lowercase "dakota", a 2-sentence blurb, a "Read More..." link and a bare toctree. No value proposition, no figure, no "start here" paths. |
| History/pedigree (slide 3) | `aboutdakota.rst` has four long sections (Summary / Intended Use / Motivation / History / C++ Toolkit) that mostly repeat each other. |

### Concrete defects found while surveying (quick wins)
- `helloworld.rst`: the run command reads `dakota -i rosen multidim.in -o rosen multidim.out > rosen multidim.stdout` (spaces instead of underscores), so a newcomer's very first command fails.
- `helloworld.rst`: item 4 runs the restart-file explanation into the output-file explanation.
- `setupdakota.rst`: the macOS PATH hardcodes `Dakota_UI_6.19.0.app`. The CLI table lists 6.24.0 while the GUI tables stop at 6.23.0. The page is mostly download links, so "Getting Started" is a misleading title.
- `index.rst`: still has the sphinx-quickstart boilerplate comment, and the H1 is lowercase.
- `conf.py`: `version`/`release` are empty, so `|version|` can't be used to avoid hardcoded version numbers.
- `inputfile/json.rst`: the "Top-level Organization" list says the required keys are "``interface``,``responses``, and ``interface``" (it should be variables, interface, responses; the missing space also breaks the RST). There are also typos ("exapmle", "preceeding").
- `dakota-examples/official/drivers/windows_bat/simulator_script.bat` (a candidate source for a Windows driver): it hardcodes `c:\dakota\6.13\`, misspells the variable as `%dakota_intall%`, and uses `%1%` instead of `%1`. Don't copy it as-is; the Windows tab uses PowerShell instead (Decision 7).

### Reusable material already in the repos
- `docs/user/usingdakota/img/InputBlocks.png` (block hierarchy) and `img/CouplingSimulations1.png` (black-box loop). Replace both with TikZ versions (Step 4).
- `inputfile/interfaces/dakotainterfacing.rst`: full docs for the `dakota.interfacing` Python module (`read_parameters_file`, `dprepro`, results writing).
- `inputfile/interfaces/buildingblackboxinterface.rst`: the dprepro/template workflow.
- JSON input: `inputfile/json.rst` (label `jsoninput`) already has a JSON Rosenbrock example, plus `dakota -json <file>` (`running/commandline.rst`) and `dakota_parser <freeform file> --json` for converting existing inputs. `test/` already has JSON-format regression inputs (for example `dakota_uq_textbook_lhs_json.json`), so there is a pattern for testing JSON samples.
- **Rosenbrock, three ways** (the running example; see Decision 3):
  - *Direct (built-in):* `rosen_multidim.in` and the other `test/examples-users/rosen_*.in.sav`. These need no external code, are already regression-tested, and are already pulled into `docs/user/usingdakota/samples/`.
  - *Black box with a Python driver:* `dakota-examples/official/drivers/Python/di/` (`driver.py`, `rosenbrock.py`, `dakota_rosenbrock_python.in`) uses `dakota.interfacing`. It's tested via the examples repo's `.base` files. **Caveat:** `driver.py` handles gradients/Hessians with active-set-vector (ASV) bitmasks, which is too much for a first example (see Step 9).
  - *Black box mimicking an engineering code:* `dakota-examples/official/drivers/bash/` (`simulator_script.sh`, `templatedir/ros.template`, `templatedir/rosenbrock_bb.py`). `rosenbrock_bb.py` reads a fake "engineering" input deck (`ros.in`, with nodes/elements/materials) and writes a log (`ros.out`). That makes it a realistic black box for showing template → run → parse. (Note: the script calls `dprepro3`. Check whether the current name is `dprepro`.)
  - *Compiled:* `rosen_syscall.in` with `test/rosenbrock.cpp`. It's currently shown on the coupling page, but needs a compiler, so it's better as a "deep dive" than a first contact.
- Cantilever (`dakota-examples/training/drivers/cantilever/`, `official/drivers/black-box_simulation/`): keep it as the *follow-on* "realistic engineering" example the tutorial links to, matching the slides' later material.
- Already enabled in `conf.py`: `sphinx_inline_tabs` (Python/bash/PowerShell driver tabs) and `sphinx_copybutton`. `sphinx.ext.imgmath` is also enabled, so LaTeX is already part of the docs toolchain.

---

## 2. Target reader journey

Organized around [Diátaxis](https://diataxis.fr/), which keeps tutorials, how-to guides, explanation and reference apart so each page has one job. Per Decision 4, **downloading and installing live in their own section, separate from Getting Started.**

```
index.rst  (splash: value prop + figure + "Start here" paths)
│
├── Download & Install                               ← separate section (Decision 4)
│   ├── Download Dakota               (reference)    ← release tables, moved out of setupdakota.rst
│   └── Install Dakota                (how-to)       ← extract, PATH/PYTHONPATH, verify
│
├── Getting Started with Dakota                      ← new grouping
│   ├── 1. What is Dakota?            (explanation)  ← rewrite of aboutdakota.rst
│   ├── 2. Anatomy of a Dakota Study  (explanation)  ← NEW, slides 11–20
│   ├── 3. Connecting Your Code: The Analysis Driver ← rewrite of couplingtosimulations.rst (top half)
│   ├── 4. Tutorial: Your First Study (tutorial)     ← helloworld.rst, extended to a black-box Rosenbrock
│   └── 5. Choosing a Method          (explanation)  ← NEW, goal → method map (slide 5)
│
└── Using Dakota (existing, mostly unchanged)
    ├── Examples, Input File, Interfaces (deep dive), Running, Output, Study Types, …
```

Each Getting Started page ends with a **"Next:"** link to the following page. The tutorial opens with "Prerequisite: :ref:`Install Dakota <setupdakota-installation>`". **Rosenbrock** is the running example on every Getting Started page; the cantilever is offered at the end as the next, more realistic step.

---

## 3. Decisions (resolved)

| # | Question | Decision | Effect on the plan |
|---|---|---|---|
| 1 | What to call the interface code | **"Analysis driver."** It covers script drivers (Python, bash, …) and GUI-built drivers (Next-Gen Workflow). | Used on every page and in the glossary. It also matches the `analysis_drivers` keyword, so there's no second term to explain. When a page talks specifically about a script, write "an analysis driver written as a Python script", not "driver script". |
| 2 | Primary driver language | **Python first**, using `dakota.interfacing`. That's the modern recommended approach. | Steps 7 and 9 show Python first; Bash and Windows (PowerShell) appear as the other tabs (Decision 7). A GUI note points to Next-Gen Workflow. |
| 3 | Running example | **Rosenbrock**, not the cantilever. It's simpler (2 variables, 1 response) and already has both direct and black-box versions. | The tutorial goes from direct Rosenbrock to black-box Rosenbrock. Block snippets on the anatomy page use Rosenbrock. The cantilever becomes the "next step" example. |
| 4 | Download/install vs Getting Started | **Separate.** Download/Install is its own section; Getting Started is about understanding and using Dakota. | Step 10 creates the Download & Install section; Getting Started doesn't include install steps, only a prerequisite link. |
| 5 | Diagram format | **Native, text-based source: TikZ.** Source is kept in the repo and rendered to SVG (HTML) and PDF (LaTeX builds). A prototype is already verified (see Step 4). | Step 4 is now "author TikZ sources + render pipeline". |
| 6 | Input-file syntax | **Format-neutral concepts; JSON shown first.** The six-block structure is what newcomers learn. Syntax is a presentation detail: every input snippet appears in synced tabs, **JSON** first and **Freeform** second. No page teaches freeform-specific syntax rules (abbreviations, `N*value` shorthand, ordering flexibility) in Getting Started. | Steps 4, 6, 7, 9, 11 and 13 change (see below). New ground rule. The tutorial ships and tests each input in both formats. |
| 7 | Analysis-driver tabs | **Three synced tabs: `Python` / `Bash` / `Windows (PowerShell)`**, in that order, wherever an analysis driver or its interface setting appears. | Steps 7 and 9 provide all three drivers. Each is tested and shipped with the tutorial example. |

### Input-format presentation (Decision 6 detail)
- **Teach structure, not syntax.** Explain blocks and keywords as a *tree*: a block contains keywords, and keywords can have values and child keywords. That description fits both formats: JSON shows the tree as nested objects, and freeform as keyword sequences. Put this explanation once, on the anatomy page.
- **Synced tabs everywhere.** Use `sphinx_inline_tabs` (already enabled) with the exact labels `JSON` and `Freeform` on every snippet. Tabs with the same label switch together across a page, so a reader who picks Freeform stays in Freeform.
- **Name the formats consistently:** "JSON format" and "freeform format", the names `json.rst` already uses. Avoid "legacy" or "traditional", which suggest freeform is deprecated; both formats are supported.
- **Run commands come in pairs:** `dakota -json study.json` and `dakota -i study.in`, in the same tabs.
- **One short "Two formats, one structure" box** on the anatomy page: both formats are supported and describe the same study, JSON is easier for scripts and tools to generate and validate, and `dakota_parser <file> --json` converts freeform to JSON. Link to `jsoninput` and `inputfile/formatting`.
- **JSON is a fully supported format.** Don't call it "experimental", "new" or "beta" anywhere in the intro or in `json.rst`. (The 6.24 release notes called it experimental; those are a historical record and stay as they are.) Because the tutorial leads with JSON, Step 9's tests make sure every tutorial input runs correctly in JSON.
- **Rewrite `json.rst` lightly** (Step 12) so it stands on its own: open with the JSON format itself, and move the "how it differs from freeform" material to a section near the end for users converting existing files.

### Analysis-driver tabs (Decision 7 detail)
- **Exact labels**, so they sync across pages and stay independent of the `JSON`/`Freeform` tabs: `Python`, `Bash`, `Windows (PowerShell)`.
- **Same three jobs in every tab.** Each driver does the same pre-process → run → post-process steps on the same black-box Rosenbrock (`ros.template` → `ros.in` → `rosenbrock_bb.py` → `ros.out` → results file), with the same comment headers (`PRE-PROCESSING` / `EXECUTION` / `POST-PROCESSING`), so readers can compare the tabs line by line.
  - **Python:** `dakota.interfacing` (`read_parameters_file`, `dprepro`, `subprocess`, results `.write()`).
  - **Bash:** `dprepro` CLI, `grep`/`awk` to extract the value, `echo` to the results file. Adapt `official/drivers/bash/simulator_script.sh`, and check `dprepro` vs `dprepro3`.
  - **Windows (PowerShell):** `dprepro` CLI, `Select-String` to extract the value, `Set-Content` for the results file. Use the pattern in the cantilever `driver.ps1`. Prefer PowerShell to `.bat`: it's on every supported Windows version, and its text handling is far easier to read. The existing `.bat` example is outdated (see §1).
- **Interface setting per tab.** How the driver is invoked differs by platform, so the interface snippet nests the driver tabs inside each format tab (or shows a small three-row table): `'python3 driver.py'` (or `'driver.py'` with a shebang on Linux/macOS), `'driver.sh'`, and `'powershell.exe -File driver.ps1'` (as in `dakota_cantilever_center.in`). If nested tabs get cluttered, show only the `analysis_drivers` line per driver tab, below a single format-tabbed interface block.
- **Run commands** (copying files, running the driver by hand, running Dakota) also get Linux/macOS vs Windows tabs where they differ, such as `cp` vs `Copy-Item` and `./driver.sh params.in results.out` vs `powershell -File driver.ps1 params.in results.out`.
- **Testing:** all three drivers ship with the tutorial example. The Python and Bash drivers are regression-tested on Linux. The PowerShell driver is tested on Windows CI if available; otherwise it's on a manual pre-release checklist (Step 13).

### Diagram rendering approach (Decision 5 detail)
TikZ works. There are two ways to wire it in:

- **A. Pre-rendered (recommended).** Commit the `.tex` sources under `docs/user/img/src/` plus a small `render_figures.sh` (or a CMake target). The script runs `pdflatex` on each standalone file, then `pdftocairo -svg` (or `dvisvgm --pdf`), and commits both the `.svg` and `.pdf`. Pages use `.. figure:: /img/intro/study_blocks.*`; the wildcard makes Sphinx choose SVG for HTML and PDF for LaTeX/PDF builds. Ordinary doc builds (CI, contributors, Read-the-Docs-style hosts) need no TeX or `pdf2svg`. The source is still the single source of truth, and a CI check can re-render the figures and fail if the committed SVGs are stale.
- **B. Rendered at build time with `sphinxcontrib-tikz`.** It provides `.. tikz::` (inline or `:include:` a file). Adds the extension plus LaTeX + `pdf2svg` (or ImageMagick/netpbm) as hard build dependencies, and slows builds. Choose this only if the team wants diagram code inline in the `.rst` files.

**Prototype verified:** `study_blocks.tex` (Environment ⊃ Method → Model → {Variables, Interface, Responses}) compiles with `pdflatex` + `pdftocairo -svg` using the `standalone` class with the `positioning`, `fit` and `arrows.meta` libraries. It's attached alongside this plan as the template for the other figures.

---

## 4. Ground rules for every agent (paste into each task prompt)

```
Context: Dakota user docs, Sphinx + sphinx_rtd_theme, reStructuredText, in docs/user/.
- Do NOT edit generated directories: docs/user/usingdakota/reference/ and docs/user/usingdakota/samples/.
- Preserve existing labels that other pages reference: aboutdakota-main, helloworld-main,
  couplingtosimulations-main, setupdakota-main, setupdakota-download, setupdakota-installation,
  setupdakota-environment, inputfile-main, studytypes-main, examples-gettingstarted-main.
  If you move content, move its label with it. If you retire a page, leave a short stub
  that keeps the label and links to the new location. grep -rn for every label you touch.
- Also update docs/user/classictoc.rst when titles or labels change.
- Terminology: always "analysis driver" (it covers script drivers AND GUI/Next-Gen Workflow
  drivers). Never "driver script" or "simulation interface script". Responses = "quantities
  of interest (QoIs)". Link first uses to the glossary with :term:`analysis driver`.
- Running example: Rosenbrock's function (variables x1, x2; one response). Use the built-in
  (direct) Rosenbrock when no external code is needed, and the black-box Rosenbrock
  (rosenbrock_bb.py + ros.template) when demonstrating analysis drivers. Do not introduce
  the cantilever or other problems on Getting Started pages except as a "next step" link.
- "Black box" means Dakota needs no knowledge of your code's internals; it does NOT mean
  "communicates through files". Don't write sentences implying file I/O is the only way to
  connect. On Getting Started pages, the in-memory Python direct interface (`interface python`)
  is mentioned only in the designated "Other ways to connect" spots (Steps 5 and 7).
- Driver examples: synced `.. tab::` blocks (sphinx_inline_tabs) labeled exactly `Python`
  (dakota.interfacing, FIRST), `Bash`, `Windows (PowerShell)`. All three do the same
  pre-process / run / post-process steps with the same section comments. Don't use .bat.
  Show the matching `analysis_drivers` value for each tab. Mention the GUI (Next-Gen
  Workflow) as an alternative way to build an analysis driver.
- JSON is a fully supported format: never call it "experimental", "beta" or "new".
- Input formats: describe blocks and keywords in format-neutral terms. Every Dakota input
  snippet appears in synced `.. tab::` blocks labeled exactly `JSON` (first, ``.. code-block:: json``)
  and `Freeform` (second, ``.. code-block:: dakota``). Every run command appears in both forms
  (`dakota -json x.json` / `dakota -i x.in`). Call the formats "JSON format" and "freeform format"
  (never "legacy" or "traditional"). Don't teach freeform-only conveniences (abbreviations,
  aliases, N*value, L:S:U) on Getting Started pages. Generate JSON from a working freeform file
  with `dakota_parser <file> --json` rather than writing it by hand, and check both versions.
- Use ``literalinclude`` from samples/ when a snippet is a full, tested input file.
- Use the :dakkw: role (defined in docs/user/_extensions/sphinx_dakota.py; see existing
  usages) to link keywords to the reference instead of hardcoding URLs.
- Figures: TikZ sources in docs/user/img/src/*.tex, rendered with docs/user/img/src/render_figures.sh
  into docs/user/img/intro/*.svg and *.pdf; reference them as `.. figure:: /img/intro/<name>.*`.
  Every figure needs :alt: text.
- No hardcoded version numbers or "N years old" claims in prose.
- Short paragraphs, second person ("you"), present tense, one idea per section.
- End every Getting Started page with a "Next:" link to the next page in sequence.
- Before finishing: build the docs (see Step 1 for the command) and confirm you added no new
  warnings. Run every complete input file you wrote through `dakota -check -i <file>.in` and
  `dakota -check -json <file>.json`.
- Deliver as one focused commit/PR per step, with a short description of what changed and why.
```

---

## 5. Work steps

Each step is sized to be one agent task / one PR. **Waves** show what can run in parallel.

### Wave 1: Foundations (all parallel)

#### Step 1. Baseline the build
- **Task:** Get the Sphinx build running standalone, from `docs/user` with the dependencies in `docs/pyproject.toml`, or via the CMake `docs-user` target if generated `reference/` and `samples/` are required. Record the exact build command, a warning count, and a `sphinx-build -b linkcheck` report in `docs/user/README_BUILD.md` (or reuse an existing dev doc).
- **Acceptance:** A reproducible one-line build command; a baseline warnings list; linkcheck results for the pages in scope.
- **Why first:** Every later step's "no new warnings" check depends on it.

#### Step 2. Quick-fix defects
- **Files:** `helloworld.rst`, `setupdakota.rst`, `index.rst`, `conf.py`, `inputfile/json.rst`.
- **Task:** Fix the defects listed in §1: the command typo, the merged list item 4, the hardcoded `Dakota_UI_6.19.0` path (replace with a placeholder plus a note), the quickstart boilerplate comment, the lowercase H1, and the `json.rst` key-list error and typos. Also populate `version`/`release` in `conf.py` from `DakotaVersion.cmake` or `pyproject.toml`.
- **Acceptance:** A small, low-risk PR that can merge right away, independent of the redesign.

#### Step 3. Glossary and terminology page
- **New file:** `docs/user/misc/glossary.rst`, using Sphinx `.. glossary::` so pages can link terms with `:term:`.
- **Terms:** study, ensemble, black-box simulation, **analysis driver** (define it as *the program Dakota runs for each evaluation to map parameters to responses; it can be a script, such as Python or bash, or a workflow built in the Dakota GUI*), parameters file, results file, template file / dprepro, pre-processing, post-processing, variables (design / uncertain [aleatory, epistemic] / state), responses / QoIs, method, model, interface, environment, evaluation, evaluation concurrency, tabular data file, restart file, surrogate, **JSON format** / **freeform format** (the two supported input-file syntaxes for the same study structure), `dakota_parser` (converts freeform to JSON), **black box** (Dakota treats the code as a mapping from parameters to responses and needs nothing else about it, however the data is exchanged), **fork/system interface** (file-based exchange through an analysis driver), **direct interface** (built-in test functions such as Rosenbrock, or in-memory calls), **Python direct interface** (`interface python`: Dakota calls a function in your Python module and passes data in memory, with no parameters/results files), library API (embedding Dakota in another C++/Python application).
- **Acceptance:** Every term in one or two sentences, linking to the deeper page. Added to the `misc` toctree.

#### Step 4. Figures in TikZ (+ render pipeline)
- **New files:** `docs/user/img/src/*.tex` (TikZ, `standalone` class), `docs/user/img/src/render_figures.sh`, and the outputs `docs/user/img/intro/*.svg` + `*.pdf`. Start from the attached `study_blocks.tex` prototype and keep one shared style (colors, fonts, node styles) by putting the `\tikzset{...}` definitions in `docs/user/img/src/dakotastyle.tex` and `\input`-ing it from each figure.
- **Figures** (matching the slides' content, not necessarily their look):
  1. `ensemble_loop`: Dakota ⇄ simulation, with "variables in → responses out" and a small run table (run, x1, x2, f) using Rosenbrock values (slides 2, 9).
  2. `one_interface_many_methods`: one analysis driver connecting to a fan of method families (SA, UQ, optimization, calibration, parameter studies) (slides 5, 10).
  3. `study_blocks`: Environment ⊃ Method → Model → {Variables, Interface, Responses} (slide 19). **Prototype done.** It replaces `InputBlocks.png`.
  4. `information_flow`: input file → [Environment: Method ⇄ Model(Variables → Interface → Responses)] → output files (console, tabular, restart) (slide 20).
  5. `analysis_driver_flow`: params file → [pre-process (`ros.template` + dprepro → `ros.in`) → run `rosenbrock_bb.py` → post-process (parse `ros.out`)] → results file. It replaces `CouplingSimulations1.png`.
- **Script:** for each `*.tex`, run `pdflatex -interaction=nonstopmode -halt-on-error`, then `pdftocairo -svg`, and copy the PDF alongside. Fail loudly on errors. Optionally add a CI job that re-renders the figures and runs `git diff --exit-code` on the outputs.
- **Format-neutral figures:** figures show block and keyword *names* only, never freeform or JSON syntax, so they stay valid for both formats. Figure 4 labels the input as "Dakota input file (JSON or freeform)".
- **Acceptance:** All five figures readable at 700 px wide, labels at least about 10 pt equivalent, consistent style, alt text drafted in the PR description.

### Wave 2: Core concept pages (parallel; need Steps 3–4 merged or at least drafted)

#### Step 5. Rewrite "What is Dakota?" (`introduction/aboutdakota.rst`)
- **Keep label** `aboutdakota-main`. Change the title to "What is Dakota?".
- **Structure:**
  1. One-paragraph definition (slides 1, 7): open-source (LGPL) software from Sandia; algorithms applied to computational simulations in **black-box** fashion for UQ, optimization, calibration and sensitivity analysis.
  2. **Dakota analyzes ensembles** (slide 2) plus the `ensemble_loop` figure: it strategically selects parameter values, manages simulation runs and analyzes responses. Mention the SPICE/Abaqus-style examples of "parameters in, responses out" to show the range of codes.
  3. **Build one interface, use many methods** (slides 5, 9) plus the `one_interface_many_methods` figure. Introduce the term **analysis driver** here (link to the glossary). Include the 4-quadrant method table (SA / UQ / Optimization / Calibration) as a compact list-table linking to `studytypes/*`.
  4. **Why use Dakota** (slide 4): practical for costly models, combined deterministic/probabilistic analysis, handles non-smooth or multimodal responses, surrogate and multifidelity methods, parallel from desktop to HPC, production software with regular releases (May 15 / Nov 15).
  5. **Questions Dakota helps answer:** keep the "What is the best design? / How safe is it? / How much confidence…" bullets from the current page.
  6. **Beyond the black box** (2–3 sentences, placed after the core message, not before it): the black-box, file-based approach works with nearly any code, so it's where most users start. If your model is in Python, Dakota can also call it directly and pass values in memory, which avoids file I/O and enables tighter, "grey-box" coupling, such as returning analytic gradients. Dakota can also be embedded as a library. Link to Step 7's "Other ways to connect" section.
  7. **Pedigree**, 3–4 sentences (slide 3). Move the long History / Motivation / C++ Toolkit text to a new `misc/history.rst` and link to it.
- **Length target:** about 1–1.5 screens of reading before the "Next:" link.

#### Step 6. NEW "Anatomy of a Dakota Study" (`introduction/studyanatomy.rst`, label `studyanatomy-main`)
- **Content (slides 11–20):**
  - Intro: a study is described by an input file made of six blocks, with the `study_blocks` figure and the key sentence: *"In each iteration of its algorithm, a method block requests a variables-to-responses mapping from its model, which the model fulfills through an interface."*
  - **"Two formats, one structure"** (right after the intro, before the block sections): the format-neutral tree explanation and the short box described under Decision 6. Show one tiny block (for example `variables` with `continuous_design`) side by side in both formats, so the reader sees the mapping once. After that, the tabs do the work.
  - One short section per block, in this order: **Environment, Method, Model, Variables, Interface, Responses**. Each has: what it's for (1–2 sentences), required/optional and how many are allowed, a short **Rosenbrock** snippet in synced `JSON` / `Freeform` tabs, and a "Learn more" link to the existing deep page (`inputfile/variables`, `inputfile/interfaces`, …) and the keyword reference.
    - Environment: `tabular_data`, plus restart/results output (adapted from slide 12).
    - Method: `multidim_parameter_study` (as in `rosen_multidim.in`), plus a one-line note that you can swap in `sampling`, `optpp_q_newton`, etc.
    - Model: `single`, with a note that it's optional in simple studies.
    - Variables: `continuous_design` with a count of 2 (x1, x2, bounds). Add a small table of the three classification axes (design / aleatory / epistemic / state × continuous/discrete × real/integer/string), saying which method families use which. A second tiny snippet shows Rosenbrock with `uniform_uncertain` variables, to make clear that the same problem can be studied as design or uncertain.
    - Interface: show **both** `direct` with the built-in `rosenbrock` driver and `fork` with a `python3 driver.py` analysis driver (black box), side by side. This is the hook into the next page.
    - Responses: `objective_functions = 1` vs `response_functions = 1`, plus `no_gradients` / `numerical_gradients`.
  - **Overall information flow** (slide 20) with the `information_flow` figure: input file → Dakota → repeated evaluations → console, tabular and restart output.
  - The complete assembled input in both formats via `literalinclude`: `samples/rosen_multidim.in` already exists and is tested; the JSON twin comes from Step 9's example work.
  - Wording: avoid "keyword block ends at the next block keyword" or any other freeform parsing rules. Required/optional and how many are allowed are stated as facts about the study, which hold in both formats.
- **Also edit `inputfile.rst`:** cut the long block-by-block prose there down to a short "See :ref:`studyanatomy-main` for an overview" plus the existing toctree (keep `json` and `formatting` side by side in it, as the two syntax pages), so the two pages don't duplicate each other. Move any detail the new page drops into the relevant `inputfile/*.rst` page rather than deleting it.

#### Step 7. Rewrite "Connecting Your Code: The Analysis Driver" (`introduction/couplingtosimulations.rst`)
- **Keep label** `couplingtosimulations-main`. Title: "Connecting Your Code: The Analysis Driver".
- **Structure:**
  1. **The idea:** Dakota knows nothing about your code. You provide *one* **analysis driver** that Dakota runs once per evaluation, with two arguments: the parameters-file path and the results-file path. Analysis drivers are most often scripts (Python recommended), but can also be built graphically in the Dakota GUI. The `analysis_driver_flow` figure goes here.
  2. **The three jobs of an analysis driver:** pre-process (put parameter values into your code's input, usually with a template and `dprepro`), execute, post-process (pull out QoIs and write the results file).
  3. **What Dakota writes and expects:** a short annotated parameters file and results file for black-box Rosenbrock (x1, x2 → f). Link to the full format spec further down the page or on its own page.
  4. **A minimal analysis driver** in synced `Python` / `Bash` / `Windows (PowerShell)` tabs (Decision 7), all three doing the same steps with the same comment headers:
     - *Python (`dakota.interfacing`):* `params, results = di.read_parameters_file()` → `di.dprepro(template="ros.template", parameters=params, output="ros.in")` → `subprocess.run([...,"rosenbrock_bb.py"])` → parse `Function value` from `ros.out` → `results["f"].function = value` → `results.write()`. About 20 lines, no ASV handling (the study uses `no_gradients` or `numerical_gradients`). Note that the existing `official/drivers/Python/di/driver.py` shows the advanced version with analytic gradients/Hessians.
     - *Bash:* adapted from `official/drivers/bash/simulator_script.sh` (`dprepro`, `grep`/`awk`, `echo`).
     - *Windows (PowerShell):* the same steps with `dprepro`, `Select-String` and `Set-Content`, following the cantilever `driver.ps1` pattern.
     - *GUI* (a note below the tabs, not a tab): a link to Next-Gen Workflow and the Markup File tool.
  5. **Telling Dakota about the analysis driver:** the interface block (in synced `JSON` / `Freeform` tabs), with the `analysis_drivers` value for each driver tab (`'python3 driver.py'`, `'driver.sh'`, `'powershell.exe -File driver.ps1'`), explaining `fork`, `parameters_file`/`results_file`, `work_directory` + `link_files`/`copy_files`, `file_save`/`directory_save`, and `asynchronous evaluation_concurrency`.
  6. **Test your analysis driver before running Dakota** (best practice): run it by hand against a sample `params.in` and check the results file. A troubleshooting checklist: executable bit, shebang, `python3` vs `python`, PATH/PYTHONPATH (for `import dakota.interfacing`), relative paths inside work directories, the results-file format, and where to look when an evaluation fails.
  7. **Other ways to connect** (short, after the reader has a working file-based driver): a small comparison table with columns *How data moves*, *When to use it*, *Example*:
     - **Analysis driver via `fork`/`system`** (files): any code in any language; the default and the focus of this page.
     - **Python direct interface** (`interface python`, `analysis_drivers = 'module.function'`; data passed in memory as lists or NumPy arrays): models already in Python, fast or very many evaluations where file and process overhead dominates, grey-box use such as returning analytic gradients. Show a 10-line Rosenbrock function, contrasted with the file-based driver from §4 (same math, no params/results files). Note that it requires a Dakota build with the Python direct interface enabled (it is enabled by default). Link to `advint:existingdirect:python` and `dakota-examples/official/drivers/Python/linked/`.
     - **Library API** (embedding Dakota in your application): link to `library-api`.
  8. **Next steps:** the existing deep-dive material (compiled Rosenbrock `rosen_syscall.in`, parameter-file format, SAND codes, GUI resources). Move it to `inputfile/interfaces/analysisdriverdeepdive.rst` (or merge it into `buildingblackboxinterface.rst`) and link to it. Also link the cantilever black-box example as "a more realistic engineering example".

#### Step 8. NEW "Choosing a Method" (`introduction/choosingamethod.rst`, label `choosingamethod-main`)
- **Content:** a goal-first table (*"I want to…"* → study type → representative Dakota methods → link). For example: "find which inputs matter" → sensitivity analysis → `sampling` with variance-based decomposition, `dace`, Morris (`psuade_moat`); "propagate input uncertainty" → UQ → `sampling`, `local_reliability`, `polynomial_chaos`; "find the best design" → optimization → gradient-based local, derivative-free local, global; "match experimental data" → calibration → `nl2sol`, Bayesian `bayes_calibration`; "explore a grid" → parameter studies.
- Short notes on cost (number of evaluations), on whether gradients are needed, and on the fact that **switching methods is just editing the method block** (point back to the tutorial's Part C, where the reader did exactly that with Rosenbrock).
- Link to `studytypes.rst` for depth. **Have a Dakota developer review the method recommendations.**

### Wave 3: Tutorial, download/install and splash (need Wave 2 labels)

#### Step 9. Tutorial: Your First Study (`introduction/helloworld.rst`)
- **Keep label** `helloworld-main`. Retitle to "Tutorial: Your First Dakota Study". Begin with "Prerequisite: Dakota installed and on your PATH (see Install Dakota)".
- **Every input and command in this tutorial appears in synced `JSON` / `Freeform` tabs, with JSON first.** The reader picks one format at the start and stays with it.
- **Part A, run a built-in example (5 min):** `rosen_multidim.json` / `rosen_multidim.in` (direct Rosenbrock), run with `dakota -json rosen_multidim.json -o rosen_multidim.out` or `dakota -i rosen_multidim.in -o rosen_multidim.out`, with the corrected command, what each output file is for, and the existing `rosen_2d_pts.png` grid plot.
- **Part B, the same problem as a black box (15 min):** run Rosenbrock through an analysis driver. Copy the example folder; look at `ros.template` and the fake engineering input `ros.in`; run `rosenbrock_bb.py` by hand; run the analysis driver by hand on a sample `params.in` (in synced `Python` / `Bash` / `Windows (PowerShell)` tabs, including the platform-specific commands); then run Dakota with an input file that is identical to Part A **except for the interface block** (`fork` + `analysis_drivers`). The tabular results match Part A, which proves the driver works.
- **Part C, one interface, many methods (5 min):** change *only* the method block (for example to `sampling` with `uniform_uncertain` variables, or a local optimizer with `numerical_gradients`) and rerun. Point to "Choosing a Method".
- **Next step:** link to the cantilever black-box example for a realistic multi-response engineering problem.
- **Prerequisite (engineering work, do first):** create a beginner-friendly black-box Rosenbrock example: three equivalent analysis drivers (`driver.py` using `dakota.interfacing` with no ASV handling, `driver.sh`, `driver.ps1`), plus `ros.template` and `rosenbrock_bb.py` copied from `official/drivers/bash/templatedir/`. Register the Python and Bash variants as tests; test the PowerShell variant on Windows CI if available, otherwise add it to the manual pre-release checklist. Put it where it **ships in the install and is regression-tested**: either under `dakota-examples/official/drivers/Python/` (as a `di_simple/` sibling with a `.base` file for the examples test harness) or in `test/examples-users/`, so `docs/CMakeLists.txt` generates it into `samples/` for `literalinclude`. **Each tutorial input ships in both formats** (`rosen_multidim.json` alongside `rosen_multidim.in`, and likewise for the Part B and C inputs). Create the JSON with `dakota_parser --json` and register both versions as tests, following the existing `*_json.json` tests in `test/`. Extend the docs `samples/` generation, which currently picks up only the freeform users examples, to copy the JSON twins too. Make sure the tutorial's paths match where the files land in an installed `share/dakota/examples/`.
- **Acceptance:** A new user with only Dakota installed (plus Python 3) can follow it on Linux/macOS **and** Windows, copy-pasting every command. Each input passes `dakota -check`, and the tests pass.

#### Step 10. Download & Install section (split from Getting Started)
- **New** `download.rst`; the label `setupdakota-download` moves here. It holds the release tables (CLI / CLI+GUI / GUI). Recommendation: generate the tables from a small YAML/CSV data file with a tiny directive or pre-build script, so a release adds one row instead of five hand-edited URL lines. It's optional, but prevents the 6.24/6.23 drift.
- `setupdakota.rst` becomes **"Install Dakota"** (keep labels `setupdakota-main`, `setupdakota-installation`, `setupdakota-environment`): choose a package (CLI vs CLI+GUI vs GUI) → extract → set PATH/PYTHONPATH (in `.. tab::` blocks per OS/shell) → verify with `dakota -v` **and** `python3 -c "import dakota.interfacing"` (because the tutorial needs it) → "Next: What is Dakota?" / "Next: Tutorial".
- Move the source-build `make install-python` note to `building/`.
- Group both pages under a "Download & Install" toctree caption, separate from Getting Started.

#### Step 11. Splash page (`index.rst`)
- **Content:**
  - H1 "Dakota".
  - A one-sentence value proposition, e.g. *"Dakota runs ensembles of your simulation to answer design and uncertainty questions — connect your code once through an analysis driver, then apply optimization, uncertainty quantification, sensitivity analysis, and calibration methods."*
  - The `ensemble_loop` or `one_interface_many_methods` figure.
  - **Three "Start here" paths** (plain RST list-table or simple cards; `sphinx-design` grid cards are an option but would add a dependency):
    - *New to Dakota?* → What is Dakota? → Anatomy → Analysis Driver → Tutorial
    - *Download & Install* → Download / Install
    - *Already using Dakota?* → Keyword Reference, Study Types, Examples, Release Notes
  - "Latest release: X.Y (date)" line using `|release|`, plus the release's headline features (currently JSON input) linked to their pages.
  - No input-file syntax on the splash page; if a snippet is wanted there, use synced JSON / Freeform tabs.
  - Short "Get help" (mailing list / GitHub discussions / issues) and "Cite Dakota" sections.
- **Toctree:** captioned toctrees in this order: `Download & Install`, `Getting Started with Dakota` (pages 1–5 from §2), then the existing `Using Dakota`, GUI, etc. Remove the moved pages from `usingdakota/usingdakota.rst`'s toctree to avoid duplicate nav entries.

### Wave 4: Integration and review

#### Step 12. Navigation, terminology and cross-link pass
- Update `usingdakota/usingdakota.rst`, `classictoc.rst`, and every `:ref:` to moved labels. Leave stub pages with redirects where URLs change (use `sphinx-reredirects`, or a meta-refresh stub if adding an extension isn't wanted) so external links into the old pages still work.
- Add "Next:" / "Previous:" links across the Getting Started sequence.
- **Terminology sweep** across `docs/user/` (outside the generated reference): replace "driver script", "simulator script" and "interface script" with "analysis driver" where they mean the same thing; leave code/file names alone.
- Replace references to `InputBlocks.png` and `CouplingSimulations1.png` with the TikZ figures (keep the old PNGs until nothing references them, then delete them).
- **Light rewrite of `inputfile/json.rst`** (Decision 6): present JSON as a fully supported format (no "experimental"), open with what the JSON format is and how to run it (`dakota -json`), keep the annotated example, and move "differences from freeform" plus the `dakota_parser` conversion recipe into a later "Converting freeform input files" section.
- Make sure `examples.rst`, `studytypes.rst` and `inputfile.rst` each open with one line pointing beginners back to the matching Getting Started page.

#### Step 13. Verification (separate agent, fresh context)
- Clean build: no new warnings against the Step 1 baseline. `linkcheck` clean for in-scope pages. `render_figures.sh` reproduces the committed SVG/PDFs.
- `dakota -check` on every full input file in Getting Started, in both formats. Check that every input snippet has both a `JSON` and a `Freeform` tab with matching content (a short script can parse the `.rst` files and diff the parsed JSON against `dakota_parser --json` output for the freeform tab). Run the tutorial (Parts A–C) end to end in **both** formats in a clean container on Linux, using the Python and Bash drivers, and on Windows using the Python and PowerShell drivers. Check that every driver snippet has all three tabs with matching labels.
- **Fresh-eyes review:** give an agent that hasn't seen the work only the built HTML, with the prompt *"You are an engineer who has never heard of Dakota and has a Python simulation. Using only these pages, explain what Dakota does, write an analysis driver for your code, and set up a sampling study. List every point where you got confused or had to guess."* Fix what it finds.
- **Human review:** one Dakota developer for technical accuracy (method recommendations, keyword usage), plus ideally one actual newcomer (intern or new hire) doing the tutorial while someone watches.

---

## 6. Dependency summary

| Step | Depends on | Parallel with |
|---|---|---|
| 1 Build baseline | — | 2, 3, 4 |
| 2 Quick fixes | — | 1, 3, 4 |
| 3 Glossary | — | 1, 2, 4 |
| 4 TikZ figures | — | 1, 2, 3 |
| 5 What is Dakota | 3, 4 | 6, 7, 8 |
| 6 Study anatomy | 3, 4 | 5, 7, 8 |
| 7 Analysis driver | 3, 4 | 5, 6, 8 |
| 8 Choosing a method | 3 | 5, 6, 7 |
| 9 Tutorial (+ new black-box Rosenbrock example) | 6, 7 | 10, 11 |
| 10 Download & Install | — (can start in Wave 1) | 9, 11 |
| 11 Splash | 5–8, 10 labels | 9 |
| 12 Nav/terminology pass | 5–11 | — |
| 13 Verification | 12 | — |

Step 9's example-creation work (the simplified Python driver and its test registration) can start in Wave 1, so the tutorial isn't blocked on it.

---

## 7. Other "good documentation" recommendations

- **Concepts before syntax.** Newcomers should understand the loop (Dakota ⇄ analysis driver ⇄ code) and the six blocks before they ever see `dakota.input.summary`. The current order (tutorial → examples → coupling → input file) reverses this.
- **One running example.** Using Rosenbrock everywhere lets each page add one new idea instead of a new problem. The direct-vs-black-box comparison in the tutorial gives a built-in correctness check.
- **Simple first, but never wrong.** Lead with the black-box story, word it so it stays true (no "only through files"), and put each important alternative, such as the in-memory Python interface, in one clearly signposted place, instead of hedging every sentence.
- **Separate concepts from syntax.** Teaching the study structure in format-neutral terms means the intro won't need rewriting if the syntax changes again (or when Python bindings arrive). Only the snippets change.
- **Progressive disclosure.** Getting Started pages show the 80% case and link out for options (ASV/analytic gradients, batch evaluation, work-directory details). Long keyword lists belong in the reference.
- **Treat docs examples as tests.** Every complete input and analysis driver in the docs should be `literalinclude`d from a file that ctest or the examples-repo harness runs. Consider a CI job that runs `dakota -check` on all `.. code-block:: dakota` and `.. code-block:: json` input snippets too.
- **Diagrams as code.** TikZ sources in the repo mean diagrams get reviewed in PRs like text, share one style, and can be reused in the slide decks (Beamer, or exported PDF/SVG), which keeps talks and docs visually consistent.
- **No drift-prone facts in prose.** No version numbers, ages ("29 years") or release lists typed by hand. Use `|release|`, and generate download tables from data.
- **Analysis-driver troubleshooting deserves its own page.** Most support questions are about drivers failing (paths, permissions, Python environment, results-file format, work directories). A "My evaluations fail" FAQ would pay off quickly.
- **A parallel GUI track.** Since "analysis driver" now covers GUI-built drivers, keep GUI users on the same conceptual pages and branch only at "how do I create the analysis driver" (Next-Gen Workflow, Markup tool), so there aren't two separate introductions.
- **Accessibility and scanning.** Alt text on every figure, descriptive link text instead of "click here" or "Read More...", and headings that work as a table of contents on their own.
- **Close the loop with talks.** Link the public YouTube training videos from the matching Getting Started page, and once the docs are refactored, point slide decks at those URLs so the talk and the docs reinforce each other.
