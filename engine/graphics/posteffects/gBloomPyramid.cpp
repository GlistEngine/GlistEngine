/*
 * gBloomPyramid.cpp
 *
 *  Created on: 1 Agu 2026
 *      Author: Numan Affan Kahya
 */

#include "gBloomPyramid.h"
#include "gShader.h"

gBloomPyramid::gBloomPyramid(int screenwidth, int screenheight, int miplevels,
							  float intensity, float threshold, float knee,
							  float bloomsize) {
	this->miplevels = miplevels;
	this->intensity = intensity;
	this->threshold = threshold;
	this->knee = knee;
	this->bloomsize = bloomsize;
	this->isrgbmask = false;
	this->isalphamask = false;
	this->rgbmask[0] = this->rgbmask[1] = this->rgbmask[2] = 0.0f;
	this->colorsoftness = 0.0f;
	this->minglowbrightness = 0.0f;

	downfbo = nullptr;
	upfbo = nullptr;

	prefiltershader = new gShader();
	prefiltershader->loadProgram(getVertSrc(), getPrefilterFragSrc());

	downsampleshader = new gShader();
	downsampleshader->loadProgram(getVertSrc(), getDownsampleFragSrc());

	upsampleshader = new gShader();
	upsampleshader->loadProgram(getVertSrc(), getUpsampleFragSrc());

	shader = new gShader();
	shader->loadProgram(getVertSrc(), getCompositeFragSrc());

	allocateMips(screenwidth, screenheight);
}

gBloomPyramid::gBloomPyramid(int miplevels, float intensity, float threshold, float knee, float bloomsize)
	: gBloomPyramid(getScreenWidth(), getScreenHeight(), miplevels, intensity, threshold, knee, bloomsize) {
}

gBloomPyramid::gBloomPyramid(int screenwidth, int screenheight, int miplevels,
							  float intensity, float bloomsize, float rgb[3],
							  float colorSoftness, float minGlowBrightness)
	: gBloomPyramid(screenwidth, screenheight, miplevels, intensity, 1.0f, 0.2f, bloomsize) {
	isrgbmask = true;
	rgbmask[0] = rgb[0];
	rgbmask[1] = rgb[1];
	rgbmask[2] = rgb[2];
	colorsoftness = colorSoftness;
	minglowbrightness = minGlowBrightness;
}

gBloomPyramid::gBloomPyramid(float rgb[3], int miplevels, float intensity, float bloomsize, float colorSoftness, float minGlowBrightness)
	: gBloomPyramid(getScreenWidth(), getScreenHeight(), miplevels, intensity, bloomsize, rgb, colorSoftness, minGlowBrightness) {
}

gBloomPyramid::~gBloomPyramid() {
	freeMips();
	delete prefiltershader;
	delete downsampleshader;
	delete upsampleshader;
	delete shader;
}

void gBloomPyramid::mipSize(int level, int& outw, int& outh) const {
	outw = screenwidth >> level;
	outh = screenheight >> level;
	if (outw < 1) outw = 1;
	if (outh < 1) outh = 1;
}

void gBloomPyramid::allocateMips(int screenwidth, int screenheight) {
	this->screenwidth = screenwidth;
	this->screenheight = screenheight;

	downfbo = new gFbo();
	downfbo->allocateMipChain(screenwidth, screenheight, miplevels);

	upfbo = new gFbo();
	upfbo->allocateMipChain(screenwidth, screenheight, miplevels - 1);
}

void gBloomPyramid::freeMips() {
	delete downfbo;
	delete upfbo;
	downfbo = nullptr;
	upfbo = nullptr;
}

void gBloomPyramid::resize(int newscreenwidth, int newscreenheight) {
	if (newscreenwidth == screenwidth && newscreenheight == screenheight) return;
	freeMips();
	allocateMips(newscreenwidth, newscreenheight);
}

void gBloomPyramid::use() {
	shader->use();
}

