#if VULKAN
#version 450
#endif
#if GLES
#version 300 es
precision highp float;
#endif
#if GLCORE
#version 330 core
#endif

// Depth-only. Depth is produced by fixed function rasterisation, so this stage
// intentionally has no descriptors and no work of its own.
void main() {}
