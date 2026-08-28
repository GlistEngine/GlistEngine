#!/usr/bin/env bash
#
# Convenience wrapper: regenerates engine/core/gVKShaders.h from the shaders in
# this directory. The build does this on its own whenever a shader changes, so
# running it by hand is only needed when working outside the build.
#
# The actual work lives in gen_vk_shaders.cmake, which is what the build calls
# and what Windows uses. Keeping one implementation means the two cannot drift.
#
# Usage: ./compile_vk_shaders.sh   (override the compiler with GLSLC=/path/to/glslc)

set -euo pipefail

GLSLC="${GLSLC:-glslc}"
if ! command -v "$GLSLC" >/dev/null 2>&1; then
	if [ -x /opt/homebrew/bin/glslc ]; then GLSLC=/opt/homebrew/bin/glslc; else
		echo "glslc not found; set GLSLC=/path/to/glslc" >&2; exit 1
	fi
fi

SHDIR="$(cd "$(dirname "$0")" && pwd)"
OUT="$(cd "$SHDIR/../../core" && pwd)/gVKShaders.h"

exec cmake -DGLSLC="$GLSLC" -DSHADER_DIR="$SHDIR" -DOUT="$OUT" -P "$SHDIR/gen_vk_shaders.cmake"
