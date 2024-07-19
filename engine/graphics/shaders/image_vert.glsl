#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>\n"

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
uniform bool isSubPart;
uniform vec2 subPos;
uniform vec2 subScale;

void main() {
    if (isSubPart) {
        TexCoords = (vertex.zw + subPos) / subScale;
    } else {
        TexCoords = vertex.zw;
    }
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
