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
#ifdef WIN32
	glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1);
#endif
}

void gRenderer::disableAlphaTest() {
#ifdef WIN32
    glDisable(GL_ALPHA_TEST);
#endif
}

const std::string gRenderer::getShaderSrcColorVertex() {
	const char* shadersource = R"glsl(
	#version 330 core
	layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
	layout (location = 1) in vec3 aNormal;
	layout (location = 2) in vec2 aTexCoords;
	layout (location = 3) in vec3 aTangent;
	layout (location = 4) in vec3 aBitangent;
	layout (location = 5) in int aUseNormalMap;

	uniform mat4 model;
	uniform mat4 view;
	uniform mat4 projection;
	uniform vec3 lightPos;
	uniform vec3 viewPos;

	out vec3 Normal;
	out vec3 FragPos;
	out vec2 TexCoords;
	flat out int mUseNormalMap;
	out vec3 TangentLightPos;
	out vec3 TangentViewPos;
	out vec3 TangentFragPos;

	void main() {
	    FragPos = vec3(model * vec4(aPos, 1.0));
	    Normal = mat3(transpose(inverse(model))) * aNormal;
	    TexCoords = aTexCoords;
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
	    
	    gl_Position = projection * view * vec4(FragPos, 1.0);
	}
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcColorFragment() {
	const char* shadersource = R"glsl(
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
	uniform Light light;

	uniform vec4 renderColor;
	uniform vec3 viewPos; 

	in vec3 Normal;
	in vec3 FragPos;
	in vec2 TexCoords;
	flat in int mUseNormalMap;
	in vec3 TangentLightPos;
	in vec3 TangentViewPos;
	in vec3 TangentFragPos;

	out vec4 FragColor;

	void main() {
	    vec4 ambient;
		vec4 diffuse;
	    vec4 specular;

	    // ambient
	    if (material.useDiffuseMap > 0) {
	        ambient = light.ambient * texture(material.diffusemap, TexCoords).rgba;
	    } else {
	        ambient = light.ambient * material.ambient;
	    }

		if (light.type > 0) {
			vec3 norm;
			if (mUseNormalMap > 0) {
			    norm = normalize(texture(material.normalMap, TexCoords).rgb * 2.0 - 1.0);  // this normal is in tangent space
			} else {
				norm = normalize(Normal);
			}

		    // diffuse 
			float diff;
			float distance;
			vec3 lightDir;
			if (light.type > 1) {
				if (mUseNormalMap > 0) {
				    lightDir = normalize(TangentLightPos - TangentFragPos);
				    distance = length(TangentLightPos - TangentFragPos);
				} else {
					lightDir = normalize(light.position - FragPos);
					distance = length(light.position - FragPos);
				}
			} else {
	//			lightDir = normalize(light.position);
				lightDir = normalize(-light.direction);
			}
			if (mUseNormalMap > 0) {
			    diff = max(dot(lightDir, norm), 0.0);
			} else {
				diff = max(dot(norm, lightDir), 0.0);
			}
		    if (material.useDiffuseMap > 0) {
			    diffuse = light.diffuse * diff * texture(material.diffusemap, TexCoords).rgba;
		    } else {
			    diffuse = light.diffuse * (diff * material.diffuse);
		    }
		    
		    // specular
		    vec3 viewDir;
		    vec3 reflectDir;
		    float spec;
		    if (mUseNormalMap > 0) {
		    	viewDir = normalize(TangentViewPos - TangentFragPos);
		    	reflectDir = reflect(-lightDir, norm);
		    	vec3 halfwayDir = normalize(lightDir + viewDir);
		    	spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
		    } else {
		    	viewDir = normalize(viewPos - FragPos);
		    	reflectDir = reflect(-lightDir, norm);
		    	spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
		    }
		    if (material.useSpecularMap > 0) {
			    specular = light.specular * spec * texture(material.specularmap, TexCoords).rgba;
		    } else {
			    specular = light.specular * (spec * material.specular);
		    }
		    
		    if (light.type > 1) {
		    	if (light.type == 3) {
		   		    // spotlight (with soft edges)
				    float theta = dot(lightDir, normalize(-light.direction)); 
				    float epsilon = (light.cutOff - light.outerCutOff);
				    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
				    diffuse *= intensity;
				    specular *= intensity;
		    	}
		
			    // attenuation
			    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
			    ambient *= attenuation;
			    diffuse *= attenuation;
			    specular *= attenuation;
		    }
	    }
	        
	    FragColor = (ambient + diffuse + specular) * renderColor;
	}
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureVertex() {
	const char* shadersource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcTextureFragment() {
	const char* shadersource = R"glsl(
#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D texture_diffuse1;

void main()
{    
    FragColor = texture(texture_diffuse1, TexCoords);
}
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageVertex() {
	const char* shadersource = R"glsl(
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
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcImageFragment() {
	const char* shadersource = R"glsl(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor;

void main()
{    
    color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
} 
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontVertex() {
	const char* shadersource = R"glsl(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
	)glsl";

	return std::string(shadersource);
}

const std::string gRenderer::getShaderSrcFontFragment() {
	const char* shadersource = R"glsl(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main() {    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
	)glsl";

	return std::string(shadersource);
}
