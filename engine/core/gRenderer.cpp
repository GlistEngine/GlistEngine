/*
 * gRenderManager.cpp
 *
 *  Created on: 4 Ara 2020
 *      Author: Acer
 */

#include "gRenderer.h"

#include "gLight.h"

const int gRenderer::DEPTHTESTTYPE_LESS = 0;
const int gRenderer::DEPTHTESTTYPE_ALWAYS = 1;

int gRenderer::width;
int gRenderer::height;

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


gRenderer::gRenderer() {
	width = gDefaultWidth();
	height = gDefaultHeight();

	// TODO Check matrix maths
	projectionmatrix = glm::mat4(1.0f);
	projectionmatrix = glm::perspective(glm::radians(60.0f), (float)width / height, 0.0f, 1000.0f);
	projectionmatrixold = projectionmatrix;
	projectionmatrix2d = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
	viewmatrix = glm::mat4(1.0f);
	viewmatrixold = viewmatrix;

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

	rendercolor = new gColor();
	rendercolor->set(255, 255, 255, 255);

	globalambientcolor = new gColor();
	globalambientcolor->set(255, 255, 255, 255);

	lightingcolor = new gColor();
	lightingcolor->set(255, 255, 255, 255);
	islightingenabled = false;
	lightingposition = glm::vec3(0.0f);
	li = 0;

	isdepthtestenabled = false;
	depthtesttype = 0;
	depthtesttypeid[0] = GL_LESS;
	depthtesttypeid[1] = GL_ALWAYS;
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

void gRenderer::setProjectionMatrix(glm::mat4 projectionMatrix) {
	projectionmatrix = projectionMatrix;
}

void gRenderer::setProjectionMatrix2d(glm::mat4 projectionMatrix2d) {
	projectionmatrix2d = projectionMatrix2d;
}

void gRenderer::setViewMatrix(glm::mat4 viewMatrix) {
	viewmatrix = viewMatrix;
}

glm::mat4 gRenderer::getProjectionMatrix() {
	return projectionmatrix;
}

glm::mat4 gRenderer::getProjectionMatrix2d() {
	return projectionmatrix2d;
}

glm::mat4 gRenderer::getViewMatrix() {
	return viewmatrix;
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
}

int gRenderer::getWidth() {
	return width;
}

int gRenderer::getHeight() {
	return height;
}

int gRenderer::getScreenWidth() {
	return width;
}

int gRenderer::getScreenHeight() {
	return height;
}


void gRenderer::setColor(int r, int g, int b, int a) {
	rendercolor->set((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}

void gRenderer::setColor(gColor color) {
	rendercolor->set(color.r, color.g, color.b, color.a);
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
}

void gRenderer::disableAlphaBlending() {
    glDisable(GL_BLEND);
}

void gRenderer::enableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1);
#endif
}

void gRenderer::disableAlphaTest() {
#if defined(WIN32) || defined(LINUX)
    glDisable(GL_ALPHA_TEST);
#endif
}

const std::string gRenderer::getShaderSrcColorVertex() {
	const char* shadersource =
"	#version 330 core\n"
"	layout (location = 0) in vec3 aPos; // the position variable has attribute position 0\n"
"	layout (location = 1) in vec3 aNormal;\n"
"	layout (location = 2) in vec2 aTexCoords;\n"
"	layout (location = 3) in vec3 aTangent;\n"
"	layout (location = 4) in vec3 aBitangent;\n"
"	layout (location = 5) in int aUseNormalMap;\n"
"   uniform int aUseShadowMap;\n"
"\n"
"	uniform mat4 model;\n"
"	uniform mat4 view;\n"
"	uniform mat4 projection;\n"
"	uniform vec3 lightPos;\n"
"	uniform vec3 viewPos;\n"
"	uniform mat4 lightMatrix;\n"
"\n"
"	flat out int mUseNormalMap;\n"
"	flat out int mUseShadowMap;\n"
"\n"
"	out vec3 Normal;\n"
"	out vec3 FragPos;\n"
"	out vec2 TexCoords;\n"
"   out vec4 FragPosLightSpace;\n"
"	out vec3 TangentLightPos;\n"
"	out vec3 TangentViewPos;\n"
"	out vec3 TangentFragPos;\n"
"\n"
"	void main() {\n"
"    	mUseShadowMap = aUseShadowMap;\n"
"	    FragPos = vec3(model * vec4(aPos, 1.0));\n"
"	    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
"	    TexCoords = aTexCoords;\n"
"		FragPosLightSpace = lightMatrix * vec4(FragPos, 1.0);\n"
"	    mUseNormalMap = aUseNormalMap;\n"
"\n"
"	    if (aUseNormalMap > 0) {\n"
"		    mat3 normalMatrix = transpose(inverse(mat3(model)));\n"
"		    vec3 T = normalize(normalMatrix * aTangent);\n"
"		    vec3 N = normalize(normalMatrix * aNormal);\n"
"		    T = normalize(T - dot(T, N) * N);\n"
"		    vec3 B = cross(N, T);\n"
"		    \n"
"		    mat3 TBN = transpose(mat3(T, B, N));    \n"
"		    TangentLightPos = TBN * lightPos;\n"
"		    TangentViewPos  = TBN * viewPos;\n"
"		    TangentFragPos  = TBN * FragPos;\n"
"	    }\n"
"\n"
"	    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"	}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcColorFragment() {
	const char* shadersource =
"	#version 330 core\n"
"\n"
"	struct Material {\n"
"	    vec4 ambient;\n"
"	    vec4 diffuse;\n"
"	    vec4 specular;\n"
"	    float shininess;\n"
"	    sampler2D diffusemap;\n"
"	    sampler2D specularmap;\n"
"	    sampler2D normalMap;\n"
"		int useDiffuseMap;\n"
"		int useSpecularMap;\n"
"	};\n"
"\n"
"	struct Light {\n"
"		int type; //0-ambient, 1-directional, 2-point, 3-spot\n"
"	    vec3 position;\n"
"	    vec3 direction;\n"
"	    float cutOff;\n"
"	    float outerCutOff;\n"
"\n"
"	    vec4 ambient;\n"
"	    vec4 diffuse;\n"
"	    vec4 specular;\n"
"		\n"
"	    float constant;\n"
"	    float linear;\n"
"	    float quadratic;\n"
"	};\n"
"\n"
"	uniform Material material;\n"
"	uniform Light light;\n"
"\n"
"	uniform vec4 renderColor;\n"
"	uniform vec3 viewPos; \n"
"\n"
"	in vec3 Normal;\n"
"	in vec3 FragPos;\n"
"	in vec2 TexCoords;\n"
"   in vec4 FragPosLightSpace;\n"
"	flat in int mUseNormalMap;\n"
"	in vec3 TangentLightPos;\n"
"	in vec3 TangentViewPos;\n"
"	in vec3 TangentFragPos;\n"
"\n"
"	flat in int mUseShadowMap;\n"
"	uniform sampler2D shadowMap;\n"
"   uniform vec3 shadowLightPos;\n"
"\n"
"	out vec4 FragColor;\n"
"\n"
"   float calculateShadow(vec4 fragPosLightSpace) {\n"
"       vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;\n"
"       projCoords = projCoords * 0.5 + 0.5;\n"
"       float closestDepth = texture(shadowMap, projCoords.xy).r; \n"
"       float currentDepth = projCoords.z;\n"
"       vec3 normal = normalize(Normal);\n"
"       vec3 lightDir = normalize(shadowLightPos - FragPos);\n"
"       float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);\n"
"       float shadow = 0.0;\n"
"       vec2 texelSize = 1.0 / textureSize(shadowMap, 0);\n"
"       for(int x = -1; x <= 1; ++x) {\n"
"           for(int y = -1; y <= 1; ++y) {\n"
"               float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;\n"
"               shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;\n"
"           }\n"
"       }\n"
"       shadow /= 9.0;\n"
"       if(projCoords.z > 1.0) shadow = 0.0;\n"
"       return shadow;\n"
"   }\n"
"\n"
"	void main() {\n"
"	    vec4 ambient;\n"
"		vec4 diffuse;\n"
"	    vec4 specular;\n"
"\n"
"	    // ambient\n"
"	    if (material.useDiffuseMap > 0) {\n"
"	        ambient = light.ambient * texture(material.diffusemap, TexCoords).rgba;\n"
"	    } else {\n"
"	        ambient = light.ambient * material.ambient;\n"
"	    }\n"
"\n"
"		if (light.type > 0) {\n"
"			vec3 norm;\n"
"			if (mUseNormalMap > 0) {\n"
"			    norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space\n"
"			} else {\n"
"				norm = normalize(Normal);\n"
"			}\n"
"\n"
"		    // diffuse \n"
"			float diff;\n"
"			float distance;\n"
"			vec3 lightDir;\n"
"			if (light.type > 1) {\n"
"				if (mUseNormalMap > 0) {\n"
"				    lightDir = normalize(TangentLightPos - TangentFragPos);\n"
"				    distance = length(TangentLightPos - TangentFragPos);\n"
"				} else {\n"
"					lightDir = normalize(light.position - FragPos);\n"
"					distance = length(light.position - FragPos);\n"
"				}\n"
"			} else {\n"
"				lightDir = normalize(-light.direction);\n"
"			}\n"
"			if (mUseNormalMap > 0) {\n"
"			    diff = max(dot(lightDir, norm), 0.0);\n"
"			} else {\n"
"				diff = max(dot(norm, lightDir), 0.0);\n"
"			}\n"
"		    if (material.useDiffuseMap > 0) {\n"
"			    diffuse = light.diffuse * diff * texture(material.diffusemap, TexCoords).rgba;\n"
"		    } else {\n"
"			    diffuse = light.diffuse * (diff * material.diffuse);\n"
"		    }\n"
"\n"
"		    // specular\n"
"		    vec3 viewDir;\n"
"		    vec3 reflectDir;\n"
"		    float spec;\n"
"		    if (mUseNormalMap > 0) {\n"
"		    	viewDir = normalize(TangentViewPos - TangentFragPos);\n"
"		    	reflectDir = reflect(-lightDir, norm);\n"
"		    	vec3 halfwayDir = normalize(lightDir + viewDir);\n"
"		    	spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);\n"
"		    } else {\n"
"		    	viewDir = normalize(viewPos - FragPos);\n"
"		    	reflectDir = reflect(-lightDir, norm);\n"
"		    	spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"		    }\n"
"		    if (material.useSpecularMap > 0) {\n"
"			    specular = light.specular * spec * texture(material.specularmap, TexCoords).rgba;\n"
"		    } else {\n"
"			    specular = light.specular * (spec * material.specular);\n"
"		    }\n"
"\n"
"		    if (light.type > 1) {\n"
"		    	if (light.type == 3) {\n"
"		   		    // spotlight (with soft edges)\n"
"				    float theta = dot(lightDir, normalize(-light.direction)); \n"
"				    float epsilon = (light.cutOff - light.outerCutOff);\n"
"				    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);\n"
"				    diffuse *= intensity;\n"
"				    specular *= intensity;\n"
"		    	}\n"
"\n"
"			    // attenuation\n"
"			    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));\n"
"			    ambient *= attenuation;\n"
"			    diffuse *= attenuation;\n"
"			    specular *= attenuation;\n"
"		    }\n"
"	    }\n"
"\n"
"	    if (mUseShadowMap > 0) {\n"
"			float shadowing = 1.0 - calculateShadow(FragPosLightSpace);\n"
"			diffuse *= shadowing;\n"
"			specular *= shadowing;\n"
"	    }"
"\n"
"	    FragColor = (ambient + diffuse + specular) * renderColor;\n"
"	}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureVertex() {
	const char* shadersource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoords;\n"
"\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"    TexCoords = aTexCoords;    \n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureFragment() {
	const char* shadersource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"  \n"
"in vec2 TexCoords;\n"
"\n"
"uniform sampler2D texture_diffuse1;\n"
"\n"
"void main()\n"
"{    \n"
"    FragColor = texture(texture_diffuse1, TexCoords);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageVertex() {
	const char* shadersource =
"#version 330 core\n"
"layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>\n"
"	\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 projection;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = vertex.zw;\n"
"    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageFragment() {
	const char* shadersource =
"#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D image;\n"
"uniform vec3 spriteColor;\n"
"\n"
"void main()\n"
"{    \n"
"    color = vec4(spriteColor, 1.0) * texture(image, TexCoords);\n"
"} \n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontVertex() {
	const char* shadersource =
"#version 330 core\n"
"layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>\n"
"out vec2 TexCoords;\n"
"\n"
"uniform mat4 projection;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
"    TexCoords = vertex.zw;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontFragment() {
	const char* shadersource =
"#version 330 core\n"
"in vec2 TexCoords;\n"
"out vec4 color;\n"
"\n"
"uniform sampler2D text;\n"
"uniform vec3 textColor;\n"
"\n"
"void main() {    \n"
"    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
"    color = vec4(textColor, 1.0) * sampled;\n"
"}\n";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxVertex() {
	const char* shadersource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"\n"
"out vec3 TexCoords;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 projection;\n"
"uniform mat4 view;\n"
"\n"
"void main()\n"
"{\n"
"    TexCoords = aPos;\n"
"    vec4 pos = projection * view * model * vec4(aPos, 1.0);\n"
"    gl_Position = pos.xyww;\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcSkyboxFragment() {
	const char* shadersource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"in vec3 TexCoords;\n"
"\n"
"uniform samplerCube skymap;\n"
"\n"
"void main() {\n"
"    FragColor = texture(skymap, TexCoords);\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapVertex() {
	const char* shadersource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"\n"
"uniform mat4 lightMatrix;\n"
"uniform mat4 model;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = lightMatrix * model * vec4(aPos, 1.0);\n"
"}";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcShadowmapFragment() {
	const char* shadersource =
"#version 330 core\n"
"\n"
"void main() {\n"
"    // gl_FragDepth = gl_FragCoord.z;\n"
"}";

	return std::string(shadersource);
}


