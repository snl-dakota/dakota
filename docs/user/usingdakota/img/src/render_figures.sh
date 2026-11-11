#!/usr/bin/env bash
#
# Render the TikZ figure sources in this folder to SVG (for HTML builds) and
# PDF (for LaTeX/PDF builds), and copy both into ../intro/.
#
#   PDF: pdflatex
#   SVG: latex (DVI output, with TikZ's dvisvgm driver), then dvisvgm.
#        Text is converted to outlines, so the SVG needs no fonts to display.
#
# Usage (from anywhere):
#   docs/user/usingdakota/img/src/render_figures.sh            # render every figure
#   docs/user/usingdakota/img/src/render_figures.sh ensemble_loop study_blocks
#                                                  # render only these figures
#
# Requires only a TeX distribution (TeX Live or MacTeX), which provides
# pdflatex, latex and dvisvgm, with the standalone, pgf/tikz and psnfss
# packages. On Linux distributions that split TeX Live into packages,
# dvisvgm may be separate (for example texlive-dvisvgm on Fedora/RHEL).
#
# Commit both the .svg and the .pdf outputs, so ordinary docs builds don't
# need TeX. Pages reference figures as:
#   .. figure:: /usingdakota/img/intro/<name>.*
# and Sphinx chooses the SVG for HTML and the PDF for LaTeX.
#
# The rendered files are reproducible (fixed dates, no random PDF IDs), so a
# CI job can re-run this script and fail on `git diff --exit-code` if the
# committed figures are out of date with their sources.

set -euo pipefail

src_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
out_dir="$(cd "$src_dir/.." && pwd)/intro"
style="dakotastyle.tex"

for tool in pdflatex latex dvisvgm; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "error: '$tool' not found on PATH" >&2
    exit 1
  fi
done

# Pin timestamps embedded in the PDF so re-rendering gives identical files.
export SOURCE_DATE_EPOCH=0
export FORCE_SOURCE_DATE=1

# Choose figures: the names given on the command line, or every .tex file
# except the shared style file.
figures=()
if [ "$#" -gt 0 ]; then
  for name in "$@"; do
    name="${name%.tex}"
    if [ ! -f "$src_dir/$name.tex" ]; then
      echo "error: no source file $src_dir/$name.tex" >&2
      exit 1
    fi
    figures+=("$name")
  done
else
  for tex in "$src_dir"/*.tex; do
    name="$(basename "$tex" .tex)"
    [ "$name.tex" = "$style" ] && continue
    figures+=("$name")
  done
fi

if [ "${#figures[@]}" -eq 0 ]; then
  echo "error: no figure sources found in $src_dir" >&2
  exit 1
fi

mkdir -p "$out_dir"
work_dir="$(mktemp -d)"
trap 'rm -rf "$work_dir"' EXIT

failed=()
for name in "${figures[@]}"; do
  echo "Rendering $name"
  # Run in the source folder so \input{dakotastyle} is found; keep the
  # auxiliary files out of the repository.
  if ! (cd "$src_dir" && pdflatex -interaction=nonstopmode -halt-on-error \
          -output-directory="$work_dir" "$name.tex" > "$work_dir/$name.console" 2>&1); then
    echo "  FAILED: pdflatex error. Last lines of the log:" >&2
    tail -n 20 "$work_dir/$name.log" >&2 || true
    failed+=("$name")
    continue
  fi
  # SVG: compile to DVI with TikZ's dvisvgm driver, then convert. The
  # standalone class records the page size (including its border) in the DVI,
  # so --bbox=papersize gives the SVG the same size as the PDF.
  if ! (cd "$src_dir" && latex -interaction=nonstopmode -halt-on-error \
          -output-directory="$work_dir" -jobname="$name" \
          "\\def\\pgfsysdriver{pgfsys-dvisvgm.def}\\input{$name}" \
          > "$work_dir/$name.dvi.console" 2>&1); then
    echo "  FAILED: latex (DVI) error. Last lines of the log:" >&2
    tail -n 20 "$work_dir/$name.log" >&2 || true
    failed+=("$name")
    continue
  fi
  if ! dvisvgm --bbox=papersize --no-fonts --verbosity=1 \
          --output="$work_dir/$name.svg" "$work_dir/$name.dvi"; then
    echo "  FAILED: dvisvgm error" >&2
    failed+=("$name")
    continue
  fi
  cp "$work_dir/$name.pdf" "$work_dir/$name.svg" "$out_dir/"
  echo "  wrote intro/$name.svg and intro/$name.pdf"
done

if [ "${#failed[@]}" -gt 0 ]; then
  echo "error: failed to render: ${failed[*]}" >&2
  exit 1
fi
echo "Done: ${#figures[@]} figure(s) in $out_dir"
