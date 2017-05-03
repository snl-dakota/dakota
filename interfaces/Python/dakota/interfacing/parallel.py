"""Manage evaluation tiling on SLURM+OpenMPI HPCs that lack srun.

This module provides support for tiling multiprocessor simulation runs in a
job allocation. As much information as possible is collected from SLURM
environment variables, including the number of nodes and processes per node.
(The terms processses and tasks are used interchangably.)

Static and dynamic scheduling are supported. Under a static scheduling model,
the launching function tiling_run_static should be used, and for dynamic, 
tiling_run_dynamic. In static scheduling, tile availability is determined using
the Dakota evaluation number, which must be provided by the user. Dynamic
scheduling makes use of named pipes (FIFOs), which can be created atomically.
The pipes are used only as tokens to represent in-use tiles. Nothing is written 
or read from them. This approach eliminates the need for pre-generating a set of 
machine files. By default, named pipes are created in $HOME/.DakotaEvalTiling,
but the location (and pipe names) is customizable.

Other module features:
  * Dedicated Dakota node. When enabled, the first node is reserved for running
    Dakota and the user's analysis drivers, which may be desireable when the 
    user's simulations are memory intensive.
  * MIMD model. The launcher functions expect a list of commands along with the
    number of processes for each.

Example Usage::

    from dakota.interfacing import parallel, read_parameters_file

    # Dynamic scheduling. On 4 processs, launch "my_sim" with the --verbose
    # option. Pass the "--bind-to none" option ot mpirun by including it
    # before the user's executable and its options.

    my_command = (4, ["--bind-to", "none", "my_sim", "--verbose"])
    parallel.tile_run_dynamic(commands=[my_command])

    # Static scheduling, same command. However, the eval number is needed.
    # Assumption: This script was forked by Dakota as analysis_driver,
    # and therefore read_parameters_file can acquire the names of the params
    # and results files from the command line arguments.

    params, results = read_parameters_file()
    parallel.tile_run_static(commands=[my_command], eval_num=params.eval_num)

    # MIMD model: mpirun -np 16 my_sim1 : -np 16 my_sim2

    my_commands = [ (16, ["my_sim1"]),
                 (16, ["my_sim2"])]
    parallel.tile_run_dynamic(commands=my_commands)

An operational example is located in
``examples/parallelism/Case3-EvaluationTiling/OpenMPI``.
"""
from __future__ import print_function, division
import os
import re
import sys
import subprocess
import time
from . import slurm
from .interfacing import read_parameters_file, UNNAMED

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014 Sandia Corporation'
__license__ = 'GNU Lesser General Public License'

# For now, only SLURM is supported, but next we can add PBS and a plugin 
# capability so that users can define their own. A plugin will need to define:
# - detection (use me if these criteria are met)
# - extra required arguments to node_placement
# - a method/function that returns a nodelist
#
# Or, not. After all, how many resource managers are in common use?

## Constants
NODE = 0
TILE = 1


# TODO: Insert code to add plugin modules to this list
_user_mangers = []

# Built-in managers
_managers = [slurm]


class MgrEnvError(Exception):
    pass

class ResourceError(Exception):
    pass

def _get_job_info():
    """Determine the resource manager type and query the environment to
    return a tuple containing the number of nodes, MPI tasks per node, and 
    job id.
    
    MgrEnvError is raised if the manager cannot be identified"""

    for plugin in _user_mangers + _managers:
        if plugin.is_me():
            return plugin.get_job_info()
    raise MgrEnvError("Unrecognized resource manager environment.")

def _get_node_list(tile=None, applic_tasks=None, 
        tasks_per_node=None, dedicated_master=None):
    """Determine the relative node list

    Keyword Args:
        tile: Tile number, 0-based.
        applic_tasks: Number of MPI processes (tasks) per tile.
        tasks_per_node: Number of MPI tasks per node.
        dedicated_master: Reserve the first NODE or TILE for Dakota (default: 
            no reserved node)

    Returns:
        A string containing the relative node list.
            
    """
    if dedicated_master == NODE:
        start_node = tile * applic_tasks//tasks_per_node + 1
    elif dedicated_master == TILE:
        start_node = (tile + 1) * applic_tasks//tasks_per_node
    else:
        start_node = tile * applic_tasks//tasks_per_node

    end_node = start_node + (applic_tasks-1)//tasks_per_node
    node_string = "+n" + ",+n".join([str(i) for i in range(start_node, end_node+1)])
    return node_string

def _calc_static_tile(eval_num=None, num_tiles=None):
    """For static scheduling, compute the static tile

    Keyword args:
        eval_num: Dakota evaluation number
        num_tiles: Number of tiles in the job
    """

    return (eval_num - 1) % num_tiles

class _TileLock(object):
    """Context manager to acquire and release tiles for dynamic scheduling.

    Currently based on creating FIFOs. Once a named FIFO has been created,
    attempts by other processes to create it again raise OSError with 
    errno = 17. The FIFO is created by default in $HOME/.DakotaEvalTiling,
    but a different location can be specified.
    """

    def __init__(self, num_tiles=None, lock_id="", lock_dir=""):
        """_TileLock context manager

        Keyword args:
            num_tiles: Number of tiles in the allocation.
            lock_id: Unique identifer for lock files.
            lock_dir: Directory for lock files
        """
        self._num_tiles = num_tiles
        self._id = lock_id
        self._dir = lock_dir

        try:
            os.mkdir(self._dir)
        except OSError as e:
            if e.errno != 17:
                raise e

    def __enter__(self):
        for tile in range(self._num_tiles):
            try:
                self._name = self._dir + os.sep + self._id + "." + str(tile)
                os.mkfifo(self._name)
                return tile
            except OSError as e:
                if e.errno != 17:  # error code for already-existing file
                    raise e
        raise ResourceError("Run requested, but no available tiles.")

    def __exit__(self, *args):
        os.remove(self._name)



