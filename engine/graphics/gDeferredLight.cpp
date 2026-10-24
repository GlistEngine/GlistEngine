#include "gDeferredLight.h"
#include "gRenderer.h"
#include <iostream>
#include "gUtils.h"
#include "graphics/shaders/deferred_geom_vert.h"
#include "graphics/shaders/deferred_geom_frag.h"
#include "graphics/shaders/deferred_light_vert.h"
#include "graphics/shaders/deferred_light_frag.h"
#include "gLight.h"
#include "gCamera.h"
#include "gShadowMap.h"

namespace {

bool isVulkanRendererActive() {
	gRenderer* activeRenderer = gRenderObject::getRenderer();
	return activeRenderer != nullptr && activeRenderer->isVulkan();
}

}

gDeferredLight::gDeferredLight() : gBufferFBO(0), positionTexture(0), normalTexture(0), albedoTexture(0), rboDepth(0), width(0), height(0),
	renderTargetFBO(0), depthTestWasEnabled(false), geometryPassActive(false), geometryShader(nullptr), lightingShader(nullptr),
	quadVAO(0), quadVBO(0) {
	renderTargetViewport[0] = 0;
	renderTargetViewport[1] = 0;
	renderTargetViewport[2] = 0;
	renderTargetViewport[3] = 0;
	geometryShader = new gShader();
	lightingShader = new gShader();

}

gDeferredLight::~gDeferredLight() {
	// A Vulkan window has no OpenGL context. Its deferred resources will be owned
	// by the Vulkan backend; never try to release them through GL here.
	if(!isVulkanRendererActive()) {
		releaseGBuffer();
		if(quadVBO != 0) glDeleteBuffers(1, &quadVBO);
		if(quadVAO != 0) glDeleteVertexArrays(1, &quadVAO);
	} else {
		gRenderObject::getRenderer()->releaseDeferredLighting();
	}
	delete geometryShader;
	delete lightingShader;
}

void gDeferredLight::setup(int screenWidth, int screenHeight) {
	if(screenWidth <= 0 || screenHeight <= 0) {
		gLoge("gDeferredLight") << "Deferred rendering requires a positive render size.";
		return;
	}
	if(isVulkanRendererActive()) {
		width = screenWidth;
		height = screenHeight;
		geometryPassActive = false;
		gRenderObject::getRenderer()->setupDeferredLighting(width, height);
		return;
	}

	if(gBufferFBO == 0) {
		geometryShader -> loadProgram(
			std::string(shader_deferred_geom_vert.data(), shader_deferred_geom_vert.size()),
			std::string(shader_deferred_geom_frag.data(), shader_deferred_geom_frag.size())
		);
		lightingShader -> loadProgram(
			std::string(shader_deferred_light_vert.data(), shader_deferred_light_vert.size()),
			std::string(shader_deferred_light_frag.data(), shader_deferred_light_frag.size())
		);
    }

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

    resize(screenWidth, screenHeight);
}

void gDeferredLight::resize(int screenWidth, int screenHeight) {
	if(screenWidth <= 0 || screenHeight <= 0) return;
	if(geometryPassActive) {
		gLogw("gDeferredLight") << "Ignoring resize during an active geometry pass.";
		return;
	}
	if(isVulkanRendererActive()) {
		if(gRenderObject::getRenderer()->resizeDeferredLighting(screenWidth, screenHeight)) {
			width = screenWidth;
			height = screenHeight;
		}
		return;
	}
	if(gBufferFBO != 0 && width == screenWidth && height == screenHeight) return;

	GLint previousFBO = 0;
	GLint previousViewport[4];
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFBO);
	glGetIntegerv(GL_VIEWPORT, previousViewport);
	releaseGBuffer();
	width = screenWidth;
	height = screenHeight;
	setupGBuffer();
	glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(previousFBO));
	glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
}

void gDeferredLight::setupGBuffer() {
	if(isVulkanRendererActive()) return;
	//FBO
	glGenFramebuffers(1, &gBufferFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);

	//Position
	glGenTextures(1, &positionTexture);
	glBindTexture(GL_TEXTURE_2D, positionTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, positionTexture, 0);

	//Normal
	glGenTextures(1, &normalTexture);
	glBindTexture(GL_TEXTURE_2D, normalTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, normalTexture, 0);

	//Albedo
	glGenTextures(1, &albedoTexture);
	glBindTexture(GL_TEXTURE_2D, albedoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, albedoTexture, 0);
	//MRT
	unsigned int attachments[4] = {GL_NONE, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments);

	//Z-Buffer RBO
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		gLoge("gDeferredLight") << "ERROR::G-BUFFER:: Framebuffer is not complete!";
	}
}

void gDeferredLight::releaseGBuffer() {
	if(isVulkanRendererActive()) return;
	if(rboDepth != 0) glDeleteRenderbuffers(1, &rboDepth);
	if(albedoTexture != 0) glDeleteTextures(1, &albedoTexture);
	if(normalTexture != 0) glDeleteTextures(1, &normalTexture);
	if(positionTexture != 0) glDeleteTextures(1, &positionTexture);
	if(gBufferFBO != 0) glDeleteFramebuffers(1, &gBufferFBO);
	rboDepth = 0;
	albedoTexture = 0;
	normalTexture = 0;
	positionTexture = 0;
	gBufferFBO = 0;
}

