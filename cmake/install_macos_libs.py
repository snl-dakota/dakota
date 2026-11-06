#!/usr/bin/env python3
import logging
import os
import pathlib
import stat
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from typing import List, Callable


# Directories whose Mach-O files are scanned to copy missing external TPL
# dylibs into bin/.  Python extensions are excluded: their deps live in lib/
# (already handled) so they add no new files to copy, but recursing through
# libdakota_src.dylib's now-large DT_NEEDED list for each .so is very slow.
copy_scan_dirs = [
    "bin",
    "lib",
    "share/dakota/test",
    "share/dakota/examples/hopspack/1-var-bnds-only",
    "share/dakota/examples/hopspack/2-linear-constraints",
    "share/dakota/examples/hopspack/3-degen-linear-constraints",
    "share/dakota/examples/hopspack/4-nonlinear-constraints",
    "share/dakota/examples/hopspack/5-multi-start",
]

# All Mach-O directories: receive RPATH cleanup and dep-name rewriting.
# Includes Python extensions so their absolute build RPATHs are removed and
# dependency references are rewritten to @rpath/name.
executable_dirs = copy_scan_dirs + [
    "share/dakota/Python",
]


def strip_path_line(line: str) -> str:
    """Strip "path" and (offset) information from otool -L output"""
    no_path = line.strip()[5:]
    offset_index = no_path.find("(offset")
    return no_path[:offset_index-1]


def extract_rpaths_from_lines(lines: List[str]) -> List[str]:
    """Get RPATH paths from otool -l output"""
    rpaths = []
    line_iter = iter(lines)
    line = next(line_iter)
    try:
        while True:
            if "LC_RPATH" in line:
                next(line_iter)
                path_line = next(line_iter)
                rpaths.append(strip_path_line(path_line))
            line = next(line_iter) 
    except StopIteration:
        return rpaths

 
