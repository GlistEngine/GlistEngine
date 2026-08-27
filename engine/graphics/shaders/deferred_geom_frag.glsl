#version 330 core

layout (location = 1) out vec3 gPosition; //GL_COLOR_ATTACHMENT0
layout (location = 2) out vec3 gNormal;   // GL_COLOR_ATTACHMENT1
layout (location = 3) out vec4 gAlbedo;   // GL_COLOR_ATTACHMENT2

//Inputs from Vertex Shader
in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;

void main() {
	//Position
	gPosition = FragPos;
	
	//Direction
	gNormal = normalize(Normal);
	
	//Color
	gAlbedo.rgb = texture(texture_diffuse1, TexCoords).rgb;
	gAlbedo.a = 1.0f; // Opaque Object
}