void gBloomPyramid::render(gFbo& src, gFbo& dst) {
	int w, h;

	// 1. PREFILTER PASS
	downfbo->bindMip(0);
	prefiltershader->use();
	prefiltershader->setInt("screenTexture", 0);
	prefiltershader->setBool("isalphamask", isalphamask);
	prefiltershader->setFloat("threshold", threshold);
	prefiltershader->setFloat("knee", knee);
	prefiltershader->setBool("isrgbmask", isrgbmask);
	prefiltershader->setVec3("rgbmask", rgbmask[0], rgbmask[1], rgbmask[2]);
	prefiltershader->setFloat("colorsoftness", colorsoftness);
	prefiltershader->setFloat("minglowbrightness", minglowbrightness);

	renderer->bindQuadVAO();
	src.getTexture().bind(0);
	renderer->drawFullscreenQuad();

	// 2. DOWNSAMPLE PASS
	downsampleshader->use();
	downsampleshader->setInt("screenTexture", 0);
	for (int level = 1; level < miplevels; level++) {
		mipSize(level - 1, w, h);
		downsampleshader->setVec2("srctexelsize", 1.0f / (float)w, 1.0f / (float)h);
		downsampleshader->setInt("srclevel", level - 1);

		downfbo->bindMip(level);
		renderer->bindQuadVAO();
		downfbo->getTexture().bind(0);
		renderer->drawFullscreenQuad();
	}

	// 3. UPSAMPLE PASS
	upsampleshader->use();
	upsampleshader->setFloat("bloomsize", bloomsize);
	upsampleshader->setInt("texSmall", 0);
	upsampleshader->setInt("texSame", 1);

	for (int level = miplevels - 2; level >= 0; level--) {
		int smalllevel = level + 1;
		mipSize(smalllevel, w, h);
		upsampleshader->setVec2("smalltexelsize", 1.0f / (float)w, 1.0f / (float)h);
		upsampleshader->setInt("smalllevel", smalllevel);
		mipSize(level, w, h);
		upsampleshader->setInt("samelevel", level);

		upfbo->bindMip(level);
		renderer->bindQuadVAO();

		if (level == miplevels - 2) {
			downfbo->getTexture().bind(0);
		} else {
			upfbo->getTexture().bind(0);
		}
		downfbo->getTexture().bind(1);

		renderer->drawFullscreenQuad();
	}

	// 4. COMPOSITE PASS
	dst.bind();
	shader->use();
	shader->setFloat("intensity", intensity);
	shader->setInt("sceneTexture", 0);
	shader->setInt("bloomTexture", 1);

	renderer->bindQuadVAO();
	src.getTexture().bind(0);
	upfbo->getTexture().bind(1);

	renderer->drawFullscreenQuad();

	renderer->resetTexture();
}

void gBloomPyramid::setIntensity(float v) { intensity = v; }
void gBloomPyramid::setThreshold(float v) { threshold = v; }
void gBloomPyramid::setAlphaMask(bool enabled) { isalphamask = enabled; }
void gBloomPyramid::setKnee(float v) { knee = v; }
void gBloomPyramid::setBloomSize(float v) { bloomsize = v; }
void gBloomPyramid::setRgbMask(float r, float g, float b) {
	isrgbmask = true;
	rgbmask[0] = r; rgbmask[1] = g; rgbmask[2] = b;
}
void gBloomPyramid::disableRgbMask() { isrgbmask = false; }
void gBloomPyramid::disableAlphaMask() { isalphamask = false; }
void gBloomPyramid::setColorSoftness(float v) { colorsoftness = v; }
void gBloomPyramid::setMinGlowBrightness(float v) { minglowbrightness = v; }


const std::string gBloomPyramid::getVertSrc() {
	const char* src =
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
		"void main(){"
		"    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);"
		"    TexCoords = aTexCoords;"
		"}\n";
	return std::string(src);
}

