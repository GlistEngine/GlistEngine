#version 330 core

out vec4 fragcolor;
uniform float utime; 

void main() {
    vec3 glowcolor = vec3(0.55, 0.76, 0.44); 
    float pulse = (sin(utime * 3.0) + 1.0) / 2.0;
    
    fragcolor = vec4(glowcolor * pulse, pulse);
}