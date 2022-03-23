/*
 * gRenderManager.cpp
 *
 *  Created on: 4 Ara 2020
 *      Author: Acer
 */

#include "gRenderer.h"

#include "gLight.h"
#include "gLine.h"
#include "gCircle.h"
#include "gRectangle.h"
#include "gBox.h"
#include "gSphere.h"


const int gRenderer::SCREENSCALING_NONE = 0;
const int gRenderer::SCREENSCALING_MIPMAP = 1;
const int gRenderer::SCREENSCALING_AUTO = 2;

const int gRenderer::DEPTHTESTTYPE_LESS = 0;
const int gRenderer::DEPTHTESTTYPE_ALWAYS = 1;

int gRenderer::width;
int gRenderer::height;
int gRenderer::unitwidth;
int gRenderer::unitheight;
int gRenderer::screenscaling;
int gRenderer::currentresolution;
int gRenderer::unitresolution;


void gEnableCulling() {
	glEnable(GL_CULL_FACE);
}

void gDisableCulling() {
	glDisable(GL_CULL_FACE);
}

bool gIsCullingEnabled() {
	return glIsEnabled(GL_CULL_FACE);
}

void gCullFace(int cullingFace) {
	glCullFace(cullingFace);
}

int gGetCullFace() {
	GLint i;
	glGetIntegerv(GL_CULL_FACE_MODE, &i);
	return i;
}

void gSetCullingDirection(int cullingDirection) {
	glFrontFace(cullingDirection);
}

int gGetCullingDirection() {
	GLint i;
	glGetIntegerv(GL_FRONT_FACE, &i);
	return i;
}

void gDrawLine(float x1, float y1, float x2, float y2) {
	gLine linemesh;
	linemesh.draw(x1, y1, x2, y2);
	linemesh.clear();
}

void gDrawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
	gLine linemesh;
	linemesh.draw(x1, y1, z1, x2, y2, z2);
	linemesh.clear();
}

 void gDrawCircle(float xCenter, float yCenter, float radius, bool isFilled, float numberOfSides) {
	gCircle circlemesh;
	circlemesh.draw(xCenter, yCenter, radius, isFilled, numberOfSides);
	circlemesh.clear();
}

void gDrawArrow(float x1, float y1, float length, float angle, float tipLength, float tipAngle) {
	gLine linemesh;
	float x2, y2;
	x2 = x1 + std::cos(gDegToRad(angle)) * length;
	y2 = y1 + std::sin(gDegToRad(angle)) * length;;
	linemesh.draw(x2, y2, x1, y1);
	linemesh.draw(x1, y1, x1 + std::cos(gDegToRad(angle) - gDegToRad(tipAngle)) * tipLength, y1 + std::sin(gDegToRad(angle) - gDegToRad(tipAngle)) * tipLength);
	linemesh.draw(x1, y1, x1 + (std::cos(gDegToRad(angle) + gDegToRad(tipAngle)) * tipLength) , y1 + std::sin(gDegToRad(angle) + gDegToRad(tipAngle)) * tipLength);
	linemesh.clear();
}

void gDrawRectangle(float x, float y, float w, float h, bool isFilled) {
	gRectangle rectanglemesh;
 	rectanglemesh.draw(x, y, w, h, isFilled);
 	rectanglemesh.clear();
}

void gDrawRectangle(float x, float y, float w, float h, bool isFilled, float thickness, float borderposition) {
	  	if(borderposition == 0.0f) {
			for(int i = 0; i < thickness; i++) {
				gRectangle rectanglemesh;
			 	rectanglemesh.draw(x + i, y + i, w - i*2, h - i*2, isFilled);
				rectanglemesh.clear();
			}
		} else if (borderposition == 1.0f) {
			for(int i = 0; i < thickness; i++) {
				gRectangle rectanglemesh;
			 	rectanglemesh.draw(x - i, y - i, w + i*2, h + i*2, isFilled);
				rectanglemesh.clear();
			}
		}else if (borderposition > 0.0f && borderposition < 1.0f) {
			//calculating how many pixel is sided to each
			int inside = 0;
			int outside = 0;
			outside = (thickness * borderposition);
			inside = thickness - outside;
			//outside border
			for(int i = 0; i < outside; i++) {
				gRectangle rectanglemesh;
			 	rectanglemesh.draw(x - i, y - i, w + i*2, h + i*2, isFilled);
				rectanglemesh.clear();
			}
			//inside border
			for(int i = 0; i < inside; i++) {
				gRectangle rectanglemesh;
			 	rectanglemesh.draw(x + i, y + i, w - i*2, h - i*2, isFilled);
				rectanglemesh.clear();
			}
		}
}

void gDrawBox(float x, float y, float z, float w, float h, float d, bool isFilled) {
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setPosition(x, y, z);
	boxmesh.scale(w, h, d);
	boxmesh.draw();
}

void gDrawBox(glm::mat4 transformationMatrix, bool isFilled) {
	gBox boxmesh;
	if(!isFilled) boxmesh.setDrawMode(gMesh::DRAWMODE_LINELOOP);
	boxmesh.setTransformationMatrix(transformationMatrix);
	boxmesh.draw();
}

void gDrawSphere(float xPos, float yPos, float zPos, int xSegmentNum, int ySegmentNum, float scale, bool isFilled) {
	gSphere spheremesh(xSegmentNum, ySegmentNum, isFilled);
	spheremesh.setPosition(xPos , yPos, zPos);
	spheremesh.scale(scale);
	spheremesh.draw();
}

