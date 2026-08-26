#include "gDeferredLight.h"
#include "gRenderer.h"
#include <Gl/glew.h>
#include<iostream>
#include "gUtils.h"
#include "graphics/shaders/deferred_geom_vert.h"
#include "graphics/shaders/deferred_geom_frag.h"
#include "graphics/shaders/deferred_light_vert.h"
#include "graphics/shaders/deferred_light_frag.h"
#include "gLight.h"
#include "gCamera.h"
#include "gShadowMap.h"

gDeferredLight::gDeferredLight() : gBufferFBO(0), positionTexture(0), normalTexture(0), albedoTexture(0), rboDepth(0), width(0), height(0),
geometryShader(nullptr), lightingShader(nullptr), quadVAO(0), quadVBO(0) {
	geometryShader = new gShader();
	lightingShader = new gShader();

}

gDeferredLight::~gDeferredLight() {
//	delete geometryShader;
//	delete lightingShader;

}

void gDeferredLight::setup(int screenWidth, int screenHeight) {
    width = screenWidth;
    height = screenHeight;
    geometryShader -> loadProgram(
    	std::string(shader_deferred_geom_vert.data(), shader_deferred_geom_vert.size()),
		std::string(shader_deferred_geom_frag.data(), shader_deferred_geom_frag.size())
		);
    lightingShader -> loadProgram(
    	std::string(shader_deferred_light_vert.data(), shader_deferred_light_vert.size()),
		std::string(shader_deferred_light_frag.data(), shader_deferred_light_frag.size())
    );

    lightingShader->use();
    
    unsigned int lightsBlockIndex = glGetUniformBlockIndex(lightingShader->id, "Lights");
    if (lightsBlockIndex != GL_INVALID_INDEX) {
       glUniformBlockBinding(lightingShader->id, lightsBlockIndex, 0);
    }
    
    unsigned int sceneBlockIndex = glGetUniformBlockIndex(lightingShader->id, "Scene");
    if (sceneBlockIndex != GL_INVALID_INDEX) {
       glUniformBlockBinding(lightingShader->id, sceneBlockIndex, 1);
    }
    glUseProgram(0);

    setupGBuffer();
}

void gDeferredLight::setupGBuffer() {
	//FBO
	glGenFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

	//Position
	glGenTextures(1, &positionTexture);
	glBindTexture(GL_TEXTURE_2D, positionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positionTexture, 0);

	//Normal
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalTexture, 0);

	//Albedo
	glGenTextures(1, &albedoTexture);
	glBindTexture(GL_TEXTURE_2D, albedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, albedoTexture, 0);
	//MRT
	unsigned int attachments[4] = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	//Z-Buffer RBO
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		gLoge("gDeferredLight") << "ERROR::G-BUFFER:: Framebuffer is not complete!";
	}
}

void gDeferredLight::enable() {
	//Redirect rendering to our GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

   //Smart Toggle
	gRenderer* renderer = gRenderObject::getRenderer();
	renderer->getColorShader()->use();
	renderer->getColorShader()->setInt("isDeferred", 1);
	renderer->getTextureShader()->use();
	renderer->getTextureShader()->setInt("isDeferred", 1);

	if(geometryShader != nullptr) {
		geometryShader->use();
		geometryShader->setMat4("view", renderer->getViewMatrix());
		geometryShader->setMat4("projection", renderer->getProjectionMatrix());
	}
}

void gDeferredLight::disable(gLight* light, gCamera* camera, gShadowMap* shadowmap) {
	gRenderer* renderer = gRenderObject::getRenderer();
	GLuint currentFBO = renderer->getBoundFramebuffer();

	glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//LightingPass
	if(lightingShader != nullptr && shadowmap != nullptr) {
		lightingShader->use();

		//G-Buffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, positionTexture);
		lightingShader->setInt("gPosition", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		lightingShader->setInt("gNormal", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, albedoTexture);
		lightingShader->setInt("gAlbedo", 2);

		//Lighting
		lightingShader->setVec3("lightPos", light->getPosition());
		lightingShader->setVec3("lightAmbient", light->getAmbientColor()->r, light->getAmbientColor()->g, light->getAmbientColor()->b);
		lightingShader->setVec3("lightDiffuse", light->getDiffuseColor()->r, light->getDiffuseColor()->g, light->getDiffuseColor()->b);
		lightingShader->setVec3("viewPos", camera->getPosX(), camera->getPosY(), camera->getPosZ());

		//Shadow
		lightingShader->setVec3("shadowLightPos", shadowmap->getLight()->getPosition());
		lightingShader->setMat4("lightMatrix", shadowmap->getLightMatrix());

		glActiveTexture(GL_TEXTURE9);
		glBindTexture(GL_TEXTURE_2D, shadowmap->getDepthFbo().getTextureId());
		lightingShader->setInt("shadowMap", 9);

		glDisable(GL_DEPTH_TEST);
		drawFullScreenQuad();
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBufferFBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, currentFBO);
	glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);

	//ForwardPass - isDeferred == 2 (UI / Unlit Pass)
	renderer->getColorShader()->use();
	renderer->getColorShader()->setInt("isDeferred", 2);
	renderer->getTextureShader()->use();
	renderer->getTextureShader()->setInt("isDeferred", 2);
}

void gDeferredLight::drawFullScreenQuad() {
	if(quadVAO == 0) {
		//Each row: X, Y, Z (position) | U, V (Texture Coordinates)
		float quadVertices[] = {
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, // Top-left
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // Bottom-left
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f, // Top-right
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // Bottom-right
		};

		//Generate buffers
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);

		//Send data to GPU
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		//Position Attribute
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

		// Texture Coordinate Attribute
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}





