#if GLES
#version 300 es
precision highp float;
#else
#version 330 core
#endif

void main() {
    // gl_FragDepth = gl_FragCoord.z;
}