gRenderer::gRenderer() {
	width = gDefaultWidth();
	height = gDefaultHeight();
	unitwidth = gDefaultUnitWidth();
	unitheight = gDefaultUnitHeight();

	// TODO Check matrix maths
	projectionmatrix = glm::mat4(1.0f);
	projectionmatrix = glm::perspective(glm::radians(60.0f), (float)width / height, 0.0f, 1000.0f);
	projectionmatrixold = projectionmatrix;
	projectionmatrix2d = glm::ortho(0.0f, (float)unitwidth, (float)unitheight, 0.0f, -1.0f, 1.0f);
	viewmatrix = glm::mat4(1.0f);
	viewmatrixold = viewmatrix;
	cameraposition = glm::vec3(0.0f);

	colorshader = new gShader();
	colorshader->loadProgram(getShaderSrcColorVertex(), getShaderSrcColorFragment());

	textureshader = new gShader();
	textureshader->loadProgram(getShaderSrcTextureVertex(), getShaderSrcTextureFragment());
    textureshader->setMat4("projection", projectionmatrix);
    textureshader->setMat4("view", viewmatrix);

	imageshader = new gShader();
	imageshader->loadProgram(getShaderSrcImageVertex(), getShaderSrcImageFragment());
	imageshader->setMat4("projection", projectionmatrix2d);

	fontshader = new gShader();
	fontshader->loadProgram(getShaderSrcFontVertex(), getShaderSrcFontFragment());

	skyboxshader = new gShader();
	skyboxshader->loadProgram(getShaderSrcSkyboxVertex(), getShaderSrcSkyboxFragment());
	skyboxshader->setMat4("projection", projectionmatrix);
	skyboxshader->setMat4("view", viewmatrix);

	shadowmapshader = new gShader();
	shadowmapshader->loadProgram(getShaderSrcShadowmapVertex(), getShaderSrcShadowmapFragment());

	pbrshader = new gShader();
	pbrshader->loadProgram(getShaderSrcPbrVertex(), getShaderSrcPbrFragment());

	equirectangularshader = new gShader();
	equirectangularshader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcEquirectangularFragment());

	irradianceshader = new gShader();
	irradianceshader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcIrradianceFragment());

	prefiltershader = new gShader();
	prefiltershader->loadProgram(getShaderSrcCubemapVertex(), getShaderSrcPrefilterFragment());

	brdfshader = new gShader();
	brdfshader->loadProgram(getShaderSrcBrdfVertex(), getShaderSrcBrdfFragment());

	fboshader = new gShader();
	fboshader->loadProgram(getShaderSrcFboVertex(), getShaderSrcFboFragment());

	rendercolor = new gColor();
	rendercolor->set(255, 255, 255, 255);

	globalambientcolor = new gColor();
	globalambientcolor->set(255, 255, 255, 255);

	lightingcolor = new gColor();
	lightingcolor->set(255, 255, 255, 255);
	islightingenabled = false;
	lightingposition = glm::vec3(0.0f);
	li = 0;

	isfogenabled = false;
	fogcolor = new gColor();
	fogcolor->set(0.3f, 0.3f, 0.3f);
	fogdensity = 0.15;
	foggradient = 1.5f;

	isdepthtestenabled = false;
	depthtesttype = 0;
	depthtesttypeid[0] = GL_LESS;
	depthtesttypeid[1] = GL_ALWAYS;

	isalphablendingenabled = false;
	isalphatestenabled = false;
}

gRenderer::~gRenderer() {
}

gShader* gRenderer::getColorShader() {
	return colorshader;
}

gShader* gRenderer::getTextureShader() {
	return textureshader;
}

gShader* gRenderer::getFontShader() {
	return fontshader;
}

gShader* gRenderer::getImageShader() {
	return imageshader;
}

gShader* gRenderer::getSkyboxShader() {
	return skyboxshader;
}

gShader* gRenderer::getShadowmapShader() {
	return shadowmapshader;
}

gShader* gRenderer::getPbrShader() {
	return pbrshader;
}

gShader* gRenderer::getEquirectangularShader() {
	return equirectangularshader;
}

gShader* gRenderer::getIrradianceShader() {
	return irradianceshader;
}

gShader* gRenderer::getPrefilterShader() {
	return prefiltershader;
}

gShader* gRenderer::getBrdfShader() {
	return brdfshader;
}

gShader* gRenderer::getFboShader() {
	return fboshader;
}

void gRenderer::setProjectionMatrix(glm::mat4 projectionMatrix) {
	projectionmatrix = projectionMatrix;
}

void gRenderer::setProjectionMatrix2d(glm::mat4 projectionMatrix2d) {
	projectionmatrix2d = projectionMatrix2d;
}

void gRenderer::setViewMatrix(glm::mat4 viewMatrix) {
	viewmatrix = viewMatrix;
}

void gRenderer::setCameraPosition(glm::vec3 cameraPosition) {
	cameraposition = cameraPosition;
}

const glm::mat4& gRenderer::getProjectionMatrix() const {
	return projectionmatrix;
}

const glm::mat4& gRenderer::getProjectionMatrix2d() const {
	return projectionmatrix2d;
}

const glm::mat4& gRenderer::getViewMatrix() const {
	return viewmatrix;
}

const glm::vec3& gRenderer::getCameraPosition() const {
	return cameraposition;
}

void gRenderer::backupMatrices() {
	projectionmatrixold = projectionmatrix;
	viewmatrixold = viewmatrix;
}

void gRenderer::restoreMatrices() {
	projectionmatrix = projectionmatrixold;
	viewmatrix = viewmatrixold;
}

void gRenderer::setScreenSize(int screenWidth, int screenHeight) {
	width = screenWidth;
	height = screenHeight;
	setCurrentResolution(getResolution(screenWidth, screenHeight));
}

void gRenderer::setUnitScreenSize(int unitWidth, int unitHeight) {
	unitwidth = unitWidth;
	unitheight = unitHeight;
	setUnitResolution(getResolution(unitWidth, unitHeight));
}

void gRenderer::setScreenScaling(int screenScaling) {
	screenscaling = screenScaling;
	gObject::setCurrentResolution(screenscaling, currentresolution);
}

int gRenderer::getWidth() {
	if (screenscaling >= SCREENSCALING_AUTO) return unitwidth;
	return width;
}

int gRenderer::getHeight() {
	if (screenscaling >= SCREENSCALING_AUTO) return unitheight;
	return height;
}

