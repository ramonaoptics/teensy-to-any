# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Ramona Optics, Inc.
#
# Fetch the pinned, permissively-licensed vendor sources the bare-metal Teensy 4
# build needs, into vendor/ (gitignored). Runs two ways:
#
#   * standalone:   python tools/fetch_mcux.py [--force]
#   * PIO pre-hook: extra_scripts = pre:tools/fetch_mcux.py
#                   (also appends the include paths to the build env)
#
# Pinned by exact tag so builds are reproducible. A pin bump is an explicit
# edit to the versions below.
#
#   NXP MCUXpresso SDK  (BSD-3-Clause) : i.MX RT1062 CMSIS device registers
#   ARM CMSIS_5         (Apache-2.0)   : Cortex-M7 core headers
#
# These two are subpaths of large CMake/west monorepos with no root
# library.json, and PlatformIO's package manager drops monorepo subpaths it
# doesn't recognize as a library layout — so it cannot pin them natively. This
# script fetches exactly those subpaths. TinyUSB, by contrast, IS a proper PIO
# library and is pinned natively via `lib_deps` in platformio.ini (not here).

import os
import sys
import urllib.request

MCUX_TAG = "MCUX_2.16.000"
CMSIS_TAG = "5.9.0"

try:
    # standalone run: resolve relative to this file
    VENDOR_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "vendor")
except NameError:
    # under PlatformIO/SCons __file__ is undefined; cwd is the project root
    VENDOR_DIR = os.path.join(os.getcwd(), "vendor")

_MCUX_BASE = (
    "https://raw.githubusercontent.com/nxp-mcuxpresso/mcux-sdk/%s/devices/MIMXRT1062"
    % MCUX_TAG
)
_CMSIS_BASE = (
    "https://raw.githubusercontent.com/ARM-software/CMSIS_5/%s/CMSIS/Core/Include"
    % CMSIS_TAG
)

# (url, destination relative to vendor/)
MANIFEST = [
    # --- NXP i.MX RT1062 device (CMSIS register definitions) -------------
    (_MCUX_BASE + "/MIMXRT1062.h", "mcux/MIMXRT1062.h"),
    (_MCUX_BASE + "/MIMXRT1062_features.h", "mcux/MIMXRT1062_features.h"),
    (_MCUX_BASE + "/fsl_device_registers.h", "mcux/fsl_device_registers.h"),
    (_MCUX_BASE + "/system_MIMXRT1062.h", "mcux/system_MIMXRT1062.h"),
    (_MCUX_BASE + "/system_MIMXRT1062.c", "mcux/system_MIMXRT1062.c"),
    # --- ARM CMSIS Cortex-M7 core ---------------------------------------
    (_CMSIS_BASE + "/core_cm7.h", "mcux/CMSIS/core_cm7.h"),
    (_CMSIS_BASE + "/cmsis_version.h", "mcux/CMSIS/cmsis_version.h"),
    (_CMSIS_BASE + "/cmsis_compiler.h", "mcux/CMSIS/cmsis_compiler.h"),
    (_CMSIS_BASE + "/cmsis_gcc.h", "mcux/CMSIS/cmsis_gcc.h"),
    (_CMSIS_BASE + "/mpu_armv7.h", "mcux/CMSIS/mpu_armv7.h"),
    (_CMSIS_BASE + "/cachel1_armv7.h", "mcux/CMSIS/cachel1_armv7.h"),
]

# Include dirs (relative to vendor/) exposed to the build.
INCLUDE_DIRS = ["mcux", "mcux/CMSIS"]


def fetch(force=False):
    for url, rel in MANIFEST:
        dest = os.path.normpath(os.path.join(VENDOR_DIR, rel))
        if os.path.exists(dest) and not force:
            continue
        os.makedirs(os.path.dirname(dest), exist_ok=True)
        sys.stdout.write("fetch_mcux: %s\n" % rel)
        urllib.request.urlretrieve(url, dest)
    return os.path.normpath(VENDOR_DIR)


# --- entry points ----------------------------------------------------------
try:
    Import("env")  # noqa: F821  (injected by PlatformIO/SCons)
    _UNDER_PIO = True
except NameError:
    _UNDER_PIO = False

if _UNDER_PIO:
    _vendor = fetch(force=False)
    env.Append(  # noqa: F821
        CPPPATH=[os.path.join(_vendor, d) for d in INCLUDE_DIRS]
    )
elif __name__ == "__main__":
    fetch(force="--force" in sys.argv)
    print("fetch_mcux: vendor sources ready in %s" % os.path.normpath(VENDOR_DIR))