def _mpirun(node_list, user_commands):
    """Use mpirun to launch a command in parallel

    Args:
        node_list (string): Relative node list formatted for use with -host option
        commands (list): Each item is a tuple: (applic_procs, tokenized command 
            to be run). len(commands) == 1 for SIMD model.
    Returns:
        returncode (int) of mpirun.
    """
    mpi_command = ["mpirun"]
    host_args = ["-host", node_list]
    # construct a composite user_command from the list of provided commands.
    user_command = []
    for command in user_commands:
        user_command += ["-np", str(command[0])] + command[1] + [":"]
    user_command.pop() # remove the final :
    returncode = subprocess.call(mpi_command + host_args + user_command)
    sys.stdout.flush()
    return returncode

def _calc_num_tiles(applic_tasks=None, tasks_per_node=None, num_nodes=None, 
        dedicated_master=None):
    """Compute the total number of tiles in the job, adjusting for a dedicated master

    Args:
        applic_tasks: Number of tasks for tile
        num_nodes: Total number of nodes in the allocation
        dedicated_master: Reserve a NODE or TILE for Dakota

    Returns:
        Number of tiles

    Raises:
        ResourseError: When a dedicated master takes up the entire allocation
    """
    if dedicated_master == NODE:
        if num_nodes == 1:
            raise ResourceError("Dedicated master node requested, but job has only one node.")
        num_tiles = (num_nodes-1)*tasks_per_node//applic_tasks 
    elif dedicated_master == TILE:
        num_tiles = num_nodes*tasks_per_node//applic_tasks - 1
        if num_tiles == 0:
            raise ResourceError("Dedicated master tile requested, but job has only one tile.")
    else: 
        num_tiles = num_nodes*tasks_per_node//applic_tasks
    return num_tiles

def tile_run_static(commands=[], dedicated_master=None, eval_num=None,
        parameters_file=None):
    """Run a command in parallel on an available tile assuming static scheduling

    Keyword args:
        commands (list): Each item is a tuple: (applic_procs, tokenized command 
            to be run). len(commands) == 1 for SIMD model.
        dedicated_master (NODE or TILE, optional): Reserve the first NODE or TILE 
            for Dakota (default: None).
        eval_num (int): Dakota evaluation number. Either eval_num or
            parameters_file is required. eval_num supercedes parameters_file.
        parameters_file (string): Extract an eval_num from this Dakota
            parameters_file.


    Returns:
        returncode (int) from mpirun

    Raises:
        ResourceError: When a dedicated master is requested but there is only
            one node in the job allocation.
    """
    # Determine the evaluation number
    if eval_num:
        pass
    elif parameters_file:
        params, results = read_parameters_file(parameters_file=parameters_file,
                results_file=UNNAMED)
        eval_num = params.eval_num
    else:
        raise 
    # Sum up the total number of tasks for the launch from each of the commands
    applic_tasks = 0
    for command in commands:
        applic_tasks += command[0]
    # Get information about the job from the environment.
    num_nodes, tasks_per_node, job_id = _get_job_info()
    # Compute the total number of tiles in the allocation, adjusting for a dedicated master
    num_tiles = _calc_num_tiles(applic_tasks, tasks_per_node, num_nodes, dedicated_master)
       
    # Calculate the available tile based on the eval_num, calculate its list of nodes, 
    # and mpirun the command(s) on these resources.
    tile = _calc_static_tile(eval_num, num_tiles)
    node_list = _get_node_list(tile, applic_tasks, tasks_per_node, dedicated_master)
    returncode = _mpirun(node_list, commands)
    return returncode

def tile_run_dynamic(commands=[], dedicated_master=None, lock_id=None, lock_dir=None):
    """Run a command in parallel on an available tile assuming dynamic scheduling

    Keyword args:
        commands (list): Each item is a tuple: (applic_procs, tokenized command 
            to be run). len(commands) == 1 for SIMD model.
        dedicated_master (NODE or TILE, optional): Reserve the first NODE or TILE 
            for Dakota (default: None).
        lock_id (str, optional): Unique prefix for lockfiles used to manage tiles.
        lock_dir (str, optional): Name of directory where lockfiles will be written.
    
    Returns:
        returncode (int) from mpirun

    Raises:
        ResourceError: When a dedicated master is requested but there is only
            one node in the job allocation, or when no available tiles were
            found.
    """
    # Sum up the total number of tasks for the launch from each of the commands
    applic_tasks = 0
    for command in commands:
        applic_tasks += command[0]
    # Get information about the job from the environment.
    num_nodes, tasks_per_node, job_id = _get_job_info()

    # Compute the total number of tiles in the allocation, adjusting for a dedicated master
    num_tiles = _calc_num_tiles(applic_tasks, tasks_per_node, num_nodes, dedicated_master)
    
    # Create default lock_id and lock_dir
    if lock_id is None:
        lock_id = job_id
    if lock_dir is None:
        lock_dir = os.environ["HOME"] + os.sep + ".DakotaEvalTiling"
    # Acquire an available tile, calculate its list of nodes, and mpirun the command(s)
    # on these resources.
    with _TileLock(num_tiles, lock_id, lock_dir) as tile:
        node_list = _get_node_list(tile, applic_tasks, tasks_per_node, dedicated_master)
        returncode = _mpirun(node_list, commands)
    sys.stdout.flush()
    return returncode