int gRenderer::getScreenWidth() {
	return width;
}

int gRenderer::getScreenHeight() {
	return height;
}

int gRenderer::getUnitWidth() {
	return unitwidth;
}

int gRenderer::getUnitHeight() {
	return unitheight;
}

int gRenderer::getScreenScaling() {
	return screenscaling;
}

void gRenderer::setCurrentResolution(int resolution) {
	currentresolution = resolution;
}

void gRenderer::setCurrentResolution(int screenWidth, int screenHeight) {
	setCurrentResolution(getResolution(screenWidth, screenHeight));
	gObject::setCurrentResolution(screenscaling, currentresolution);
}

void gRenderer::setUnitResolution(int resolution) {
	unitresolution = resolution;
}

void gRenderer::setUnitResolution(int screenWidth, int screenHeight) {
	setUnitResolution(getResolution(screenWidth, screenHeight));
}

int gRenderer::getResolution(int screenWidth, int screenHeight) {
	int resolutions[8][2] = {
			{7680, 4320}, //8k
			{3840, 2160}, //4k
			{2560, 1440}, //qhd
			{1920, 1080}, //fullhd
			{1280, 720},  //hd
			{960, 540},   //qfhd
			{800, 480},   //wvga
			{480, 320}    //hvga
	};

	int res = 0;
	for(int i = 0; i < 8; i++) {
		if (screenWidth >= resolutions[i][0] * 0.9f && screenHeight >= resolutions[i][1] * 0.9f) {
			res =i;
			break;
		}
	}

	return res;
}

int gRenderer::getCurrentResolution() {
	return currentresolution;
}

int gRenderer::getUnitResolution() {
	return unitresolution;
}

int gRenderer::scaleX(int x) {
	return (x * unitwidth) / width;
}

int gRenderer::scaleY(int y) {
	return (y * unitheight) / height;
}




