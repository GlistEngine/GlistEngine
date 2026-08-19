#version 450

// 3D mesh shader for the Vulkan backend. Deliberately separate from the OpenGL
// color_vert.glsl: that one reads its matrices from default-block uniforms, which
// Vulkan/SPIR-V forbids, and carries shadow/fog/instancing machinery this path does
// not have yet. The lighting maths in the fragment stage is ported from
// color_frag.glsl so both backends shade a surface the same way.
//
// Only the attributes this stage reads are declared. The pipeline describes the
// gVertex layout by hand (see gVKPipeline.cpp) and hands over just the entries the
// shader actually consumes, so the locations here line up with that description.

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
// The bitangent attribute at location 4 is deliberately not read: it is derived
// from the normal and the tangent below, exactly as color_vert.glsl does.
layout(location = 3) in vec3 aTangent;
layout(location = 5) in vec3 aColor;

// Per instance rather than per vertex: it comes from a second vertex binding whose
// input rate is INSTANCE, which is Vulkan's equivalent of glVertexAttribDivisor(1).
// A mat4 occupies four consecutive locations, so this claims 6 through 9.
//
// There is no "is this instanced" flag, unlike color_vert.glsl's useInstancing. A
// draw that is not instanced binds a one-element buffer holding the identity
// matrix, so the multiply below leaves the model matrix untouched and one code path
// covers both cases.
layout(location = 6) in mat4 aInstanceModel;

// Everything that is the same for every mesh in a frame. Matches gVKSceneUniforms
// in gVKUniform.h field for field.
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
    // Declared to match mesh3d.frag, which reads them. Every shader binding a
    // uniform block has to declare it identically; leaving these out moves the light
    // array and the block stops meaning the same thing in each stage.
    int flags;
    vec4 fogcolor;
    vec4 fogparams;
    Light lights[8];
} scene;

// Per mesh. Held to 128 bytes, the smallest push constant range Vulkan guarantees,
// so this works on every conforming implementation rather than only on desktop
// drivers that happen to offer 256.
layout(push_constant) uniform Push {
    mat4 model;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    // x is shininess; the rest is padding kept so the block stays vec4 aligned.
    vec4 misc;
} pc;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec3 vFragPos;
layout(location = 2) out vec3 vColor;
layout(location = 3) out vec2 vTexCoords;
// The tangent alone, rather than the finished tangent space. This stage used to
// build the TBN matrix here and hand it over along with the view and fragment
// positions already rotated into it - fifteen floats of varyings, paid on every
// mesh in the scene whether or not it has a normal map, and interpolated per
// fragment. Only the tangent actually has to be interpolated; the basis is
// rebuilt in mesh3d.frag from it and the normal, and only where a normal map is
// bound. The Gram-Schmidt step moves with it, so a mesh without one now costs
// neither the varyings nor the maths.
layout(location = 4) out vec3 vTangent;
// Where this fragment falls in the shadow map, still homogeneous - the perspective
// divide happens in the fragment stage so it is done per pixel rather than
// interpolated, which is what color_vert.glsl does with FragPosLightSpace.
layout(location = 5) out vec4 vFragPosLightSpace;

void main() {
    // Matches color_vert.glsl's "model * instanceModel" ordering.
    mat4 model = pc.model * aInstanceModel;

    vec4 world = model * vec4(aPos, 1.0);
    // The projection already carries the Vulkan depth correction; see
    // gVKRenderEngine::setProjectionMatrix.
    gl_Position = scene.projection * scene.view * world;

    vFragPos = world.xyz;

    // Normals do not survive a non-uniform scale under the model matrix itself, so
    // the inverse-transpose is what keeps them perpendicular to the surface. The
    // OpenGL path builds this on the CPU; here it is cheaper to derive than to
    // spend another 48 bytes of the push constant budget carrying it.
    //
    // Derived as the cofactor matrix rather than transpose(inverse(m)), which is the
    // same thing scaled by the determinant: the columns of the cofactor matrix are
    // the cross products of the other two columns, so this is three cross products
    // instead of a nine-term inverse with a division, per vertex of every mesh in the
    // frame. Everything downstream normalises what it reads out of this, so a
    // positive scale factor changes nothing - but a negative one would flip the
    // normals of mirrored geometry, and the determinant's sign is one dot product
    // away because its first cross product has already been computed.
    mat3 m = mat3(model);
    mat3 normalmatrix = mat3(cross(m[1], m[2]), cross(m[2], m[0]), cross(m[0], m[1]));
    normalmatrix *= dot(m[0], normalmatrix[0]) < 0.0 ? -1.0 : 1.0;

    vNormal = normalmatrix * aNormal;
    // Not orthogonalised or normalised here: interpolation would break both anyway,
    // so mesh3d.frag redoes the Gram-Schmidt per fragment where it is needed.
    vTangent = normalmatrix * aTangent;
    vColor = aColor;
    vTexCoords = aTexCoords;

    vFragPosLightSpace = scene.lightmatrix * vec4(vFragPos, 1.0);
}
