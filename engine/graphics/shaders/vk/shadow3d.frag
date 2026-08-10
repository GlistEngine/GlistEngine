#version 450

// Depth-only, matching OpenGL's shadowmap_frag.glsl. Depth is produced by fixed
// function rasterisation, so this stage intentionally has no descriptors or work.
void main() {}
