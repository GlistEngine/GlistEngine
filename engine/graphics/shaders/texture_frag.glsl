#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 gPosition; 
layout (location = 2) out vec4 gNormal;   
layout (location = 3) out vec4 gAlbedo;  

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform int isDeferred;

void main() {
	if(isDeferred == 1) {
		vec4 texColor = texture(texture_diffuse1, TexCoords);
		if(texColor.a < 0.5) discard;
		gPosition = vec4(FragPos, 32.0); // Default shininess
		gNormal = vec4(0.0); // Zero normal acts as an unlit mask
		gAlbedo = texColor;
		FragColor = vec4(0.0);
	
		return;
	}
	
	FragColor = texture(texture_diffuse1, TexCoords);	
	
	gPosition = vec4(FragPos, 32.0);
    gNormal = vec4(normalize(Normal), 0.0);
    gAlbedo = texture(texture_diffuse1, TexCoords);

}