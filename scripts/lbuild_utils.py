import os

from pathlib import Path

def repo_path_rel_repolb(file, path):
    """
    Relocate given path to the path of the repo file.
    Copied from Taproot, which copied it from `modm/test/all/run_all.py`

    - file: __file__ that the function is called in
    - path: path relative to repo.lb file that you want get
    """
    return (Path(os.path.abspath(file)).parents[1] / path)