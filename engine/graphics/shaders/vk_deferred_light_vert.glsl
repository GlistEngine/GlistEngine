#version 450

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords;
layout(location = 0) out vec2 vTexCoords;

// Keep set 0 layout-compatible with the existing scene descriptor set. That set
// is shared with mesh rendering, where both shader stages declare the block.
struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float constant;
    float linear;
    float quadratic;
    float cutOff;
    float outerCutOff;
};

layout(set = 0, binding = 0) uniform Scene {
    mat4 projection;
    mat4 view;
    mat4 lightmatrix;
    vec4 viewpos;
    vec4 globalambientcolor;
    vec4 rendercolor;
    vec4 shadowlightpos;
    int lightnum;
    int enabledlights;
    int softshadows;
    int flags;
    vec4 fogcolor;
    vec4 fogparams;
    Light lights[8];
} scene;

void main() {
    vTexCoords = aTexCoords;
    gl_Position = vec4(aPosition, 0.0, 1.0);
    // Negative light counts are invalid engine state. Keeping this runtime branch
    // makes set 0 visible to the vertex stage so its layout exactly matches the
    // scene sets already allocated for the forward mesh pipeline.
    if (scene.lightnum < 0) gl_Position.xy = -gl_Position.xy;
}