void gDeferredLight::restoreRenderTarget() {
	if(isVulkanRendererActive()) {
		geometryPassActive = false;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, renderTargetFBO);
	glViewport(renderTargetViewport[0], renderTargetViewport[1], renderTargetViewport[2], renderTargetViewport[3]);
	if(depthTestWasEnabled) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
	geometryPassActive = false;
}

void gDeferredLight::enable() {
	if(isVulkanRendererActive()) {
		gRenderer* activeRenderer = gRenderObject::getRenderer();
		// An activated shadow map makes the Vulkan frame loop draw the canvas twice.
		// The first traversal already owns a depth-only render pass; deferred calls in
		// the shared canvas code belong only to the following scene traversal.
		if(activeRenderer->isShadowPassActive()) {
			geometryPassActive = false;
			return;
		}
		geometryPassActive = activeRenderer->beginDeferredGeometryPass();
		return;
	}
	if(gBufferFBO == 0) {
		gLoge("gDeferredLight") << "Call setup() before enable().";
		return;
	}
	if(geometryPassActive) {
		gLogw("gDeferredLight") << "Geometry pass is already active.";
		return;
	}
	GLint currentFBO = 0;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &currentFBO);
	renderTargetFBO = static_cast<GLuint>(currentFBO);
	glGetIntegerv(GL_VIEWPORT, renderTargetViewport);
	depthTestWasEnabled = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
	geometryPassActive = true;
	//Redirect rendering to our GBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, gBufferFBO);
	glViewport(0, 0, width, height);
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

void gDeferredLight::renderLightingPass(gCamera* camera, gShadowMap* shadowmap) {
	if(isVulkanRendererActive()) {
		gRenderer* activeRenderer = gRenderObject::getRenderer();
		// Pair with enable()'s shadow-traversal no-op. The canvas is intentionally
		// replayed during the scene traversal, where the real lighting pass runs.
		if(activeRenderer->isShadowPassActive()) return;
		if(!geometryPassActive || !activeRenderer->isDeferredGeometryPassActive()) {
			gLoge("gDeferredLight") << "Call enable() before renderLightingPass().";
			geometryPassActive = false;
			return;
		}
		if(camera == nullptr || shadowmap == nullptr) {
			gLoge("gDeferredLight") << "Lighting pass requires valid camera and shadow map objects.";
			activeRenderer->endDeferredLighting();
			geometryPassActive = false;
			return;
		}
		activeRenderer->renderDeferredLightingPass();
		geometryPassActive = false;
		return;
	}
	if(!geometryPassActive) {
		gLoge("gDeferredLight") << "Call enable() before renderLightingPass().";
		return;
	}
	if(camera == nullptr || shadowmap == nullptr) {
		gLoge("gDeferredLight") << "Lighting pass requires valid light, camera, and shadow map objects.";
		restoreRenderTarget();
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, renderTargetFBO);
	glViewport(renderTargetViewport[0], renderTargetViewport[1], renderTargetViewport[2], renderTargetViewport[3]);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if(lightingShader != nullptr) {
		lightingShader->use();

		// G-Buffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, positionTexture);
		lightingShader->setInt("gPosition", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalTexture);
		lightingShader->setInt("gNormal", 1);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, albedoTexture);
		lightingShader->setInt("gAlbedo", 2);

		// Lighting
//		lightingShader->setVec3("lightPos", light->getPosition());
//		lightingShader->setVec3("lightAmbient", light->getAmbientColor()->r, light->getAmbientColor()->g, light->getAmbientColor()->b);
//		lightingShader->setVec3("lightDiffuse", light->getDiffuseColor()->r, light->getDiffuseColor()->g, light->getDiffuseColor()->b);
		lightingShader->setVec3("viewPos", camera->getPosX(), camera->getPosY(), camera->getPosZ());

		// Shadow
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderTargetFBO);
	glBlitFramebuffer(0, 0, width, height,
		renderTargetViewport[0], renderTargetViewport[1],
		renderTargetViewport[0] + renderTargetViewport[2], renderTargetViewport[1] + renderTargetViewport[3],
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	restoreRenderTarget();
}

void gDeferredLight::disable() {
	if(isVulkanRendererActive()) {
		if(geometryPassActive) gRenderObject::getRenderer()->endDeferredLighting();
		geometryPassActive = false;
		return;
	}
	if(geometryPassActive) restoreRenderTarget();
	gRenderer* renderer = gRenderObject::getRenderer();
	renderer->getColorShader()->use();
	renderer->getColorShader()->setInt("isDeferred", 0);
	renderer->getTextureShader()->use();
	renderer->getTextureShader()->setInt("isDeferred", 0);
}

void gDeferredLight::drawFullScreenQuad() {
	if(isVulkanRendererActive()) return;
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





