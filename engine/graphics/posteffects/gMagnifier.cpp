/*
 * gMagnifier.cpp
 *
 *  Created on: 26 Aug 2026
 *      Author: Bahar Kucukozer, Mehmet Sefa Ciftci
 */

#include "gMagnifier.h"

gMagnifier::gMagnifier(float radius, float zoom, float softness) {
	mousex = 0.0f;
	mousey = 0.0f;

	screenwidth = 1.0f;
	screenheight = 1.0f;

	this->radius = radius > 0.0f ? radius : 1.0f;
	this->zoom = zoom >= 1.0f ? zoom : 1.0f;
	this->softness = softness > 0.0f ? softness : 0.1f;

	shader = new gShader();

	renderer->setBuiltinShaderType(gRenderer::BUILTINSHADER_MAGNIFIER);
	shader->loadProgram(getVertSrc(), getFragSrc());
	renderer->setBuiltinShaderType(gRenderer::BUILTINSHADER_NONE);

	use();
}

gMagnifier::~gMagnifier() {
	delete shader;
}

void gMagnifier::use() {
	shader->use();

	shader->setFloat("mousex", mousex);
	shader->setFloat("mousey", mousey);
	shader->setFloat("screenwidth", screenwidth);
	shader->setFloat("screenheight", screenheight);
	shader->setFloat("radius", radius);
	shader->setFloat("zoom", zoom);
	shader->setFloat("softness", softness);
}

void gMagnifier::setMousePosition(float mousex, float mousey) {
	this->mousex = mousex;
	this->mousey = mousey;
}

void gMagnifier::setScreenSize(float width, float height) {
	screenwidth = width > 0.0f ? width : 1.0f;
	screenheight = height > 0.0f ? height : 1.0f;
}

void gMagnifier::setRadius(float radius) {
	this->radius = radius > 0.0f ? radius : 1.0f;
}

void gMagnifier::setZoom(float zoom) {
	this->zoom = zoom >= 1.0f ? zoom : 1.0f;
}

void gMagnifier::setSoftness(float softness) {
	this->softness = softness > 0.0f ? softness : 0.1f;
}

const std::string gMagnifier::getVertSrc() {
	const char* shadersource =
			"#if VULKAN\n"
			"#version 450\n"
			"#endif\n"
			"#if GLES\n"
			"#version 300 es\n"
			"precision highp float;\n"
			"#endif\n"
			"#if GLCORE\n"
			"#version 330 core\n"
			"#endif\n"

			"#if VULKAN\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"layout (location = 0) out vec2 TexCoords;\n"
			"#endif\n"

			"#if GLES\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"out vec2 TexCoords;\n"
			"#endif\n"

			"#if GLCORE\n"
			"layout (location = 0) in vec2 aPos;\n"
			"layout (location = 1) in vec2 aTexCoords;\n"
			"out vec2 TexCoords;\n"
			"#endif\n"

			"void main()\n"
			"{\n"
			"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
			"    TexCoords = aTexCoords;\n"
			"}\n";

	return std::string(shadersource);
}

const std::string gMagnifier::getFragSrc() {
	const char* shadersource =
			"#if VULKAN\n"
			"#version 450\n"
			"#endif\n"
			"#if GLES\n"
			"#version 300 es\n"
			"precision highp float;\n"
			"#endif\n"
			"#if GLCORE\n"
			"#version 330 core\n"
			"#endif\n"

			"#if VULKAN\n"
			"layout (location = 0) out vec4 FragColor;\n"
			"layout (location = 0) in vec2 TexCoords;\n"
			"layout (set = 0, binding = 0) uniform sampler2D screenTexture;\n"
			"layout (set = 0, binding = 1) uniform Params {\n"
			"    float mousex;\n"
			"    float mousey;\n"
			"    float screenwidth;\n"
			"    float screenheight;\n"
			"    float radius;\n"
			"    float zoom;\n"
			"    float softness;\n"
			"};\n"
			"#endif\n"

			"#if GLES\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float mousex;\n"
			"uniform float mousey;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"uniform float radius;\n"
			"uniform float zoom;\n"
			"uniform float softness;\n"
			"#endif\n"

			"#if GLCORE\n"
			"out vec4 FragColor;\n"
			"in vec2 TexCoords;\n"
			"uniform sampler2D screenTexture;\n"
			"uniform float mousex;\n"
			"uniform float mousey;\n"
			"uniform float screenwidth;\n"
			"uniform float screenheight;\n"
			"uniform float radius;\n"
			"uniform float zoom;\n"
			"uniform float softness;\n"
			"#endif\n"

			"void main()\n"
			"{\n"
			"    vec2 resolution = max(\n"
			"        vec2(screenwidth, screenheight),\n"
			"        vec2(1.0)\n"
			"    );\n"

			// Mouse origin is top-left, texture origin is bottom-left.
			"    vec2 mouseuv = vec2(\n"
			"        mousex / resolution.x,\n"
			"        1.0 - mousey / resolution.y\n"
			"    );\n"

			"    vec2 pixeldelta = (TexCoords - mouseuv) * resolution;\n"
			"    float lensdistance = length(pixeldelta);\n"

			"    float safezoom = max(zoom, 1.0);\n"
			"    vec2 magnifieduv = mouseuv +\n"
			"        (TexCoords - mouseuv) / safezoom;\n"

			"    magnifieduv = clamp(magnifieduv, vec2(0.0), vec2(1.0));\n"

			"    vec4 originalcolor = texture(screenTexture, TexCoords);\n"
			"    vec4 magnifiedcolor = texture(screenTexture, magnifieduv);\n"

			"    float innerradius = max(radius - softness, 0.0);\n"
			"    float lensmask = 1.0 - smoothstep(\n"
			"        innerradius,\n"
			"        radius,\n"
			"        lensdistance\n"
			"    );\n"

			"    FragColor = mix(\n"
			"        originalcolor,\n"
			"        magnifiedcolor,\n"
			"        lensmask\n"
			"    );\n"
			"}\n";

	return std::string(shadersource);
}
