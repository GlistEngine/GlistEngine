#version 450

// Vertex stage of the Vulkan PBR path, matching pbr_vert.glsl. Simpler than
// mesh3d.vert: the PBR fragment shader derives its tangent basis from screen-space
// derivatives rather than from a vertex tangent, so nothing but position, normal
// and texture coordinates has to travel.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
// Per instance, from binding 1; see mesh3d.vert for why this is always present.
layout(location = 6) in mat4 aInstanceModel;

// Declared in full even though this stage only reads the two matrices: a uniform
// block has to be described identically in every stage that names it, so leaving
// the tail out would change the layout rather than just hide it.
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
    // xyz the shadow-casting light's position, w whether a shadow map is bound.
    vec4 shadowlightpos;
    int lightnum;
    int enabledlights;
    int softshadows;
    Light lights[8];
} scene;

layout(push_constant) uniform Push {
    mat4 model;
    // Which maps this material supplies; see mesh3dpbr.frag.
    ivec4 maps0;
    ivec4 maps1;
} pc;

layout(location = 0) out vec2 vTexCoords;
layout(location = 1) out vec3 vWorldPos;
layout(location = 2) out vec3 vNormal;
// Where this vertex lands in the shadow-casting light's clip space, which is what
// the fragment stage compares against the depth map. Same as in mesh3d.vert.
layout(location = 3) out vec4 vFragPosLightSpace;

void main() {
    mat4 model = pc.model * aInstanceModel;

    vec4 world = model * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vTexCoords = aTexCoords;
    vNormal = mat3(transpose(inverse(model))) * aNormal;
    vFragPosLightSpace = scene.lightmatrix * world;

    gl_Position = scene.projection * scene.view * world;
}
