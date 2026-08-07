/*
 * gFXAA.cpp
 *
 *  Created on: 6 Agu 2026
 *      Author: Numan Affan Kahya
 *
 *  Post-process class implementing FXAA (Fast Approximate Anti-Aliasing).
 */

#include "gFXAA.h"

gFXAA::gFXAA(float subpixelblend, float edgethresholdmin, float edgethresholdmax)
	: subpixelblend(subpixelblend), edgethresholdmin(edgethresholdmin), edgethresholdmax(edgethresholdmax) {
	shader = new gShader();
	shader->loadProgram(getVertSrc(), getFragSrc());
	shader->use();
	shader->setInt("screenTexture", 0);
	shader->setFloat("subpixelblend", subpixelblend);
	shader->setFloat("edgethresholdmin", edgethresholdmin);
	shader->setFloat("edgethresholdmax", edgethresholdmax);

	glGenSamplers(1, &linearsampler);
	glSamplerParameteri(linearsampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(linearsampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(linearsampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(linearsampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

gFXAA::~gFXAA() {
	delete shader;
	glDeleteSamplers(1, &linearsampler);
}

void gFXAA::use() {
	shader->use();
}

void gFXAA::render(gFbo& src, gFbo& dst) {
	dst.bind();
	use();

	glBindSampler(0, linearsampler);

	renderer->bindQuadVAO();
	src.getTexture().bind();

	GLint activeunit;
	glGetIntegerv(GL_ACTIVE_TEXTURE, &activeunit);

	renderer->drawFullscreenQuad();

	glBindSampler(0, 0);
}

void gFXAA::setSubpixelBlend(float value) {
	subpixelblend = value;
	shader->use();
	shader->setFloat("subpixelblend", subpixelblend);
}

void gFXAA::setEdgeThresholdMin(float value) {
	edgethresholdmin = value;
	shader->use();
	shader->setFloat("edgethresholdmin", edgethresholdmin);
}

void gFXAA::setEdgeThresholdMax(float value) {
	edgethresholdmax = value;
	shader->use();
	shader->setFloat("edgethresholdmax", edgethresholdmax);
}

const std::string gFXAA::getVertSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"layout (location = 0) in vec2 aPos;"
			"layout (location = 1) in vec2 aTexCoords;"
			""
			"out vec2 TexCoords;"
			""
			"void main()"
			"{"
			"	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
			"   TexCoords = aTexCoords;"
			"}\n";
	return std::string(shadersource);
}

const std::string gFXAA::getFragSrc() {
	const char* shadersource =
			"#version 330 core\n"
			"out vec4 FragColor;"
			""
			"in vec2 TexCoords;"
			""
			"uniform sampler2D screenTexture;"
			"uniform float subpixelblend;"
			"uniform float edgethresholdmin;"
			"uniform float edgethresholdmax;"
			""
			"float luma(vec3 c) {"
			"	return dot(c, vec3(0.299, 0.587, 0.114));"
			"}"
			""
			"void main()"
			"{"
			"	vec2 texelsize = 1.0 / vec2(textureSize(screenTexture, 0));"
			""
			"	vec3 colorcenter = texture(screenTexture, TexCoords).rgb;"
			""
			"	float lumacenter = luma(colorcenter);"
			"	float lumadown  = luma(texture(screenTexture, TexCoords + vec2(0.0, -texelsize.y)).rgb);"
			"	float lumaup    = luma(texture(screenTexture, TexCoords + vec2(0.0,  texelsize.y)).rgb);"
			"	float lumaleft  = luma(texture(screenTexture, TexCoords + vec2(-texelsize.x, 0.0)).rgb);"
			"	float lumaright = luma(texture(screenTexture, TexCoords + vec2( texelsize.x, 0.0)).rgb);"
			""
			"	float lumamin = min(lumacenter, min(min(lumadown, lumaup), min(lumaleft, lumaright)));"
			"	float lumamax = max(lumacenter, max(max(lumadown, lumaup), max(lumaleft, lumaright)));"
			"	float lumarange = lumamax - lumamin;"
			""
			"	if (lumarange < max(edgethresholdmin, lumamax * edgethresholdmax)) {"
			"		FragColor = vec4(colorcenter, 1.0);"
			"		return;"
			"	}"
			""
			"	float lumadownleft  = luma(texture(screenTexture, TexCoords + vec2(-texelsize.x, -texelsize.y)).rgb);"
			"	float lumaupright   = luma(texture(screenTexture, TexCoords + vec2( texelsize.x,  texelsize.y)).rgb);"
			"	float lumaupleft    = luma(texture(screenTexture, TexCoords + vec2(-texelsize.x,  texelsize.y)).rgb);"
			"	float lumadownright = luma(texture(screenTexture, TexCoords + vec2( texelsize.x, -texelsize.y)).rgb);"
			""
			"	float lumadownup = lumadown + lumaup;"
			"	float lumaleftright = lumaleft + lumaright;"
			""
			"	float edgevertical = abs(lumaupleft + lumaupright - 2.0 * lumaup)"
			"		+ 2.0 * abs(lumaleftright - 2.0 * lumacenter)"
			"		+ abs(lumadownleft + lumadownright - 2.0 * lumadown);"
			"	float edgehorizontal = abs(lumaupright + lumadownright - 2.0 * lumaright)"
			"		+ 2.0 * abs(lumadownup - 2.0 * lumacenter)"
			"		+ abs(lumaupleft + lumadownleft - 2.0 * lumaleft);"
			""
			"	bool ishorizontal = edgehorizontal >= edgevertical;"
			""
			"	float luma1 = ishorizontal ? lumadown : lumaleft;"
			"	float luma2 = ishorizontal ? lumaup   : lumaright;"
			"	float gradient1 = luma1 - lumacenter;"
			"	float gradient2 = luma2 - lumacenter;"
			"	bool is1steepest = abs(gradient1) >= abs(gradient2);"
			""
			"	float gradientscaled = 0.25 * max(abs(gradient1), abs(gradient2));"
			""
			"	float stepsize = ishorizontal ? texelsize.y : texelsize.x;"
			"	float lumalocalavg = 0.0;"
			""
			"	if (is1steepest) {"
			"		stepsize = -stepsize;"
			"		lumalocalavg = 0.5 * (luma1 + lumacenter);"
			"	} else {"
			"		lumalocalavg = 0.5 * (luma2 + lumacenter);"
			"	}"
			""
			"	vec2 currentuv = TexCoords;"
			"	if (ishorizontal) {"
			"		currentuv.y += stepsize * 0.5;"
			"	} else {"
			"		currentuv.x += stepsize * 0.5;"
			"	}"
			""
			"	vec2 offset = ishorizontal ? vec2(texelsize.x, 0.0) : vec2(0.0, texelsize.y);"
			"	vec2 uv1 = currentuv - offset;"
			"	vec2 uv2 = currentuv + offset;"
			""
			"	float lumaend1 = luma(texture(screenTexture, uv1).rgb) - lumalocalavg;"
			"	float lumaend2 = luma(texture(screenTexture, uv2).rgb) - lumalocalavg;"
			"	bool reached1 = abs(lumaend1) >= gradientscaled;"
			"	bool reached2 = abs(lumaend2) >= gradientscaled;"
			"	bool reachedboth = reached1 && reached2;"
			""
			"	if (!reached1) uv1 -= offset;"
			"	if (!reached2) uv2 += offset;"
			""
			"	if (!reachedboth) {"
			"		float quality[12] = float[12](1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0, 8.0, 16.0, 32.0);"
			"		for (int i = 0; i < 12; i++) {"
			"			if (!reached1) {"
			"				lumaend1 = luma(texture(screenTexture, uv1).rgb) - lumalocalavg;"
			"				reached1 = abs(lumaend1) >= gradientscaled;"
			"				if (!reached1) uv1 -= offset * quality[i];"
			"			}"
			"			if (!reached2) {"
			"				lumaend2 = luma(texture(screenTexture, uv2).rgb) - lumalocalavg;"
			"				reached2 = abs(lumaend2) >= gradientscaled;"
			"				if (!reached2) uv2 += offset * quality[i];"
			"			}"
			"			if (reached1 && reached2) break;"
			"		}"
			"	}"
			""
			"	float distance1 = ishorizontal ? (TexCoords.x - uv1.x) : (TexCoords.y - uv1.y);"
			"	float distance2 = ishorizontal ? (uv2.x - TexCoords.x) : (uv2.y - TexCoords.y);"
			""
			"	bool isdirection1 = distance1 < distance2;"
			"	float distancefinal = min(distance1, distance2);"
			"	float edgethickness = distance1 + distance2;"
			"	float pixeloffset = -distancefinal / edgethickness + 0.5;"
			""
			"	bool iscentersmaller = lumacenter < lumalocalavg;"
			"	bool correctvariation = ((isdirection1 ? lumaend1 : lumaend2) < 0.0) != iscentersmaller;"
			"	float finaloffset = correctvariation ? pixeloffset : 0.0;"
			""
			"	float lumaaverage = (1.0 / 12.0) * (2.0 * (lumadownup + lumaleftright) + lumadownleft + lumadownright + lumaupleft + lumaupright);"
			"	float subpixeloffset1 = clamp(abs(lumaaverage - lumacenter) / lumarange, 0.0, 1.0);"
			"	float subpixeloffset2 = (-2.0 * subpixeloffset1 + 3.0) * subpixeloffset1 * subpixeloffset1;"
			"	float subpixeloffsetfinal = subpixeloffset2 * subpixeloffset2 * subpixelblend;"
			""
			"	finaloffset = max(finaloffset, subpixeloffsetfinal);"
			""
			"	vec2 finaluv = TexCoords;"
			"	if (ishorizontal) {"
			"		finaluv.y += finaloffset * stepsize;"
			"	} else {"
			"		finaluv.x += finaloffset * stepsize;"
			"	}"
			""
			"	FragColor = vec4(texture(screenTexture, finaluv).rgb, 1.0);"
			"}";
	return std::string(shadersource);
}
