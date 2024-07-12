#version 330 core

in vec3 nearpoint;
in vec3 farpoint;

uniform float near; 
uniform float far;

in mat4 fragview;
in mat4 fragprojection;

out vec4 outColor;

// draw grid lines
// TODO enable / disable axis color
vec4 grid(vec3 fragpos, float linespacing, bool drawaxis) {
    vec2 coord = fragpos.xz * linespacing; // setting the distance between the lines
    
    // use screen-space partial derivatives to compute the line width and falloff for Anti Aliasing.
    vec2 derivative = fwidth(coord); 
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    
    // default line color
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    
    // x axis color
    float minz = min(derivative.y, 1);
    if(fragpos.z > -0.1 * minz && fragpos.z < 0.1 * minz)
        color.x = 1.0;
    
    // z axis color
    float minx = min(derivative.x, 1);
    if(fragpos.x > -0.1 * minx && fragpos.x < 0.1 * minx)
        color.z = 1.0;
    
    return color;
}

// calculating depth to determine if the fragment is behind an object
float computeDepth (vec3 position) {
    vec4 clip_space_position = fragprojection * fragview * vec4 (position.xyz, 1.0);
    return (1.0 + (clip_space_position.z / clip_space_position.w)) * 0.5; //(ndc depth + 1.0) / 2 gets us the correct depth for opengl beacuse (max depth 1.0, min 0.0) 
}

// using the linear depth to determine the alpha of the lines to get the fading / fog effect from the near and far clips of the camera
float computeLinearDepth(vec3 pos) {
    vec4 clip_space_position = fragprojection * fragview * vec4(pos.xyz, 1.0);
    float clip_space_depth = (clip_space_position.z / clip_space_position.w) * 2.0 - 1.0; // shifting the ndc depth values to a range of [-1, 1]
    float lineardepth = (near * far * 2.0) / (far + near - clip_space_depth * (far - near)); // get linear value
    return lineardepth / far; // normalize
}

void main() {
	// since we want to draw the point at the floor (y = 0), we just need the intersection
    float t = -nearpoint.y / (farpoint.y - nearpoint.y);
    vec3 fragpos = nearpoint + t * (farpoint - nearpoint); // getting the intersection by solving for xy and zy lines using t at y = 0 (a line is defined as L: C + t*F)

    gl_FragDepth = computeDepth(fragpos);

    float lineardepth = computeLinearDepth(fragpos);
    float fading = max(0, (0.5 - lineardepth));

	// finally we draw the 2d grid
    outColor = (grid(fragpos, 10, true) + grid(fragpos, 1, true)) * float(t > 0);  // t must be bigger than 0 for a valid intersection point else just make it transparent since it doesnt intersect
    outColor.a *= fading;
}