void gRenderer::setColor(int r, int g, int b, int a) {
	rendercolor->set((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}

void gRenderer::setColor(float r, float g, float b, float a) {
	rendercolor->set(r, g, b, a);
}

void gRenderer::setColor(gColor color) {
	rendercolor->set(color.r, color.g, color.b, color.a);
}

void gRenderer::setColor(gColor* color) {
	rendercolor->set(color->r, color->g, color->b, color->a);
}

gColor* gRenderer::getColor() {
	return rendercolor;
}

void gRenderer::clear() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gRenderer::clearColor(int r, int g, int b, int a) {
	glClearColor((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gRenderer::clearColor(gColor color) {
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gRenderer::enableFog() {
	isfogenabled = true;
}

void gRenderer::disableFog() {
    isfogenabled = false;
}

bool gRenderer::isFogEnabled() {
	return isfogenabled;
}

void gRenderer::setFogColor(float r, float g, float b) {
	fogcolor->set(r, g, b);
}

void gRenderer::enableLighting() {
	lightingcolor->set(0.0f, 0.0f, 0.0f, 1.0f);
	islightingenabled = true;
}

void gRenderer::disableLighting() {
	globalambientcolor = new gColor();
	globalambientcolor->set(255, 255, 255, 255);
	lightingcolor->set(globalambientcolor->r, globalambientcolor->g, globalambientcolor->b, globalambientcolor->a);
	lightingposition = glm::vec3(0.0f);
	removeAllSceneLights();
	islightingenabled = false;
}

bool gRenderer::isLightingEnabled() {
	return islightingenabled;
}

void gRenderer::setLightingColor(int r, int g, int b, int a) {
	lightingcolor->set(r, g, b, a);
}

void gRenderer::setLightingColor(gColor* color) {
	lightingcolor->set(color);
}

gColor* gRenderer::getLightingColor() {
	return lightingcolor;
}

void gRenderer::setLightingPosition(glm::vec3 lightingPosition) {
	lightingposition = lightingPosition;
}

glm::vec3 gRenderer::getLightingPosition() {
	return lightingposition;
}

void gRenderer::setGlobalAmbientColor(int r, int g, int b, int a) {
	globalambientcolor->set(r, g, b, a);
}

void gRenderer::setGlobalAmbientColor(gColor color) {
	globalambientcolor->set(color.r, color.g, color.b, color.a);
}

gColor* gRenderer::getGlobalAmbientColor() {
	return globalambientcolor;
}

void gRenderer::addSceneLight(gLight* light) {
	scenelights.push_back(light);
}

gLight* gRenderer::getSceneLight(int lightNo) {
	return scenelights[lightNo];
}

int gRenderer::getSceneLightNum() {
	return scenelights.size();
}

void gRenderer::removeSceneLight(gLight* light) {
	for (li = 0; li < scenelights.size(); li++) {
		if (light == scenelights[li]) {
			scenelights.erase(scenelights.begin() + li);
			break;
		}
	}
}

void gRenderer::removeAllSceneLights() {
	scenelights.clear();
}

void gRenderer::enableDepthTest() {
	enableDepthTest(DEPTHTESTTYPE_LESS);
}

void gRenderer::enableDepthTest(int depthTestType) {
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(depthtesttypeid[depthTestType]);
	isdepthtestenabled = true;
	depthtesttype = depthTestType;
}

void gRenderer::setDepthTestFunc(int depthTestType) {
	glDepthFunc(depthtesttypeid[depthTestType]);
	depthtesttype = depthTestType;
}

void gRenderer::disableDepthTest() {
	glDisable(GL_DEPTH_TEST);
	isdepthtestenabled = false;
}

bool gRenderer::isDepthTestEnabled() {
	return isdepthtestenabled;
}

int gRenderer::getDepthTestType() {
	return depthtesttype;
}

void gRenderer::enableAlphaBlending() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    isalphablendingenabled = true;
}

void gRenderer::disableAlphaBlending() {
    glDisable(GL_BLEND);
    isalphablendingenabled = false;
}

bool gRenderer::isAlphaBlendingEnabled() {
	return isalphablendingenabled;
}

void gRenderer::enableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1);
    isalphatestenabled = true;
#endif
}

void gRenderer::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
    glDisable(GL_ALPHA_TEST);
    isalphatestenabled = false;
#endif
}

bool gRenderer::isAlphaTestEnabled() {
	return isalphatestenabled;
}

const std::string gRenderer::getShaderSrcColorVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
	layout (location = 1) in vec3 aNormal;
	layout (location = 2) in vec2 aTexCoords;
	layout (location = 3) in vec3 aTangent;
	layout (location = 4) in vec3 aBitangent;
	layout (location = 5) in int aUseNormalMap;
	uniform int aUseShadowMap;
	uniform int aUseFog;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec3 lightPos;
	uniform vec3 viewPos;
	uniform mat4 lightMatrix;
	out float visibility;
	uniform float fogdensity;
	uniform float foggradient;

	flat out int mUseNormalMap;
	flat out int mUseShadowMap;
	flat out int mUseFog;

	out vec3 Normal;
	out vec3 FragPos;
	out vec2 TexCoords;
	out vec4 FragPosLightSpace;
	out vec3 TangentLightPos;
	out vec3 TangentViewPos;
	out vec3 TangentFragPos;

	void main() {
		mUseShadowMap = aUseShadowMap;
		mUseFog = aUseFog;
		FragPos = vec3(model * vec4(aPos, 1.0));
		Normal = mat3(transpose(inverse(model))) * aNormal;
		TexCoords = aTexCoords;
		FragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);
		mUseNormalMap = aUseNormalMap;

	    if (aUseNormalMap > 0) {
		    mat3 normalMatrix = transpose(inverse(mat3(model)));
		    vec3 T = normalize(normalMatrix * aTangent);
		    vec3 N = normalize(normalMatrix * aNormal);
		    T = normalize(T - dot(T, N) * N);
		    vec3 B = cross(N, T);

		    mat3 TBN = transpose(mat3(T, B, N));
		    TangentLightPos = TBN * lightPos;
		    TangentViewPos  = TBN * viewPos;
		    TangentFragPos  = TBN * FragPos;
	    }

	    gl_Position = projection * view * model * vec4(aPos, 1.0);

		if (aUseFog > 0) {
			float distance = length(gl_Position.xyz);
			visibility = exp(-pow((distance * fogdensity), foggradient));
			visibility = clamp(visibility, 0.0, 1.0);
		}
	}
)";
	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcColorFragment() {
	const char* shadersource = R"(
	#version 330 core

	struct Material {
	    vec4 ambient;
	    vec4 diffuse;
	    vec4 specular;
	    float shininess;
	    sampler2D diffusemap;
	    sampler2D specularmap;
	    sampler2D normalMap;
		int useDiffuseMap;
		int useSpecularMap;
	};

	struct Light {
		int type; //0-ambient, 1-directional, 2-point, 3-spot
	    vec3 position;
	    vec3 direction;
	    float cutOff;
	    float outerCutOff;

	    vec4 ambient;
	    vec4 diffuse;
	    vec4 specular;

	    float constant;
	    float linear;
	    float quadratic;
	};

	uniform Material material;
	#define MAX_LIGHTS 8
	uniform Light lights[MAX_LIGHTS];

	uniform vec4 renderColor;
	uniform vec3 viewPos;
	uniform vec3 fogColor;

	in vec3 Normal;
	in vec3 FragPos;
	in vec2 TexCoords;
	in vec4 FragPosLightSpace;
	flat in int mUseNormalMap;
	in vec3 TangentLightPos;
	in vec3 TangentViewPos;
	in vec3 TangentFragPos;
	in float visibility;

	flat in int mUseShadowMap;
	flat in int mUseFog;
	uniform sampler2D shadowMap;
	uniform vec3 shadowLightPos;

	out vec4 FragColor;

	float calculateShadow(vec4 fragPosLightSpace) {
		vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		projCoords = projCoords * 0.5 + 0.5;
		float closestDepth = texture(shadowMap, projCoords.xy).r;
		float currentDepth = projCoords.z;
		vec3 normal = normalize(Normal);
		vec3 lightDir = normalize(shadowLightPos - FragPos);
		float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
		float shadow = 0.0;
		vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
		for(int x = -1; x <= 1; ++x) {
			for(int y = -1; y <= 1; ++y) {
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
				shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
			}
		}
		shadow /= 9.0;
		if(projCoords.z > 1.0) shadow = 0.0;

		return shadow;
	}

	vec4 calcAmbLight(Light light, vec4 materialAmbient) {
		vec4 ambient = light.ambient * materialAmbient;
		return ambient;
	}

	vec4 calcDirLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular) {
		vec3 lightDir = normalize(-light.direction);
		float diff;
		float spec;
		if (mUseNormalMap > 0) {
			diff = max(dot(lightDir, normal), 0.0);
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		} else {
			diff = max(dot(normal, lightDir), 0.0);
			vec3 reflectDir = reflect(-lightDir, normal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		}
		vec4 ambient = light.ambient * materialAmbient;
		vec4 diffuse = light.diffuse * diff * materialDiffuse;
		vec4 specular = light.specular * spec * materialSpecular;
		if (mUseShadowMap > 0) {
			diffuse *= shadowing;
			specular *= shadowing;
		}
		return (ambient + diffuse + specular);
	}

	vec4 calcPointLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){
		vec3 lightDir;
		float distance;
		if (mUseNormalMap > 0) {
			lightDir = normalize(TangentLightPos - TangentFragPos);
			distance = length(TangentLightPos - TangentFragPos);
		} else {
			lightDir = normalize(light.position - FragPos);
			distance = length(light.position - FragPos);
		}
		float diff;
		float spec;
		if (mUseNormalMap > 0) {
			diff = max(dot(lightDir, normal), 0.0);
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		} else {
			diff = max(dot(normal, lightDir), 0.0);
			vec3 reflectDir = reflect(-lightDir, normal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		}
		vec4 ambient = light.ambient * materialAmbient;
		vec4 diffuse = light.diffuse * diff * materialDiffuse;
		vec4 specular = light.specular * spec * materialSpecular;
		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
		ambient  *= attenuation;
		diffuse  *= attenuation;
		specular *= attenuation;
		if (mUseShadowMap > 0) {
			diffuse  *= shadowing;
			specular *= shadowing;
		}
		return (ambient + diffuse + specular);
	}

	vec4 calcSpotLight(Light light, vec3 normal, vec3 viewDir, float shadowing, vec4 materialAmbient, vec4 materialDiffuse, vec4 materialSpecular){
		vec3 lightDir;
		float distance;
		if (mUseNormalMap > 0) {
			lightDir = normalize(TangentLightPos - TangentFragPos);
			distance = length(TangentLightPos - TangentFragPos);
		} else {
			lightDir = normalize(light.position - FragPos);
			distance = length(light.position - FragPos);
		}
		float diff;
		float spec;
		if (mUseNormalMap > 0) {
			diff = max(dot(lightDir, normal), 0.0);
			vec3 halfwayDir = normalize(lightDir + viewDir);
			spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
		} else {
			diff = max(dot(normal, lightDir), 0.0);
			vec3 reflectDir = reflect(-lightDir, normal);
			spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		}
		vec4 ambient  = light.ambient  * materialAmbient;
		vec4 diffuse  = light.diffuse  * diff * materialDiffuse;
		vec4 specular = light.specular * spec * materialSpecular;

		float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

		float theta = dot(lightDir, normalize(-light.direction));
		float epsilon = (light.cutOff - light.outerCutOff);
		float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
		ambient  *= attenuation;
		diffuse  *= attenuation * intensity;
		specular *= attenuation * intensity;
		if (mUseShadowMap > 0) {
			diffuse *= shadowing;
			specular *= shadowing;
		}
		return (ambient + diffuse + specular);
	}

	void main() {
		vec4 result;
		vec3 norm;
		if (mUseNormalMap > 0) {
		    norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space
		} else {
			norm = normalize(Normal);
		}
		vec3 viewDir;
		if (mUseNormalMap > 0) {
			viewDir = normalize(TangentViewPos - TangentFragPos);
		}
		else {
			viewDir = normalize(viewPos - FragPos);
		}
		vec4 materialAmbient;
		vec4 materialDiffuse;
		if (material.useDiffuseMap > 0) {
			materialAmbient = texture(material.diffusemap, TexCoords).rgba;
			materialDiffuse = texture(material.diffusemap, TexCoords).rgba;
		} else {
			materialAmbient = material.ambient;
			materialDiffuse = material.diffuse;
		}
		vec4 materialSpecular;
		if (material.useSpecularMap > 0) {
			materialSpecular = texture(material.specularmap, TexCoords).rgba;
		}
		else {
			materialSpecular = material.specular;
		}
		float shadowing;
		if (mUseShadowMap > 0) {
			shadowing = 1.0 - calculateShadow(FragPosLightSpace);
		}
		for (int i = 0; i < MAX_LIGHTS; i++) {
			if (lights[i].type == 0) {
				result += calcAmbLight(lights[i], materialAmbient);
			}
			else if (lights[i].type == 1) {
				result += calcDirLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
			}
			else if (lights[i].type == 2) {
				result += calcPointLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
			}
			else if (lights[i].type == 3) {
				result += calcSpotLight(lights[i], norm, viewDir, shadowing, materialAmbient, materialDiffuse, materialSpecular);
			}
		}

		FragColor = result * renderColor;

		if (mUseFog > 0) {
			FragColor = mix(vec4(fogColor, 1.0), FragColor, visibility);
		}
	}
)";
	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aNormal;
	layout (location = 2) in vec2 aTexCoords;
	
	out vec2 TexCoords;
	
	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	
	void main() {
		TexCoords = aTexCoords;
		gl_Position = projection * view * model * vec4(aPos, 1.0);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureFragment() {
	const char* shadersource = R"(
	#version 330 core
	out vec4 FragColor;
	
	in vec2 TexCoords;
	
	uniform sampler2D texture_diffuse1;
	
	void main()
	{
		FragColor = texture(texture_diffuse1, TexCoords);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
	
	out vec2 TexCoords;
	
	uniform mat4 model;
	uniform mat4 projection;
	
	void main()
	{
		TexCoords = vertex.zw;
		gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageFragment() {
	const char* shadersource = R"(
	#version 330 core
	in vec2 TexCoords;
	out vec4 color;
	
	uniform sampler2D image;
	uniform vec4 spriteColor;
	
	void main()
	{
		color = spriteColor * texture(image, TexCoords);
	}
)";


	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
	out vec2 TexCoords;
	
	uniform mat4 projection;
	
	void main() {
		gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
		TexCoords = vertex.zw;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontFragment() {
	const char* shadersource = R"(
	#version 330 core
	in vec2 TexCoords;
	out vec4 color;
	
	uniform sampler2D text;
	uniform vec3 textColor;
	
	void main() {
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
		color = vec4(textColor, 1.0) * sampled;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	uniform int aIsHDR;
	
	out vec3 TexCoords;
	flat out int mIsHDR;
	
	uniform mat4 model;
	uniform mat4 projection;
	uniform mat4 view;
	
	void main()
	{
		mIsHDR = aIsHDR;
		TexCoords = aPos;
		vec4 pos = projection * view * model * vec4(aPos, 1.0);
		gl_Position = pos.xyww;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxFragment() {
	const char* shadersource = R"(
	#version 330 core
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
//			fc = vec4(0.0, 1.0, 0.0, 1.0);
		} else {
			fc = vec4(1.0, 0.0, 0.0, 1.0);
		}
		FragColor = fc;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	
	uniform mat4 lightMatrix;
	uniform mat4 model;
	
	void main()
	{
		gl_Position = lightMatrix * model * vec4(aPos, 1.0);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapFragment() {
	const char* shadersource = R"(
	#version 330 core
	
	void main() {
		// gl_FragDepth = gl_FragCoord.z;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPbrVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec3 aNormal;
	layout (location = 2) in vec2 aTexCoords;
	
	out vec2 TexCoords;
	out vec3 WorldPos;
	out vec3 Normal;
	
	uniform mat4 projection;
	uniform mat4 view;
	uniform mat4 model;
	
	void main()
	{
		TexCoords = aTexCoords;
		WorldPos = vec3(model * vec4(aPos, 1.0));
		Normal = mat3(model) * aNormal;
	
		gl_Position =  projection * view * vec4(WorldPos, 1.0);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPbrFragment() {
	const char* shadersource = R"(
	#version 330 core
	out vec4 FragColor;
	in vec2 TexCoords;
	in vec3 WorldPos;
	in vec3 Normal;
	
	// material parameters
	uniform sampler2D albedoMap;
	uniform sampler2D normalMap;
	uniform sampler2D metallicMap;
	uniform sampler2D roughnessMap;
	uniform sampler2D aoMap;
	
	// IBL
	uniform samplerCube irradianceMap;
	uniform samplerCube prefilterMap;
	uniform sampler2D brdfLUT;
	
	// lights
	uniform int lightNum;
	uniform vec3 lightPositions[8];
	uniform vec3 lightColors[8];
	
	uniform vec3 camPos;
	
	const float PI = 3.14159265359;
	// ----------------------------------------------------------------------------
	// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
	// Don't worry if you don't get what's going on; you generally want to do normal
	// mapping the usual way for performance anways; I do plan make a note of this
	// technique somewhere later in the normal mapping tutorial.
	vec3 getNormalFromMap()
	{
		vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
	
		vec3 Q1  = dFdx(WorldPos);
		vec3 Q2  = dFdy(WorldPos);
		vec2 st1 = dFdx(TexCoords);
		vec2 st2 = dFdy(TexCoords);
	
		vec3 N   = normalize(Normal);
		vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
		vec3 B  = -normalize(cross(N, T));
		mat3 TBN = mat3(T, B, N);
	
		return normalize(TBN * tangentNormal);
	}
	// ----------------------------------------------------------------------------
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness*roughness;
		float a2 = a*a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;
	
		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;
	
		return nom / denom;
	}
	// ----------------------------------------------------------------------------
	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		float r = (roughness + 1.0);
		float k = (r*r) / 8.0;
	
		float nom   = NdotV;
		float denom = NdotV * (1.0 - k) + k;
	
		return nom / denom;
	}
	// ----------------------------------------------------------------------------
	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
		return ggx1 * ggx2;
	}
	// ----------------------------------------------------------------------------
	vec3 fresnelSchlick(float cosTheta, vec3 F0)
	{
		return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
	}
	// ----------------------------------------------------------------------------
	vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
	{
		return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
	}
	// ----------------------------------------------------------------------------
	void main()
	{
		// material properties
		vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
		float metallic = texture(metallicMap, TexCoords).r;
		float roughness = texture(roughnessMap, TexCoords).r;
		float ao = texture(aoMap, TexCoords).r;
	
		// input lighting data
		vec3 N = getNormalFromMap();
		vec3 V = normalize(camPos - WorldPos);
		vec3 R = reflect(-V, N);
	
		// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
		// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
		vec3 F0 = vec3(0.04);
		F0 = mix(F0, albedo, metallic);
	
		// reflectance equation
		vec3 Lo = vec3(0.0);
		for(int i = 0; i < lightNum; ++i) {
			// calculate per-light radiance
			vec3 L = normalize(lightPositions[i] - WorldPos);
			vec3 H = normalize(V + L);
			float distance = length(lightPositions[i] - WorldPos);
			float attenuation = 1.0 / (distance * distance);
			vec3 radiance = lightColors[i] * attenuation;
	
			// Cook-Torrance BRDF
			float NDF = DistributionGGX(N, H, roughness);
			float G   = GeometrySmith(N, V, L, roughness);
			vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);
	
			vec3 nominator    = NDF * G * F;
			float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
			vec3 specular = nominator / denominator;
	
			 // kS is equal to Fresnel
			vec3 kS = F;
			// for energy conservation, the diffuse and specular light can't
			// be above 1.0 (unless the surface emits light); to preserve this
			// relationship the diffuse component (kD) should equal 1.0 - kS.
			vec3 kD = vec3(1.0) - kS;
			// multiply kD by the inverse metalness such that only non-metals
			// have diffuse lighting, or a linear blend if partly metal (pure metals
			// have no diffuse light).
			kD *= 1.0 - metallic;
	
			// scale light by NdotL
			float NdotL = max(dot(N, L), 0.0);
	
			// add to outgoing radiance Lo
			Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
		}
	
		// ambient lighting (we now use IBL as the ambient term)
		vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	
		vec3 kS = F;
		vec3 kD = 1.0 - kS;
		kD *= 1.0 - metallic;
	
		vec3 irradiance = texture(irradianceMap, N).rgb;
		vec3 diffuse      = irradiance * albedo;
	
		// sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
		const float MAX_REFLECTION_LOD = 4.0;
		vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;
		vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
		vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
	
		vec3 ambient = (kD * diffuse + specular) * ao;
	
		vec3 color = ambient + Lo;
	
		// HDR tonemapping
		color = color / (color + vec3(1.0));
		// gamma correct
		color = pow(color, vec3(1.0/2.2));
	
		FragColor = vec4(color , 1.0);
	}
)";


	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcCubemapVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec3 aPos;
	
	out vec3 WorldPos;
	
	uniform mat4 projection;
	uniform mat4 view;
	
	void main()
	{
		WorldPos = aPos;
		gl_Position =  projection * view * vec4(WorldPos, 1.0);
	}
)";
		return std::string(shadersource);
	}
	
const std::string gRenderer::getShaderSrcEquirectangularFragment() {
		const char* shadersource = R"(
	#version 330 core
	out vec4 FragColor;
	in vec3 WorldPos;
	
	uniform sampler2D equirectangularMap;
	
	const vec2 invAtan = vec2(0.1591, 0.3183);
	
	vec2 SampleSphericalMap(vec3 v) {
		vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
		uv *= invAtan;
		uv += 0.5;
		return uv;
	}
	
	void main() {
		vec2 uv = SampleSphericalMap(normalize(WorldPos));
		vec3 color = texture(equirectangularMap, uv).rgb;
	
		FragColor = vec4(color, 1.0);
	}
)";


	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcIrradianceFragment() {
	const char* shadersource = R"(
	#version 330 core
	
	in vec3 WorldPos;
	out vec4 FragColor;
	uniform samplerCube environmentMap;
	
	const float PI = 3.14159265359;
	
	// Mirror binary digits about the decimal point
	float radicalInverse_VdC(int bits)
	{
		bits = (bits << 16) | (bits >> 16);
		bits = ((bits & 0x55555555) << 1) | ((bits & 0xAAAAAAAA) >> 1);
		bits = ((bits & 0x33333333) << 2) | ((bits & 0xCCCCCCCC) >> 2);
		bits = ((bits & 0x0F0F0F0F) << 4) | ((bits & 0xF0F0F0F0) >> 4);
		bits = ((bits & 0x00FF00FF) << 8) | ((bits & 0xFF00FF00) >> 8);
		return float(bits) * 2.3283064365386963e-10; // / 0x100000000
	}
	
	// Randomish sequence that has pretty evenly spaced points
	// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	vec2 hammersley(int i, int N)
	{
		return vec2(float(i)/float(N), radicalInverse_VdC(i));
	}
	
	vec3 importanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness * roughness;
	
		float phi = 2.0 * PI * Xi.x;
		float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
		float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
		// from spherical coordinates to cartesian coordinates
		vec3 H;
		H.x = cos(phi) * sinTheta;
		H.y = sin(phi) * sinTheta;
		H.z = cosTheta;
	
		// from tangent-space vector to world-space sample vector
		vec3 up = abs(N.z) < 0.999 ? vec3 (0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 tangent = normalize(cross(up, N));
		vec3 bitangent = cross(N, tangent);
	
		vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
		return normalize(sampleVec);
	}
	
	void main()
	{
		// Not the normal of the cube, but the normal that we're calculating the irradiance for
		vec3 normal = normalize(WorldPos);
		vec3 N = normal;
	
		vec3 irradiance = vec3(0.0);
	
		vec3 up = vec3(0.0, 1.0, 0.0);
		vec3 right = cross(up, normal);
		up = cross(normal, right);
	
		// ------------------------------------------------------------------------------
	
		const int SAMPLE_COUNT = 16384;
		float totalWeight = 0.0;
		for (int i = 0; i < SAMPLE_COUNT; ++i) {
			vec2 Xi = hammersley(i, SAMPLE_COUNT);
			vec3 H = importanceSampleGGX(Xi, N, 1.0);
	
			// NdotH is equal to cos(theta)
			float NdotH = max(dot(N, H), 0.0);
			// With roughness == 1 in the distribution function we get 1/pi
			float D = 1.0 / PI;
			float pdf = (D * NdotH / (4.0)) + 0.0001;
	
			float resolution = 1024.0; // resolution of source cubemap (per face)
			// with a higher resolution, we should sample coarser mipmap levels
			float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
			// as we take more samples, we can sample from a finer mipmap.
			// And places where H is more likely to be sampled (higher pdf) we
			// can use a finer mipmap, otherwise use courser mipmap.
			// The tutorial treats this portion as optional to reduce noise but I think it's
			// actually necessary for importance sampling to get the correct result
			float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
	
			float mipLevel = 0.5 * log2(saSample / saTexel);
	
			irradiance += textureLod(environmentMap, H, mipLevel).rgb * NdotH;
			// irradiance += texture(environmentMap, H).rgb * NdotH;
			totalWeight += NdotH;
		}
		irradiance = (PI * irradiance) / totalWeight;
	
		// irradiance = texture(environmentMap, normal).rgb;
		FragColor = vec4(irradiance, 1.0);
	}
)";


	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcPrefilterFragment() {
	const char* shadersource = R"(
	#version 330 core
	out vec4 FragColor;
	in vec3 WorldPos;
	
	uniform samplerCube environmentMap;
	uniform float roughness;
	
	const float PI = 3.14159265359;
	// ----------------------------------------------------------------------------
	float DistributionGGX(vec3 N, vec3 H, float roughness)
	{
		float a = roughness*roughness;
		float a2 = a*a;
		float NdotH = max(dot(N, H), 0.0);
		float NdotH2 = NdotH*NdotH;
	
		float nom   = a2;
		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = PI * denom * denom;
	
		return nom / denom;
	}
	// ----------------------------------------------------------------------------
	// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	// efficient VanDerCorpus calculation.
	float RadicalInverse_VdC(uint bits)
	{
		 bits = (bits << 16u) | (bits >> 16u);
		 bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		 bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		 bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		 bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
		 return float(bits) * 2.3283064365386963e-10; // / 0x100000000
	}
	// ----------------------------------------------------------------------------
	vec2 Hammersley(uint i, uint N)
	{
		return vec2(float(i)/float(N), RadicalInverse_VdC(i));
	}
	// ----------------------------------------------------------------------------
	vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness*roughness;
	
		float phi = 2.0 * PI * Xi.x;
		float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
		float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
		// from spherical coordinates to cartesian coordinates - halfway vector
		vec3 H;
		H.x = cos(phi) * sinTheta;
		H.y = sin(phi) * sinTheta;
		H.z = cosTheta;
	
		// from tangent-space H vector to world-space sample vector
		vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 tangent   = normalize(cross(up, N));
		vec3 bitangent = cross(N, tangent);
	
		vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
		return normalize(sampleVec);
	}
	// ----------------------------------------------------------------------------
	void main()
	{
		vec3 N = normalize(WorldPos);
	
		// make the simplyfying assumption that V equals R equals the normal
		vec3 R = N;
		vec3 V = R;
	
		const uint SAMPLE_COUNT = 1024u;
		vec3 prefilteredColor = vec3(0.0);
		float totalWeight = 0.0;
	
		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			// generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H = ImportanceSampleGGX(Xi, N, roughness);
			vec3 L  = normalize(2.0 * dot(V, H) * H - V);
	
			float NdotL = max(dot(N, L), 0.0);
			if(NdotL > 0.0)
			{
				// sample from the environment's mip level based on roughness/pdf
				float D   = DistributionGGX(N, H, roughness);
				float NdotH = max(dot(N, H), 0.0);
				float HdotV = max(dot(H, V), 0.0);
				float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;
	
				float resolution = 512.0; // resolution of source cubemap (per face)
				float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
				float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
	
				float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);
	
				prefilteredColor += textureLod(environmentMap, L, mipLevel).rgb * NdotL;
				totalWeight      += NdotL;
			}
		}
	
		prefilteredColor = prefilteredColor / totalWeight;
	
		FragColor = vec4(prefilteredColor, 1.0);
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcBrdfVertex() {
	const char* shadersource = R"(

	#version 330 core
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexCoords;
	
	out vec2 TexCoords;
	
	void main()
	{
		TexCoords = aTexCoords;
		gl_Position = vec4(aPos, 1.0);
	}
)";
	
		return std::string(shadersource);
}
	
const std::string gRenderer::getShaderSrcBrdfFragment() {
	const char* shadersource = R"(
	#version 330 core
	out vec2 FragColor;
	in vec2 TexCoords;
	
	const float PI = 3.14159265359;
	// ----------------------------------------------------------------------------
	// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
	// efficient VanDerCorpus calculation.
	float RadicalInverse_VdC(uint bits)
	{
		 bits = (bits << 16u) | (bits >> 16u);
		 bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
		 bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
		 bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
		 bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
		 return float(bits) * 2.3283064365386963e-10; // / 0x100000000
	}
	// ----------------------------------------------------------------------------
	vec2 Hammersley(uint i, uint N)
	{
		return vec2(float(i)/float(N), RadicalInverse_VdC(i));
	}
	// ----------------------------------------------------------------------------
	vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
	{
		float a = roughness*roughness;
	
		float phi = 2.0 * PI * Xi.x;
		float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
		float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
		// from spherical coordinates to cartesian coordinates - halfway vector
		vec3 H;
		H.x = cos(phi) * sinTheta;
		H.y = sin(phi) * sinTheta;
		H.z = cosTheta;
	
		// from tangent-space H vector to world-space sample vector
		vec3 up          = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
		vec3 tangent   = normalize(cross(up, N));
		vec3 bitangent = cross(N, tangent);
	
		vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
		return normalize(sampleVec);
	}
	// ----------------------------------------------------------------------------
	float GeometrySchlickGGX(float NdotV, float roughness)
	{
		// note that we use a different k for IBL
		float a = roughness;
		float k = (a * a) / 2.0;
	
		float nom   = NdotV;
		float denom = NdotV * (1.0 - k) + k;
	
		return nom / denom;
	}
	// ----------------------------------------------------------------------------
	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
	{
		float NdotV = max(dot(N, V), 0.0);
		float NdotL = max(dot(N, L), 0.0);
		float ggx2 = GeometrySchlickGGX(NdotV, roughness);
		float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
		return ggx1 * ggx2;
	}
	// ----------------------------------------------------------------------------
	vec2 IntegrateBRDF(float NdotV, float roughness)
	{
		vec3 V;
		V.x = sqrt(1.0 - NdotV*NdotV);
		V.y = 0.0;
		V.z = NdotV;
	
		float A = 0.0;
		float B = 0.0;
	
		vec3 N = vec3(0.0, 0.0, 1.0);
	
		const uint SAMPLE_COUNT = 1024u;
		for(uint i = 0u; i < SAMPLE_COUNT; ++i)
		{
			// generates a sample vector that's biased towards the
			// preferred alignment direction (importance sampling).
			vec2 Xi = Hammersley(i, SAMPLE_COUNT);
			vec3 H = ImportanceSampleGGX(Xi, N, roughness);
			vec3 L = normalize(2.0 * dot(V, H) * H - V);
	
			float NdotL = max(L.z, 0.0);
			float NdotH = max(H.z, 0.0);
			float VdotH = max(dot(V, H), 0.0);
	
			if(NdotL > 0.0)
			{
				float G = GeometrySmith(N, V, L, roughness);
				float G_Vis = (G * VdotH) / (NdotH * NdotV);
				float Fc = pow(1.0 - VdotH, 5.0);
	
				A += (1.0 - Fc) * G_Vis;
				B += Fc * G_Vis;
			}
		}
		A /= float(SAMPLE_COUNT);
		B /= float(SAMPLE_COUNT);
		return vec2(A, B);
	}
	// ----------------------------------------------------------------------------
	void main()
	{
		vec2 integratedBRDF = IntegrateBRDF(TexCoords.x, TexCoords.y);
		FragColor = integratedBRDF;
	}
)";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFboVertex() {
	const char* shadersource = R"(
	#version 330 core
	layout (location = 0) in vec2 aPos;
	layout (location = 1) in vec2 aTexCoords;

	out vec2 TexCoords;

	void main()
	{
		gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
		TexCoords = aTexCoords;
	}
)";


	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFboFragment() {
	const char* shadersource = R"(
	#version 330 core
	out vec4 FragColor;
	
	in vec2 TexCoords;
	
	uniform sampler2D screenTexture;
	
	void main()
	{ 
		FragColor = vec4(texture(screenTexture, TexCoords).rgb, 1.0);
	}
)";

	return std::string(shadersource);
}
