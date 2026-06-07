# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Ramona Optics, Inc.
#
# PlatformIO extra-script for the bare-metal Teensy 4.0 env. The platform's
# built-in `_bare_arm.py` sets `-mcpu=cortex-m7` but not the Cortex-M7 FPU
# flags, and `build_flags` alone do not reach the link step — so the hard-float
# objects fail to merge with the soft-float default libraries. Apply the FPU
# selection to compile, assemble, and link uniformly.
Import("env")

# build_flags carry the FPU selection to the compile/assemble steps; the link
# step (gcc driver) needs the same selection to pick the hard-float multilib.
env.Append(LINKFLAGS=["-mfloat-abi=hard", "-mfpu=fpv5-d16"])
