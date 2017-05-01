import re
import os

__author__ = 'J. Adam Stephens'
__copyright__ = 'Copyright 2014 Sandia Corporation'
__license__ = 'GNU Lesser General Public License'

def is_me():
    try:
        os.environ["SLURM_JOBID"]
        return True
    except KeyError:
        return False

def get_job_info():
    tasks_per_node = os.environ["SLURM_TASKS_PER_NODE"]
    job_id = os.environ["SLURM_JOBID"]
    # TODO: Catch the case when there's only one node and a dedicated master is requested
    # Two cases
    # Case 1: Nodes are uniformly sized
    if len(tasks_per_node.split(",")) == 1:
        m = re.match("(\d+)(?:\(x(\d+)\))?", tasks_per_node)
        r = m.groups()
        tasks_per_node = int(r[0])
        num_nodes = 1
        if r[1]:
            num_nodes = int(r[1])
        return num_nodes, tasks_per_node, job_id

