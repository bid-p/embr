from SCons.Script import *

CMD_LINE_ARGS = 1
HARDWARE_BUILD_TARGET_ACCEPTED_ARGS = ["build", "run", "size", "gdb"]
VALID_BUILD_PROFILES = ["debug", "release", "fast"]
VALID_PROFILING_TYPES = ["true", "false"]

USAGE = "Usage: scons <target> [profile=<debug|release|fast>] [profiling=<true|false>]\n\
    \"<target>\" is one of:\n\
        - \"build\": build all code for the hardware platform.\n\
        - \"run\": build all code for the hardware platform, and deploy it to the board via a connected ST-Link.\n\
        - \"size\": build all code for the hardware platform, and display build size information.\n\
        - \"gdb\": build all code for the hardware platform, opens a gdb session.\n\
        - \"build-tests\": build core code and tests for the current host platform.\n\
        - \"run-tests\": build core code and tests for the current host platform, and execute them locally with the test runner.\n\
        - \"run-tests-gcov\": builds core code and tests, executes them locally, and captures and prints code coverage information\n\
        - \"build-sim\": build all code for the simulated environment, for the current host platform.\n\
        - \"run-sim\": build all code for the simulated environment, for the current host platform, and execute the simulator locally."

def parse_args():
    args = {
        "TARGET_ENV": "",
        "BUILD_PROFILE": "",
        "PROFILING": ""
    }

    if len(COMMAND_LINE_TARGETS) > CMD_LINE_ARGS:
        raise Exception("Too many arguments provided. " + USAGE)
    
    build_target = COMMAND_LINE_TARGETS[0]
    if build_target == "help":
        print(USAGE)
        exit(0)
    elif build_target not in HARDWARE_BUILD_TARGET_ACCEPTED_ARGS:
        raise Exception("Invalid build target provided. " + USAGE)
    else:
        args["TARGET_ENV"] = "hardware"
    
    default_build_profile = "release"
    default_profiling = "false"

    args["BUILD_PROFILE"] = ARGUMENTS.get("profile", default_build_profile)
    ARGUMENTS["profile"] = args["BUILD_PROFILE"]
    if args["BUILD_PROFILE"] not in VALID_BUILD_PROFILES:
        raise Exception("Invalid build profile provided. " + USAGE)
    
    args["PROFILING"] = ARGUMENTS.get("profiling", default_profiling)
    ARGUMENTS["profiling"] = args["PROFILING"]
    if args["PROFILING"] not in VALID_PROFILING_TYPES:
        raise Exception("Invalid profiling type provided. " + USAGE)
    
    return args