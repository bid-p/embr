import glob
import lxml.etree as ET

from lbuild_utils import repo_path_rel_repolb

parsed_board_info = {}

def parse_board_info(device):
    '''
    Taken from https://gitlab.com/aruw/controls/taproot/-/blob/release/lbuild-scripts/board_info_parser.py
    '''
    global parsed_board_info

    # change - to _ in device name
    folder_name = device.replace("-", "_")

    device_file_names = glob.glob(str(repo_path_rel_repolb(__file__, f"modm/src/modm/board/{folder_name}/board.xml")))
    assert len(device_file_names) == 1, f"Device {device} not found in modm /board folder"
    device_file_name = device_file_names[0]

    if device not in parsed_board_info:
        # Board XML not already parsed
        parser = ET.XMLParser(no_network=True)
        xmlroot = ET.parse(device_file_name, parser=parser)
        xmlroot.xinclude()
        parsed_board_info[device] = xmlroot.getroot()

    return parsed_board_info[device]

def parse_default_modm_modules():
    default_modules_filename = glob.glob(str(repo_path_rel_repolb(__file__, "modm-project-files/default_modm_modules.xml")))
    assert len(default_modules_filename) == 1, "Default modm modules file not found"
    
    parser = ET.XMLParser(no_network=True)
    xmlroot = ET.parse(default_modules_filename[0], parser=parser)
    xmlroot.xinclude()
    module_metadata = xmlroot.getroot()
    return module_metadata

def get_comma_separated_values(string):
    """
    Convert a comma separated string into a list of strings with whitespace removed.
    """
    return [x.strip() for x in string.split(',')]