def get_rpaths(filename: str) -> List[str]:
    """Return RPATHs for a file"""
    p = subprocess.run(["otool", "-l", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode != 0:
        logging.debug(f"Running otool -l on {filename} failed. stderr: {p.stderr}")
        return []
    rpaths = extract_rpaths_from_lines(p.stdout.split("\n"))
    return rpaths


def get_dependencies(filename: str) -> List[str]:
    """Return all dynamic lib dependencies of filename"""
    p = subprocess.run(["otool", "-L", filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode != 0:
        logging.debug(f"Running otool -L on {filename} failed. stderr: {p.stderr}")
        return []
    lines = p.stdout.split("\n")
    # First line is the name of the file. Remove it.
    del lines[0]
    deps = []
    for line in lines[:-1]:  # final line is blank
        deps.append(line.split()[0].strip())
    return deps


def strip_ats(lines: List[str]) -> List[str]:
    """Return the lines that don't begin with @"""
    return [line for line in lines if line[0] != '@']


def strip_system_locations(lines: List[str]) -> List[str]:
    """Remove lines that don't look like system paths"""
    system_locations = ["/System", "/usr/lib", "/usr/X11"]
    stripped = []
    for line in lines:
        if not any(line.startswith(loc) for loc in system_locations):
            stripped.append(line)
    return stripped


def resolve_rpaths(rpaths: List[str], requests: List[str], base_dir: str) -> List[str]:
    """Resolve paths in requests with rpaths if possible

    Resolve means, try to figure out from the RPATH and
    the library id (request) where the library is located
    on the file system.

    Substitute rpath entries for @rpath in 
    requests. If that file exists, it's been resolved. Any
    instances of @loader_path in rpaths are substituted
    with base_dir before the attempts are made.
    
    If a requested library can't be resolved, it's returned as-is.
    """
    resolved = []
    rpaths = [r.replace("@loader_path", base_dir) for r in rpaths]
    rpaths_paths = [pathlib.Path(r) for r in rpaths]
    for req in requests:
        if not req.startswith("@rpath"):
            resolved.append(req)
        else:
            clipped = req[7:]  # remove @rpath
            for rpath in rpaths_paths:
                abs_path = rpath / clipped
                try:
                    abs_path_exists = abs_path.exists()
                except PermissionError:
                    logging.warning(f"Existence of {abs_path} could not be determined due to permission problem.")
                    continue
                if abs_path_exists:
                    resolved.append(str(abs_path))
                    break
            else:  # couldn't be resolved
                resolved.append(req)
    return resolved


def strip_at_executable(lines: List[str]) -> List[str]:
    """Return the lines that don't begin with @"""
    return [line for line in lines if not line.startswith("@executable")]


def copy_writable(src: str, dst: str) -> None:
    """Copy src to dst and ensure dst remains user-writable.

    shutil.copy() preserves permission bits from the source file. When a
    dependency is read-only, the copied file becomes read-only too, which can
    break subsequent overwrites or later install_name_tool/codesign updates.
    """
    if os.path.exists(dst):
        current_mode = os.stat(dst).st_mode
        os.chmod(dst, current_mode | stat.S_IWUSR)

    shutil.copy(src, dst, follow_symlinks=True)

    current_mode = os.stat(dst).st_mode
    os.chmod(dst, current_mode | stat.S_IWUSR)


def copy_external_deps(filename: str, copied: List[str], dest: str, exclude: List[str]) -> List[str]:
    """Copy external dependencies of filename into the dest folder

   This function is recursively called on dependencies that are discovered.
   
   if the dependency's path begins with any of the strings in exclude,
   don't copy it (but do still recurse).

   Dependencies in system locations are skipped entirely.

   Return a list of copied files so that the copies aren't repeatedly
   done.

   """
    base_dir = str(pathlib.Path(filename).resolve().parent)
    logging.debug(f"Copy: base_dir of {filename} resolved to {base_dir}")
    all_rpaths = get_rpaths(filename)
    rpaths = strip_at_executable(all_rpaths)
    logging.debug(f"Copy: RPATHs of {filename}: {', '.join(rpaths)}")

    all_deps = get_dependencies(filename)
    logging.debug(f"Copy: All deps of {filename}: {', '.join(all_deps)}")
    no_sys = strip_system_locations(all_deps)
    resolved = resolve_rpaths(rpaths, no_sys, base_dir)
    deps_to_use = strip_ats(resolved)
    logging.debug(f"Copy: Deps of {filename} to process: {', '.join(deps_to_use)}")

    newly_copied = []
    for dep in deps_to_use:
        if dep in copied or dep == filename:
            logging.debug(f"Copy: dependency {dep} skipped")
            continue
        else:
            p = pathlib.Path(dep)
            dest_filename = os.path.join(dest, p.name)
            if not any(dep.startswith(e) for e in exclude):
                copy_writable(dep, dest_filename)
                newly_copied.append(dep)
                logging.debug(f"Copy: {dep} copied")
                newly_copied += copy_external_deps(dep, copied + newly_copied, dest, exclude)
    return newly_copied

def change_external_deps(filename, sig_warnings):
    """Change external dependencies to @rpath (single batched install_name_tool call)"""
    all_deps = get_dependencies(filename)
    no_sys = strip_system_locations(all_deps)
    deps_to_use = strip_ats(no_sys)
    if not deps_to_use:
        return []
    cmd = ["install_name_tool"]
    for dep in deps_to_use:
        cmd += ["-change", dep, f"@rpath/{pathlib.Path(dep).name}"]
    cmd.append(filename)
    p = subprocess.run(cmd, text=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    if p.returncode != 0:
        logging.error(f"Changing deps in {filename} failed: {p.stderr}")
    if "signature" in p.stderr and filename not in sig_warnings:
        logging.debug(f"Change External Deps: Code signature changed for {filename}")
        return [filename]
    return []


def remove_rpaths(file: str, rpaths: List[str], sig_warnings: List[str]) -> List[str]:
    """Remove rpaths from file (single batched install_name_tool call)"""
    if not rpaths:
        return []
    cmd = ["install_name_tool"]
    for rpath in rpaths:
        cmd += ["-delete_rpath", rpath]
    cmd.append(file)
    p = subprocess.run(cmd, text=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
    if p.returncode != 0:
        logging.error(f"Remove RPATH: Deleting RPATHs from {file} failed: {p.stderr}")
    if "signature" in p.stderr and file not in sig_warnings:
        logging.debug(f"Remove RPATH: Code signature changed for {file}")
        return [file]
    return []


def remove_external_rpaths(filename: str, sig_warnings: List[str]) -> None:
    """Remove RPATHs that don't begin with @"""
    all_rpaths = get_rpaths(filename)
    rpaths_to_remove = strip_ats(all_rpaths)
    return remove_rpaths(filename, rpaths_to_remove, sig_warnings)


_MACHO_MAGIC = frozenset([
    b'\xfe\xed\xfa\xce',  # 32-bit big-endian
    b'\xce\xfa\xed\xfe',  # 32-bit little-endian
    b'\xfe\xed\xfa\xcf',  # 64-bit big-endian
    b'\xcf\xfa\xed\xfe',  # 64-bit little-endian
    b'\xca\xfe\xba\xbe',  # fat binary (universal)
    b'\xbe\xba\xfe\xca',  # fat binary big-endian
])


def _is_mach_o(filepath: str) -> bool:
    try:
        with open(filepath, 'rb') as f:
            return f.read(4) in _MACHO_MAGIC
    except (IOError, PermissionError, IsADirectoryError):
        return False


def mach_o_files_in_dir(dir: str) -> List[str]:
    """Return a list of Mach-O files in dir (no subprocess)"""
    try:
        entries = os.listdir(dir)
    except (PermissionError, NotADirectoryError):
        return []
    result = []
    for name in entries:
        path = os.path.join(dir, name)
        if os.path.isfile(path) and _is_mach_o(path):
            logging.debug(f"Mach-O: In {dir}, {name} is a Mach-O binary")
            result.append(name)
    return result
    

def apply_to_dirs(dir_list: List[str], state: List[str], f: Callable,  *args) -> List[str]:
    """Recursively call f on all files in the list of dirs

    For each file, f is called like f(filename, state, *args)

    f is expected to return state, which is accumulated. The accumulated
    state + the original state is passed to subsequent calls to f.

    Returns the accumulated state (not including the original state)
    """
    updated_state = []
    for dir in dir_list:
        for (dirpath, _dirnames, _filenames) in os.walk(dir):
            mach_o_filenames = mach_o_files_in_dir(dirpath)
            for filename in mach_o_filenames:
                filepath = os.path.join(dirpath, filename)
                updated_state += f(filepath, state + updated_state, *args)
    return updated_state


def change_ids(filenames: List[str], dest_dir: str, sig_warnings: List[str]) -> List[str]:
    """Update library ids to @rpath/name"""
    new_sig_warnings = []
    for file in filenames:
        file_p = pathlib.Path(file)
        dest_dir_p = pathlib.Path(dest_dir)
        filepath = str(dest_dir_p/file_p.name)
        p = subprocess.run(["install_name_tool", "-id", f"@rpath/{file_p.name}", filepath], check=True, text=True, stderr=subprocess.PIPE)
        if "signature" in p.stderr and filepath not in sig_warnings + new_sig_warnings:
            new_sig_warnings.append(filepath)
    return new_sig_warnings


def _sign_one(file: str) -> None:
    p = subprocess.run(
        ["codesign", "--sign", "-", "--force",
         "--preserve-metadata=entitlements,requirements,flags,runtime", file],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if p.returncode != 0:
        logging.error(f"Couldn't re-sign {file}: {p.stderr}")
    else:
        logging.debug(f"Re-signed {file}")


def fix_code_signatures(files: List[str]) -> None:
    """Re-sign files in parallel (codesign is the main packaging bottleneck)"""
    if not files:
        return
    workers = min(len(files), os.cpu_count() or 4)
    with ThreadPoolExecutor(max_workers=workers) as executor:
        list(executor.map(_sign_one, files))


if __name__ == '__main__':
    logging.basicConfig(filename='process_external_tpls.log', level=logging.DEBUG)

    if len(sys.argv) == 2:
        os.chdir(sys.argv[1])

    logging.info(f"PWD is {os.getcwd()}")
    logging.info(f"Will scan dependencies of executables and libraries in {', '.join(executable_dirs)}")

    copy_black_list = [str(pathlib.Path("lib").resolve()),
                       str(pathlib.Path("bin").resolve())]

    copied = apply_to_dirs(copy_scan_dirs, [], copy_external_deps, "bin", copy_black_list)
    logging.info("Dependency copying completed")

    sig_warnings = change_ids(copied, "bin", [])
    logging.info("Library IDs of copied dependencies updated")

    sig_warnings += apply_to_dirs(executable_dirs, sig_warnings, remove_external_rpaths)
    logging.info("External RPATHs removed from executables and libraries")

    sig_warnings += apply_to_dirs(executable_dirs, sig_warnings, change_external_deps)
    logging.info("Paths of dependencies updated to @rpath/name")

    fix_code_signatures(sig_warnings)
    logging.info("Code re-signed")
    
