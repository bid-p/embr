"""
Writes compile_commands.json for clangd.

modm:build:compilation_db generates modm/tools/builder_compilation_db.py, which lists every modm, embr
and application source with its GCC flags. Each entry is then adapted so clang parses the code the way
arm-none-eabi-gcc compiles it:

- --target=arm-none-eabi, so clang parses for the right architecture.
- GCC's system include directories (newlib, libstdc++), queried from the cross compiler with each
  file's target flags, so clangd needs no --query-driver argument and works in any editor. GCC's
  compiler-internal headers (arm_acle.h, stddef.h...) are left out: clang uses its own.
- GCC's integer type definitions. For arm-none-eabi GCC makes int32_t a long, clang an int, which
  breaks overloads such as modm::IOStream::operator<<(int32_t) next to operator<<(int).
- GCC-only flags that clang rejects are dropped.
"""

import json
import os
import re
import shutil
import subprocess
import sys

MODM_GENERATOR = os.path.join("modm", "tools", "builder_compilation_db.py")
OUTPUT = "compile_commands.json"

# Flags that select the GCC multilib, and with it the system include directories and type sizes
TARGET_FLAG_PREFIXES = ("-mcpu=", "-mfpu=", "-mfloat-abi=", "-mthumb", "-std=")

# Accepted by arm-none-eabi-gcc but unknown to clang
GCC_ONLY_FLAGS = ("-finline-limit=", "-funsigned-bitfields", "-Wduplicated-cond",
                  "-Werror=maybe-uninitialized", "-Wlogical-op", "-Wno-volatile")

# Integer type macros that newlib's <stdint.h> builds on
TYPE_MACRO = re.compile(r"#define (__U?INT(?:_LEAST|_FAST)?\d+_TYPE__|__WINT_TYPE__) (.+)")

# Host include path variables would otherwise be reported as if they were the compiler's own
INCLUDE_ENV_VARS = ("CPATH", "C_INCLUDE_PATH", "CPLUS_INCLUDE_PATH", "OBJC_INCLUDE_PATH")

# The generator converts CPPDEFINES with map(), an iterator that only the first source consumes, so
# every later file loses its -D flags. Running it with map() returning a list works around that.
_RUN_GENERATOR = (
    "import builtins, runpy; "
    "_map = map; "
    "builtins.map = lambda *args: list(_map(*args)); "
    f"runpy.run_path({MODM_GENERATOR!r}, run_name='__main__')"
)


def _query_gcc(compiler, language, target_flags, extra_flags, cache={}):
    key = (compiler, language, tuple(target_flags), tuple(extra_flags))
    if key not in cache:
        env = {k: v for k, v in os.environ.items() if k.upper() not in INCLUDE_ENV_VARS}
        cache[key] = subprocess.run([compiler, f"-x{language}", *target_flags, *extra_flags, "-E", "-"],
                                    input="", capture_output=True, text=True, env=env)
    return cache[key]


def _gcc_internal_include_dir(compiler, cache={}):
    if compiler not in cache:
        output = subprocess.run([compiler, "-print-file-name=include"], capture_output=True, text=True)
        cache[compiler] = os.path.dirname(os.path.normpath(output.stdout.strip()))
    return cache[compiler]


def _gcc_system_includes(compiler, language, target_flags):
    lines = _query_gcc(compiler, language, target_flags, ["-v"]).stderr.splitlines()
    start = lines.index("#include <...> search starts here:") + 1
    end = lines.index("End of search list.")
    # GCC's own headers (lib/gcc/<target>/<version>/include and include-fixed: arm_acle.h, stddef.h...)
    # use GCC builtins; clang must use its own versions of them instead
    internal = _gcc_internal_include_dir(compiler) + os.sep
    return [d for d in (os.path.normpath(line.strip()) for line in lines[start:end])
            if not d.startswith(internal)]


def _gcc_type_macros(compiler, language, target_flags):
    output = _query_gcc(compiler, language, target_flags, ["-dM"]).stdout
    return TYPE_MACRO.findall(output)


def _convert(entry):
    tool, *args = entry["command"].split()

    # The generator writes "{TOOL} -o {OBJECT} -c {FLAGS} ...". Drop the object path: it isn't needed for
    # code navigation, and on Windows it can contain a literal carriage return ("build\rxmodule-src" in
    # the generated script), which splits it into a stray input file that confuses clangd.
    args = [a for a in args[args.index("-c"):] if not a.startswith(GCC_ONLY_FLAGS)]

    compiler = shutil.which(tool) or tool
    language = "c++" if tool.endswith("++") else "c"
    target_flags = [a for a in args if a.startswith(TARGET_FLAG_PREFIXES)]

    gcc_flags = []
    for name, value in _gcc_type_macros(compiler, language, target_flags):
        gcc_flags += [f"-U{name}", f"-D{name}={value}"]
    for directory in _gcc_system_includes(compiler, language, target_flags):
        gcc_flags += ["-isystem", directory]

    return {
        "directory": entry["directory"],
        "file": entry["file"],
        "arguments": [compiler, "--target=arm-none-eabi", *args, *gcc_flags],
    }


def generate(profile, source_dirs):
    # The generator only knows modm's debug and release profiles
    profile = "debug" if profile == "debug" else "release"
    # Silence the SyntaxWarnings from Windows paths in the generated script
    env = dict(os.environ, PYTHONWARNINGS="ignore")
    subprocess.run([sys.executable, "-c", _RUN_GENERATOR, f"--{profile}", *source_dirs],
                   check=True, stdout=subprocess.DEVNULL, env=env)
    with open(OUTPUT) as f:
        # Only compiled sources: the generator also emits a link step for the .elf
        entries = [e for e in json.load(f) if " -c " in e["command"]]
    with open(OUTPUT, "w") as f:
        json.dump([_convert(e) for e in entries], f, indent=2)
    return len(entries)


if __name__ == "__main__":
    profile = "debug" if "--debug" in sys.argv else "release"
    sources = [a for a in sys.argv[1:] if not a.startswith("--")] or ["src"]
    print(f"Wrote {OUTPUT} with {generate(profile, sources)} entries")
