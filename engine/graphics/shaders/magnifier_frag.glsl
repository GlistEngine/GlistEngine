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

#if VULKAN
layout (location = 0) out vec4 FragColor;
layout (location = 0) in vec2 TexCoords;
layout (set = 0, binding = 0) uniform sampler2D screenTexture;

layout (set = 0, binding = 1) uniform Params {
    float mousex;
    float mousey;
    float screenwidth;
    float screenheight;
    float radius;
    float zoom;
    float softness;
};
#endif

#if GLES
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float mousex;
uniform float mousey;
uniform float screenwidth;
uniform float screenheight;
uniform float radius;
uniform float zoom;
uniform float softness;
#endif

#if GLCORE
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTexture;
uniform float mousex;
uniform float mousey;
uniform float screenwidth;
uniform float screenheight;
uniform float radius;
uniform float zoom;
uniform float softness;
#endif

void main()
{
    vec2 resolution = max(
        vec2(screenwidth, screenheight),
        vec2(1.0)
    );

    // Mouse origin is top-left, texture origin is bottom-left.
    vec2 mouseuv = vec2(
        mousex / resolution.x,
        1.0 - mousey / resolution.y
    );

    vec2 pixeldelta = (TexCoords - mouseuv) * resolution;
    float lensdistance = length(pixeldelta);

    float safezoom = max(zoom, 1.0);
    vec2 magnifieduv = mouseuv +
        (TexCoords - mouseuv) / safezoom;

    magnifieduv = clamp(magnifieduv, vec2(0.0), vec2(1.0));

    vec4 originalcolor = texture(screenTexture, TexCoords);
    vec4 magnifiedcolor = texture(screenTexture, magnifieduv);

    float innerradius = max(radius - softness, 0.0);
    float lensmask = 1.0 - smoothstep(
        innerradius,
        radius,
        lensdistance
    );

    FragColor = mix(
        originalcolor,
        magnifiedcolor,
        lensmask
    );
}