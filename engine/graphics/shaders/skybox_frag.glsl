#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

out vec4 FragColor;

in vec3 TexCoords;
flat in int mIsHDR;
vec4 fc;

uniform samplerCube skymap;

void main() {
    if (mIsHDR == 0) {
        fc = vec4(1.0, 0.0, 0.0, 1.0);
        fc = texture(skymap, TexCoords);
    } else if (mIsHDR == 1) {
        vec3 envColor = textureLod(skymap, TexCoords, 0.0).rgb; //environmentMap->skymap
        envColor = envColor / (envColor + vec3(1.0));
        envColor = pow(envColor, vec3(1.0 / 2.2));
        fc = vec4(envColor, 1.0);
        fc = vec4(0.0, 1.0, 0.0, 1.0);
    } else {
        fc = vec4(1.0, 0.0, 0.0, 1.0);
    }
    FragColor = fc;
}