const std::string gBloomPyramid::getPrefilterFragSrc() {
	const char* src =
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
		"    float threshold;\n"
		"    float knee;\n"
		"    bool isrgbmask;\n"
		"    bool isalphamask;\n"
		"    vec3 rgbmask;\n"
		"    float colorsoftness;\n"
		"    float minglowbrightness;\n"
		"};\n"
		"#endif\n"
		"#if GLES\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D screenTexture;\n"
		"uniform float threshold;\n"
		"uniform float knee;\n"
		"uniform bool isrgbmask;\n"
		"uniform bool isalphamask;\n"
		"uniform vec3 rgbmask;\n"
		"uniform float colorsoftness;\n"
		"uniform float minglowbrightness;\n"
		"#endif\n"
		"#if GLCORE\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D screenTexture;\n"
		"uniform float threshold;\n"
		"uniform float knee;\n"
		"uniform bool isrgbmask;\n"
		"uniform bool isalphamask;\n"
		"uniform vec3 rgbmask;\n"
		"uniform float colorsoftness;\n"
		"uniform float minglowbrightness;\n"
		"#endif\n"
		"void main(){"
		"    vec4 texcolor = texture(screenTexture, TexCoords);"
		"    vec3 result;"
		"    if(isalphamask) {"
		"        result = texcolor.rgb * texcolor.a;"
		"    } else if(isrgbmask) {"
		"        float colordist = length(texcolor.rgb - rgbmask);"
		"        float colorweight = 1.0 - smoothstep(0.0, max(colorsoftness, 0.0001), colordist);"
		"        float brightweight = step(minglowbrightness, dot(texcolor.rgb, vec3(0.2126, 0.7152, 0.0722)));"
		"        result = texcolor.rgb * colorweight * brightweight;"
		"    } else {"
		"        float brightness = max(texcolor.r, max(texcolor.g, texcolor.b));"
		"        float knee2 = threshold * knee + 1e-5;"
		"        float soft = brightness - threshold + knee2;"
		"        soft = clamp(soft, 0.0, 2.0 * knee2);"
		"        soft = soft * soft / (4.0 * knee2 + 1e-5);"
		"        float contribution = max(soft, brightness - threshold);"
		"        contribution /= max(brightness, 1e-5);"
		"        result = texcolor.rgb * contribution;"
		"    }"
		"    FragColor = vec4(result, 1.0);"
		"}";
	return std::string(src);
}

const std::string gBloomPyramid::getDownsampleFragSrc() {
	const char* src =
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
		"    vec2 srctexelsize;\n"
		"    int srclevel;\n"
		"};\n"
		"#endif\n"
		"#if GLES\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D screenTexture;\n"
		"uniform vec2 srctexelsize;\n"
		"uniform int srclevel;\n"
		"#endif\n"
		"#if GLCORE\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D screenTexture;\n"
		"uniform vec2 srctexelsize;\n"
		"uniform int srclevel;\n"
		"#endif\n"
		"vec3 samp(vec2 uv){ return textureLod(screenTexture, uv, float(srclevel)).rgb; }"
		"void main(){"
		"    vec2 uv = TexCoords;"
		"    vec2 o = srctexelsize;"
		"    vec3 a = samp(uv + vec2(-o.x, -o.y));"
		"    vec3 b = samp(uv + vec2( o.x, -o.y));"
		"    vec3 c = samp(uv + vec2(-o.x,  o.y));"
		"    vec3 d = samp(uv + vec2( o.x,  o.y));"
		"    vec3 e = samp(uv + vec2(-2.0*o.x, -2.0*o.y));"
		"    vec3 f = samp(uv + vec2( 0.0,     -2.0*o.y));"
		"    vec3 g = samp(uv + vec2( 2.0*o.x, -2.0*o.y));"
		"    vec3 h = samp(uv + vec2(-2.0*o.x,  0.0));"
		"    vec3 i = samp(uv);"
		"    vec3 j = samp(uv + vec2( 2.0*o.x,  0.0));"
		"    vec3 k = samp(uv + vec2(-2.0*o.x,  2.0*o.y));"
		"    vec3 l = samp(uv + vec2( 0.0,      2.0*o.y));"
		"    vec3 m = samp(uv + vec2( 2.0*o.x,  2.0*o.y));"
		"    vec3 result = (a+b+c+d) * 0.125;"
		"    result += (e+g+k+m) * 0.03125;"
		"    result += (f+h+j+l) * 0.0625;"
		"    result += i * 0.125;"
		"    FragColor = vec4(result, 1.0);"
		"}";
	return std::string(src);
}

