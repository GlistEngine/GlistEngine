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

// See skybox_vert.glsl for why the two dialects hold different shaders rather
// than one shader with different declarations.

#if VULKAN
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
#endif

#if OPENGL
out vec4 FragColor;

in vec3 TexCoords;
flat in int mIsHDR;
vec4 fc;

uniform samplerCube skymap;

void main() {
    if (mIsHDR == 0) {
        fc = vec4(1.0, 0.0, 0.0, 1.0);
        fc = texture(skymap, TexCoords);
    } else if (mIsHDR == 1) {
        vec3 envColor = textureLod(skymap, TexCoords, 0.0).rgb; //environmentMap->skymap
        envColor = envColor / (envColor + vec3(1.0));
        envColor = pow(envColor, vec3(1.0 / 2.2));
        fc = vec4(envColor, 1.0);
        fc = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fc = vec4(1.0, 0.0, 0.0, 1.0);
    }
    FragColor = fc;
}
#endif
