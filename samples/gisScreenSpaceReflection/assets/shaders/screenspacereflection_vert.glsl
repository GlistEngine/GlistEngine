#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (location = 0) in vec3 apos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoords;

out vec3 viewpos;
out vec3 viewnormal;
out vec3 worldpos;
out vec2 texcoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const float depthbias = 0.0001;

void main() {
	vec4 worldposition = model * vec4(apos, 1.0);
	vec4 viewposition = view * worldposition;
	worldpos = worldposition.xyz;
	viewpos = viewposition.xyz;
	viewnormal = mat3(view) * mat3(transpose(inverse(model))) * anormal;
	texcoord = atexcoords;
	gl_Position = projection * view * worldposition;
	gl_Position.z -= depthbias * gl_Position.w;
}