const std::string gBloomPyramid::getUpsampleFragSrc() {
	const char* src =
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
		"layout (set = 0, binding = 0) uniform sampler2D texSmall;\n"
		"layout (set = 0, binding = 1) uniform sampler2D texSame;\n"
		"layout (set = 0, binding = 2) uniform Params {\n"
		"    vec2 smalltexelsize;\n"
		"    int smalllevel;\n"
		"    int samelevel;\n"
		"    float bloomsize;\n"
		"};\n"
		"#endif\n"
		"#if GLES\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D texSmall;\n"
		"uniform sampler2D texSame;\n"
		"uniform vec2 smalltexelsize;\n"
		"uniform int smalllevel;\n"
		"uniform int samelevel;\n"
		"uniform float bloomsize;\n"
		"#endif\n"
		"#if GLCORE\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D texSmall;\n"
		"uniform sampler2D texSame;\n"
		"uniform vec2 smalltexelsize;\n"
		"uniform int smalllevel;\n"
		"uniform int samelevel;\n"
		"uniform float bloomsize;\n"
		"#endif\n"
		"void main(){"
		"    float r = bloomsize;"
		"    vec2 o = smalltexelsize * r;"
		"    vec2 uv = TexCoords;"
		"    vec3 s = textureLod(texSmall, uv + vec2(-o.x,  o.y), float(smalllevel)).rgb * 1.0;"
		"    s += textureLod(texSmall, uv + vec2( 0.0,    o.y), float(smalllevel)).rgb * 2.0;"
		"    s += textureLod(texSmall, uv + vec2( o.x,    o.y), float(smalllevel)).rgb * 1.0;"
		"    s += textureLod(texSmall, uv + vec2(-o.x,    0.0), float(smalllevel)).rgb * 2.0;"
		"    s += textureLod(texSmall, uv,                      float(smalllevel)).rgb * 4.0;"
		"    s += textureLod(texSmall, uv + vec2( o.x,    0.0), float(smalllevel)).rgb * 2.0;"
		"    s += textureLod(texSmall, uv + vec2(-o.x, -o.y), float(smalllevel)).rgb * 1.0;"
		"    s += textureLod(texSmall, uv + vec2( 0.0, -o.y), float(smalllevel)).rgb * 2.0;"
		"    s += textureLod(texSmall, uv + vec2( o.x, -o.y), float(smalllevel)).rgb * 1.0;"
		"    s /= 16.0;"
		"    vec3 same = textureLod(texSame, uv, float(samelevel)).rgb;"
		"    FragColor = vec4(s + same, 1.0);"
		"}";
	return std::string(src);
}

const std::string gBloomPyramid::getCompositeFragSrc() {
	const char* src =
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
		"layout (set = 0, binding = 0) uniform sampler2D sceneTexture;\n"
		"layout (set = 0, binding = 1) uniform sampler2D bloomTexture;\n"
		"layout (set = 0, binding = 2) uniform Params {\n"
		"    float intensity;\n"
		"};\n"
		"#endif\n"
		"#if GLES\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D sceneTexture;\n"
		"uniform sampler2D bloomTexture;\n"
		"uniform float intensity;\n"
		"#endif\n"
		"#if GLCORE\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoords;\n"
		"uniform sampler2D sceneTexture;\n"
		"uniform sampler2D bloomTexture;\n"
		"uniform float intensity;\n"
		"#endif\n"
		"void main(){"
		"    vec3 scene = texture(sceneTexture, TexCoords).rgb;"
		"    vec3 bloom = texture(bloomTexture, TexCoords).rgb;"
		"    FragColor = vec4(scene + bloom * intensity, 1.0);"
		"}";
	return std::string(src);
}

const std::string gBloomPyramid::getFragSrc() {
	return getCompositeFragSrc();
}
