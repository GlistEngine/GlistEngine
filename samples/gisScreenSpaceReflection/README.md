# Screen-Space Reflections (SSR)

Real-time, depth-aware screen-space reflections with temporal reprojection, shown on a reflective floor and a metallic sphere in a procedurally generated sunset environment.

## Demo
<video src="https://github.com/user-attachments/assets/e2cb9e6f-78f5-4573-a3c3-8290758eb3f6" controls></video>


## What this sample shows

- **`engine/graphics/shaders/screenspacereflection_vert.glsl` / `_frag.glsl`** — the SSR shader itself. It raymarches the scene's own depth buffer to find what each reflective surface should show, falls back to the skybox (or a flat color) when a ray misses, and blurs the result based on the surface's roughness.
- **`src/gCanvas.cpp`** — a plain, class-free example of driving that shader from application code. The step-by-step guide at the top of the file walks through the exact call sequence; the `ssr*` methods right below it are grouped together so they're easy to find separately from the scene-building code.

## How it works, in short

1. Load the SSR shader and allocate the scene-capture FBO plus the two ping-pong reflection FBOs.
2. Attach the shader to the material of every mesh that should be reflective.
3. Each frame, capture the whole scene (color + depth) into an offscreen buffer.
4. Raymarch reflections for the reflective meshes against that captured buffer, blending with the previous frame's result for stability.
5. Draw the scene normally, then redraw the reflective meshes once more so the shader can composite the computed reflection onto the final image.

## Controls

- `W` `A` `S` `D` — move
- `Q` / `E` — down / up
- Mouse — look around
