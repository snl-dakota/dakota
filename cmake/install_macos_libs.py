#!/usr/bin/env python3
import logging
import os
import pathlib
import shutil
import subprocess
import sys
from typing import List, Callable


# Scan for and, as needed, update dependencies of executables in these subdirectories
executable_dirs = [
    "bin",
    "lib",
    "share/dakota/test",
    "share/dakota/examples/hopspack/1-var-bnds-only",
    "share/dakota/examples/hopspack/2-linear-constraints",
    "share/dakota/examples/hopspack/3-degen-linear-constraints",
    "share/dakota/examples/hopspack/4-nonlinear-constraints",
    "share/dakota/examples/hopspack/5-multi-start",
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
                if abs_path.exists():
                    resolved.append(str(abs_path))
                    break
            else:  # couldn't be resolved
                resolved.append(req)
    return resolved


def strip_at_executable(lines: List[str]) -> List[str]:
    """Return the lines that don't begin with @"""
    return [line for line in lines if not line.startswith("@executable")]


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
                shutil.copy(dep, dest_filename, follow_symlinks=True)
                newly_copied.append(dep)
                logging.debug(f"Copy: {dep} copied")
            newly_copied += copy_external_deps(dep, copied + newly_copied, dest, exclude)
    return newly_copied

def change_external_deps(filename, sig_warnings):
    """Change external dependencies to @rpath"""
    all_deps = get_dependencies(filename)
    no_sys = strip_system_locations(all_deps)
    deps_to_use = strip_ats(no_sys)
    new_sig_warnings = []
    for dep in deps_to_use:
        dep_name = pathlib.Path(dep).name
        p = subprocess.run(["install_name_tool", "-change", dep, f"@rpath/{dep_name}", filename],
                           text=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        if p.returncode != 0:
            logging.error(f"Changing the path of library {dep_name} in {filename} failed: {p.stderr}")
        if "signature" in p.stderr:
            logging.debug(f"Change External Deps: Code signature changed for {filename}")
            if filename not in sig_warnings + new_sig_warnings:
                new_sig_warnings.append(filename)
    return new_sig_warnings


def remove_rpaths(file: str, rpaths: List[str], sig_warnings: List[str]) -> List[str]:
    """Remove rpaths from file"""
    new_sig_warnings = []
    for rpath in rpaths:
        p = subprocess.run(["install_name_tool", "-delete_rpath", rpath, file],
                           text=True, stderr=subprocess.PIPE, stdout=subprocess.PIPE)
        if p.returncode != 0:
            logging.error(f"Remove RPATH: Deleting {rpath} from {file} failed: {p.stderr}")
        if "signature" in p.stderr:
            logging.debug(f"Remove RPATH: Code signature changed for {file}")
            if file not in sig_warnings + new_sig_warnings:
                new_sig_warnings.append(file)
    return new_sig_warnings


def remove_external_rpaths(filename: str, sig_warnings: List[str]) -> None:
    """Remove RPATHs that don't begin with @"""
    all_rpaths = get_rpaths(filename)
    rpaths_to_remove = strip_ats(all_rpaths)
    return remove_rpaths(filename, rpaths_to_remove, sig_warnings)


def mach_o_files_in_dir(dir: str) -> List[str]:
    """Return a list of Mach-O files in dir"""
    p = subprocess.run("file --exclude ascii *", text=True, cwd=dir, stderr=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
    if p.returncode != 0:
        logging.error(f"Mach-O Scan: Error in {dir}: {p.stderr}")

    mach_o_files = []
    for line in p.stdout.split("\n"):
        if not line:
            continue
        filename_and_note, description = line.split(":", 1) # sometimes description contains :
        filename = filename_and_note.split()[0]
        if "Mach-O" in description:
            logging.debug(f"Macho-O: In {dir}, {filename} is an executable")
            mach_o_files.append(filename)
    return mach_o_files  
    

def apply_to_dirs(dir_list: List[str], state: List[str], f: Callable,  *args) -> List[str]:
    """Recursively call f on all files in the list of dirs

    For each file, f is called like f(filename, state, *args)

    f is expected to return state, which is accumulated. The accumulated
    state + the original state is passed to subsequent calls to f.

    Returns the accumulated state (not including the original state)
    """
    updated_state = []
    for dir in dir_list:
        for (dirpath, dirnames, filenames) in os.walk(dir):
            mach_o_filenames = mach_o_files_in_dir(dirpath)
            use_filenames = mach_o_filenames if mach_o_filenames else filenames
            for filename in use_filenames:
                filepath = os.path.join(dirpath, filename)
                updated_state += f(filepath, state + updated_state, *args)
            updated_state += apply_to_dirs(dirnames, state + updated_state, f, *args)
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


def fix_code_signatures(files: List[str]) -> None:
    """Re-sign files

    source: https://stackoverflow.com/questions/51833310/what-is-killed9-and-how-to-fix-in-macos-terminal 
    """
    for file in files:
        p = subprocess.run(["codesign", "--sign", "-", "--force", "--preserve-metadata=entitlements,requirements,flags,runtime", file],
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        if p.returncode != 0:
            logging.error(f"Couldn't re-sign {file}: {p.stderr}")
        logging.debug(f"Re-signed {file}")
        


if __name__ == '__main__':
    logging.basicConfig(filename='process_external_tpls.log', level=logging.DEBUG)

    if len(sys.argv) == 2:
        os.chdir(sys.argv[1])

    logging.info(f"PWD is {os.getcwd()}")
    logging.info(f"Will scan dependencies of executables and libraries in {', '.join(executable_dirs)}")


    # don't copy files from lib/, which are targets built by cmake, to bin/
    copy_black_list = [str(pathlib.Path("lib").resolve())]

    # Recursively copy the external dependencies of targets to the bin folder
    copied = apply_to_dirs(executable_dirs, [], copy_external_deps, "bin", copy_black_list)
    logging.info("Dependency copying completed")
 
    # Update the library ids of copied files
    sig_warnings = change_ids(copied, "bin", [])
    logging.info("Library IDs of copied dependencies updated")
     
    # Remove absolute RPATHs from all executables and libs
    sig_warnings += apply_to_dirs(executable_dirs, sig_warnings, remove_external_rpaths)
    logging.info("External RPATHs removed from executables and libraries")

    # Convert library dependencies of all executables and libraries to @rpath/foo.dylib
    sig_warnings += apply_to_dirs(executable_dirs, sig_warnings, change_external_deps)
    logging.info("Paths of dependencies updated to @rpath/name")

    # Updating executables and libs may have invalidated some of their signatures. Fix them.
    fix_code_signatures(sig_warnings)
    logging.info("Code re-signed")
    
