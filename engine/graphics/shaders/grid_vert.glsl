#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (location = 0) in vec2 aPos;
out vec3 nearpoint;
out vec3 farpoint;
out mat4 fragview;
out mat4 fragprojection;

uniform mat4 view;
uniform mat4 projection;

// grid positions are in xy clipped space
vec3 gridplane[6] = vec3[] (
    vec3( 1,  1, 0),
    vec3(-1, -1, 0), 
    vec3(-1,  1, 0),
    vec3(-1, -1, 0), 
    vec3( 1,  1, 0), 
    vec3( 1, -1, 0)
);

// unproject the plane coordinates so that the points are always at infinity
vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 projection) {
    mat4 viewinv = inverse(view);
    mat4 projinv = inverse(projection);
    vec4 unprojectedpoint =  viewinv * projinv * vec4(x, y, z, 1.0);
    return unprojectedpoint.xyz / unprojectedpoint.w;
}

void main() {
    vec3 point = gridplane[gl_VertexID].xyz;
    nearpoint = unprojectPoint(point.x, point.y, 0.0, view, projection).xyz; // unprojecting current vertex on the near plane
    farpoint = unprojectPoint(point.x, point.y, 1.0, view, projection).xyz; // unprojecting on the far plane
    fragview = view; // passing view and projection to fragment
    fragprojection = projection;
    gl_Position = vec4(point, 1.0); // use the clipped coordinates
}