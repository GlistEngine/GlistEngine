#version 450

// Fragment stage of the shadow pass. Writes nothing - the render pass has no colour
// attachment and depth comes from the rasteriser - and exists for one reason: to
// throw away the fragments a cutout material has punched holes in, so foliage casts
// a perforated shadow instead of the shadow of its bounding silhouette.
//
// This deliberately goes past what the OpenGL backend does: shadowmap_frag.glsl is
// empty there, so a cutout mesh casts a solid shadow on that path. The two backends
// disagree here on purpose - a tree whose leaves are holes casting a filled blob is
// wrong, and matching that would only be worth it if the wrong result were the one
// anything depended on.

layout(location = 0) in vec2 vTexCoords;

// The material's diffuse map, and always a real descriptor: a mesh with no map of
// its own gets the 1x1 white texture, whose alpha is 1 everywhere and so discards
// nothing. misc.x still says whether to sample it at all, so an opaque mesh costs
// no texture fetch.
layout(set = 0, binding = 0) uniform sampler2D diffusemap;

layout(push_constant) uniform Push {
    mat4 lightmodel;
    // x: non-zero when the caster has a cutout diffuse map worth sampling.
    vec4 misc;
} pc;

void main() {
    // The same 0.5 threshold mesh3d.frag uses when it discards in the shading pass.
    // They have to agree: a fragment that survives here but is cut out there would
    // cast the shadow of something that was never drawn.
    if (pc.misc.x > 0.0 && texture(diffusemap, vTexCoords).a < 0.5) discard;
}
