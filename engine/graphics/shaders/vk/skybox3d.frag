#version 450

// One face of the skybox. Nothing is lit here: the sky is the light source, not a
// surface receiving it, which is why this has a pipeline of its own rather than
// going through mesh3d with a flag - there is no room left in that shader's 128
// byte push constant block for one, and a sky quad needs none of what it carries.

layout(location = 0) in vec2 vTexCoords;

layout(set = 0, binding = 0) uniform sampler2D facemap;

layout(location = 0) out vec4 outColor;

void main() {
    // Alpha forced to 1: a sky face is opaque whatever the source image says, and
    // the 3D pipelines do not blend anyway.
    outColor = vec4(texture(facemap, vTexCoords).rgb, 1.0);
}
