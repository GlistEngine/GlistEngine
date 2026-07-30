#!/usr/bin/env bash
#
# Compiles the Vulkan 2D shaders in this directory to SPIR-V and embeds them as
# uint32 arrays in engine/core/gVKShaders.h. That header is committed so the
# engine builds without glslc present; only rerun this when a shader changes.
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
OUT="$(cd "$SHDIR/../../../core" && pwd)/gVKShaders.h"
TMP="$(mktemp)"
trap 'rm -f "$TMP"' EXIT

{
	echo "/*"
	echo " * gVKShaders.h"
	echo " *"
	echo " * SPIR-V for the Vulkan 2D shaders, generated from the .vert and .frag"
	echo " * sources in engine/graphics/shaders/vk by compile_vk_shaders.sh."
	echo " * Do not edit by hand: change the .vert/.frag source and rerun the script."
	echo " */"
	echo
	echo "#pragma once"
	echo
	echo "#include <cstdint>"
	echo
} > "$OUT"

emit() {
	local name="$1" file="$2"
	"$GLSLC" --target-env=vulkan1.2 -O "$SHDIR/$file" -mfmt=c -o "$TMP"
	{
		echo "static const uint32_t ${name}[] ="
		cat "$TMP"
		echo ";"
		echo
	} >> "$OUT"
}

emit gvkspv_color2d_vert color2d.vert
emit gvkspv_color2d_frag color2d.frag
emit gvkspv_image2d_vert image2d.vert
emit gvkspv_image2d_frag image2d.frag

echo "Wrote $OUT"
