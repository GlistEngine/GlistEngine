/*
 * gSkybox.cpp
 *
 *  Created on: 18 Mar 2021
 *      Author: Acer
 */

#include "gSkybox.h"
#include "gShader.h"
#include "gTexture.h"
#include "gFbo.h"
#include "gTracy.h"
// std::min and the scratch buffer the Vulkan face upload builds. Both were reaching
// this file only through another header on one toolchain.
#include <algorithm>
#include <vector>

glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
glm::mat4 captureViews[] = {
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3( 0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

gSkybox::gSkybox() {
	id = GL_NONE;

	width = 0;
	height = 0;
	nrChannels = 0;
	ishdr = false;
	ispbr = false;
	scale(200);
	setupRenderData();
}

gSkybox::~gSkybox() {
	if(renderer != nullptr && renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
		}
	}
}

unsigned int gSkybox::getTextureId() const {
	return id;
}

unsigned int gSkybox::loadTextures(const std::vector<std::string>& paths) {
	std::vector<std::string> fullpaths;
	fullpaths.resize(6);
	for (int i = 0; i < paths.size(); i++) {
		fullpaths[i] = gGetTexturesDir() + paths[i];
	}
	return load(fullpaths);
}

unsigned int gSkybox::load(const std::vector<std::string>& fullPaths) {
	// Vulkan gets six plain 2D textures instead of a cube map; see drawVulkan.
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(size_t i = 0; i < std::min<size_t>(6, fullPaths.size()); i++) {
			// Forced to three channels so every face has the layout uploadVulkanFace
			// hands to texImage2D, whatever the file happens to carry.
			unsigned char* data = stbi_load(fullPaths[i].c_str(), &width, &height, &nrChannels, 3);
			if(data != nullptr) vkfaceids[i] = uploadVulkanFace(width, height, data);
			else gLoge("gSkyBox") << "Cubemap tex failed to load at path: " << fullPaths[i];
			stbi_image_free(data);
		}
		id = vkfaceids[0];
		return id;
	}

	skymapslot = GL_TEXTURE0;
	skymapint = 0;

	renderer->enableCubeMap();

	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);

	for (unsigned int i = 0; i < fullPaths.size(); i++) {
        unsigned char *data = stbi_load(fullPaths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
        	renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
        	gLoge("gSkyBox") << "Cubemap tex failed to load at path: " << fullPaths[i];
            stbi_image_free(data);
        }
    }
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	if(ispbr) generatePbrMaps();
	renderer->unbindSkyTexture(0);

    return id;
}

void gSkybox::loadSkybox(gImage* images) {
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(int i = 0; i < 6; i++) {
			vkfaceids[i] = uploadVulkanFace(images[i].getWidth(), images[i].getHeight(),
					images[i].getImageData());
		}
		id = vkfaceids[0];
		return;
	}

	skymapslot = GL_TEXTURE0;
	skymapint = 0;

	renderer->enableCubeMap();

	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);

	for (unsigned int i = 0; i < 6; i++) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB,
			images[i].getWidth(), images[i].getHeight(), GL_RGB,
			GL_UNSIGNED_BYTE, images[i].getImageData());
    }

	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	if(ispbr) generatePbrMaps();

	renderer->unbindSkyTexture(0);
}

void gSkybox::loadFromData(std::array<int, 6> widths, std::array<int, 6> heights, std::array<void*, 6> rawdata, std::array<bool, 6> ishdr) {
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(int i = 0; i < 6; i++) {
			vkfaceids[i] = uploadVulkanFace(widths[i], heights[i], rawdata[i], ishdr[i]);
		}
		id = vkfaceids[0];
		return;
	}

	skymapslot = GL_TEXTURE0;
	skymapint = 0;

	renderer->enableCubeMap();

	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);

	for (unsigned int i = 0; i < 6; i++) {
		if (ishdr[i]) {
			renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB, widths[i], heights[i], GL_RGB, GL_FLOAT, rawdata[i]);
		} else {
			renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB, widths[i], heights[i], GL_RGB, GL_UNSIGNED_BYTE, rawdata[i]);
		}
	}

	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	if(ispbr) generatePbrMaps();

	renderer->unbindSkyTexture(0);
}

void gSkybox::loadDataSkybox(std::string *data, int width, int height) {
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(int i = 0; i < 6; i++) {
			std::string decoded = gDecodeBase64(data[i]);
			vkfaceids[i] = uploadVulkanFace(width, height, decoded.data());
		}
		id = vkfaceids[0];
		return;
	}

	skymapslot = GL_TEXTURE0;
	skymapint = 0;

	renderer->enableCubeMap();

	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);

	for (unsigned int i = 0; i < 6; i++) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB,
				width, height, GL_RGB, GL_UNSIGNED_BYTE,
				(unsigned char*)gDecodeBase64(data[i]).c_str());
    }

	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	if(ispbr) generatePbrMaps();

	renderer->unbindSkyTexture(0);
}

unsigned int gSkybox::loadTextureEquirectangular(const std::string& texturePath) {
	return loadEquirectangular(gGetTexturesDir() + texturePath);
}

unsigned int gSkybox::loadEquirectangular(const std::string& fullPath) {
	// Not ported: this one projects an HDR panorama into a cube map with a render
	// pass of its own, which needs the framebuffer path Vulkan does not have here
	// yet. Left returning nothing so draw() skips rather than showing a wrong sky.
	if(renderer->isVulkan()) {
		gLogw("gSkyBox") << "Equirectangular skyboxes are not supported on the Vulkan backend yet.";
		id = GL_NONE;
		return id;
	}

	ishdr = true;
	skymapslot = GL_TEXTURE0;
	skymapint = 0;
//	glActiveTexture(skymapslot);
//	glGenTextures(1, &id);
//	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	renderer->enableCubeMap();

	equirectangularToCubemapShader = renderer->getEquirectangularShader();

	captureFBO = renderer->createFramebuffer();
	captureRBO = renderer->createRenderbuffer();
	renderer->bindFramebuffer(captureFBO);
	renderer->bindRenderbuffer(captureRBO);
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 512, 512);
	renderer->attachRenderbufferToFramebuffer(GL_DEPTH_ATTACHMENT, captureRBO);

	//***********************************************
	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 512, 512, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	equirectangularToCubemapShader->use();
	equirectangularToCubemapShader->setInt("equirectangularMap", 0);
	equirectangularToCubemapShader->setMat4("projection", captureProjection);

	// pbr: load the HDR environment map
	gTexture hdr;
	hdr.load(fullPath);
	renderer->bindTexture(hdr.getId(), 0);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	renderer->bindFramebuffer(captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		equirectangularToCubemapShader->setMat4("view", captureViews[i]);
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id);
		renderer->clearScreen(true, true);
		renderCube();
	}
	renderer->bindDefaultFramebuffer();

	// then let OpenGL generate mipmaps from first mip face (combatting visible dots artifact)
	renderer->bindSkyTexture(id);
	renderer->generateSkyMipMap();

	if(ispbr) generatePbrMaps();
	glViewport(0, 0, getScreenWidth(), getScreenHeight());
	return id;
}

void gSkybox::generatePbrMaps() {
	// Image-based lighting needs the irradiance, prefilter and BRDF passes, all of
	// which render into framebuffers the Vulkan backend has no equivalent for yet.
	if(renderer->isVulkan()) return;

	renderer->bindSkyTexture(id, skymapslot);

	pbrShader = renderer->getPbrShader();
	irradianceShader = renderer->getIrradianceShader();
	prefilterShader = renderer->getPrefilterShader();
	brdfShader = renderer->getBrdfShader();

	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------

	irradianceMap = renderer->createTextures();
	renderer->bindSkyTexture(irradianceMap);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 32, 32, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->bindFramebuffer(captureFBO);
	renderer->bindRenderbuffer(captureRBO);
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradianceShader->use();
	irradianceShader->setInt("environmentMap", 0);
	irradianceShader->setMat4("projection", captureProjection);
	renderer->bindSkyTexture(id, GL_TEXTURE0);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	renderer->bindFramebuffer(captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		irradianceShader->setMat4("view", captureViews[i]);
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap);
		renderer->clearScreen(true, true);
		renderCube();
	}
	renderer->bindDefaultFramebuffer();

	// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
	// --------------------------------------------------------------------------------

	prefilterMap = renderer->createTextures();
	renderer->bindSkyTexture(prefilterMap);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 128, 128, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	renderer->generateSkyMipMap();
	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilterShader->use();
	prefilterShader->setInt("environmentMap", 0);
	prefilterShader->setMat4("projection", captureProjection);
	renderer->bindSkyTexture(id, GL_TEXTURE0);

	renderer->bindFramebuffer(captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		renderer->bindRenderbuffer(captureRBO);
		renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader->setFloat("roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i) {
			prefilterShader->setMat4("view", captureViews[i]);
			renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
			renderer->clearScreen(true, true);
			renderCube();
		}
	}
	renderer->bindDefaultFramebuffer();

	// pbr: generate a 2D LUT from the BRDF equations used.
	// ----------------------------------------------------

	brdfLUTTexture = renderer->createTextures();

	// pre-allocate enough memory for the LUT texture.
	renderer->bindTexture(brdfLUTTexture);
	renderer->texImage2D(GL_TEXTURE_2D, GL_RG32F, 512, 512, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
	                                      GL_LINEAR, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	captureFBO = renderer->createFramebuffer();
	captureRBO = renderer->createRenderbuffer();
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 512, 512);
	renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture);

	glViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
	brdfShader->use();
	renderer->clearScreen(true, true);
	renderQuad();

	renderer->bindDefaultFramebuffer();

	pbrShader->use();
	pbrShader->setInt("irradianceMap", 0);
	pbrShader->setInt("prefilterMap", 1);
	pbrShader->setInt("brdfLUT", 2);


	// initialize static shader uniforms before rendering
	// --------------------------------------------------
	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)renderer->getScreenWidth() / (float)renderer->getScreenHeight(), 0.1f, 100.0f);
	pbrShader->use();
	pbrShader->setMat4("projection", projection);


	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	renderer->unbindSkyTexture(0);
	ispbr = true;
}

void gSkybox::bindPbrMaps() {
	// Nothing to bind: generatePbrMaps never ran on this backend.
	if(renderer->isVulkan()) return;

	pbrShader->use();
	pbrShader->setMat4("view", renderer->getViewMatrix());
	pbrShader->setVec3("camPos", renderer->getCameraPosition());
	renderer->bindSkyTexture(irradianceMap, GL_TEXTURE0);
	renderer->bindSkyTexture(prefilterMap, GL_TEXTURE1);
	renderer->bindTexture(brdfLUTTexture, GL_TEXTURE2 - GL_TEXTURE0);
}

unsigned int gSkybox::uploadVulkanFace(int faceWidth, int faceHeight, void* pixels, bool hdr) {
	if(pixels == nullptr || faceWidth <= 0 || faceHeight <= 0) return 0;

	// An HDR source arrives as floats. The Vulkan texture path takes bytes, so the
	// range is clamped and scaled here rather than carrying a float format through
	// for the one case that needs it.
	std::vector<unsigned char> converted;
	void* source = pixels;
	if(hdr) {
		const float* fpixels = static_cast<const float*>(pixels);
		converted.resize(static_cast<size_t>(faceWidth) * faceHeight * 3);
		for(size_t i = 0; i < converted.size(); i++) {
			converted[i] = static_cast<unsigned char>(glm::clamp(fpixels[i], 0.0f, 1.0f) * 255.0f);
		}
		source = converted.data();
	}

	unsigned int texture = renderer->createTextures();
	renderer->bindTexture(texture);
	renderer->texImage2D(GL_TEXTURE_2D, GL_RGB, faceWidth, faceHeight, GL_RGB, GL_UNSIGNED_BYTE, source);
	// Clamped, because a sky face must not wrap: a repeat would fold the opposite
	// edge back in and show a seam along every corner of the box.
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
			GL_LINEAR, GL_LINEAR);
	return texture;
}

void gSkybox::drawVulkan() {
	if(id == GL_NONE) return;

	// A box centred on the camera, big enough to sit outside anything the scene
	// draws but well inside the far plane. It moves with the camera, so the sky
	// never gets closer no matter how far the viewer travels.
	const glm::vec3 c = renderer->getCameraPosition();
	const float s = 200.0f;
	const glm::vec3 corners[8] = {
		c + glm::vec3(-s, -s, -s), c + glm::vec3(s, -s, -s), c + glm::vec3(s, s, -s), c + glm::vec3(-s, s, -s),
		c + glm::vec3(-s, -s, s), c + glm::vec3(s, -s, s), c + glm::vec3(s, s, s), c + glm::vec3(-s, s, s)
	};
	// right, left, top, bottom, front, back - the order the cubemap API loads them in.
	static const int faces[6][6] = {
		{1, 5, 6, 1, 6, 2}, {4, 0, 3, 4, 3, 7}, {3, 2, 6, 3, 6, 7},
		{4, 5, 1, 4, 1, 0}, {5, 4, 7, 5, 7, 6}, {0, 1, 2, 0, 2, 3}
	};

	const glm::mat4 viewprojection = renderer->getProjectionMatrix() * renderer->getViewMatrix();
	for(int face = 0; face < 6; face++) {
		if(vkfaceids[face] == 0) continue;

		float xyzuv[6 * 5];
		for(int vertex = 0; vertex < 6; vertex++) {
			const glm::vec3 position = corners[faces[face][vertex]];
			// Direction from the camera, on the unit cube, which is what a samplerCube
			// would have been given. The per-face mapping below is the OpenGL cube map
			// convention; getting it wrong flips a face rather than shifting it, so it
			// is written out per face instead of derived.
			const glm::vec3 d = (position - c) / s;
			glm::vec2 uv;
			if(face == 0)      uv = {(-d.z + 1.0f) * 0.5f, (-d.y + 1.0f) * 0.5f};
			else if(face == 1) uv = {( d.z + 1.0f) * 0.5f, (-d.y + 1.0f) * 0.5f};
			else if(face == 2) uv = {( d.x + 1.0f) * 0.5f, ( d.z + 1.0f) * 0.5f};
			else if(face == 3) uv = {( d.x + 1.0f) * 0.5f, (-d.z + 1.0f) * 0.5f};
			else if(face == 4) uv = {( d.x + 1.0f) * 0.5f, (-d.y + 1.0f) * 0.5f};
			else               uv = {(-d.x + 1.0f) * 0.5f, (-d.y + 1.0f) * 0.5f};

			float* v = xyzuv + vertex * 5;
			v[0] = position.x; v[1] = position.y; v[2] = position.z;
			v[3] = uv.x;       v[4] = uv.y;
		}
		renderer->drawSkyboxFace(vkfaceids[face], xyzuv, 6, viewprojection);
	}
}

void gSkybox::draw() {
	G_PROFILE_ZONE_SCOPED_N("gSkybox::draw()");
	if(renderer->isVulkan()) {
		drawVulkan();
		return;
	}

	skyboxshader = renderer->getSkyboxShader();
	renderer->enableDepthTestEqual(); // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxshader->use();
	skyboxshader->setInt("aIsHDR", ishdr);
	skyboxshader->setMat4("projection", renderer->getProjectionMatrix());
	skyboxshader->setMat4("view", renderer->getViewMatrix());
	skyboxshader->setMat4("model", localtransformationmatrix.back());

//	skyboxshader->setInt("skymap", skymapint);

	renderer->bindSkyTexture(id, skymapslot);
    vbo.bind();
	renderer->drawElements(GL_TRIANGLES, vbo.getIndicesNum());
    vbo.unbind();
	renderer->unbindSkyTexture();
}

void gSkybox::setupRenderData() {

	float vertexdata[]= {
	    // x,   y,   z,  s,  t,
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Back
		 1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // Front
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // Left
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Right
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // Top
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // Bottom
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 0.0f
    };

	float normaldata[] = {
		-1.0f,  1.0f, -1.0f, // Back
	     1.0f,  1.0f, -1.0f,
	    -1.0f, -1.0f, -1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f,  1.0f, // Front
	     1.0f,  1.0f,  1.0f,
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f,  1.0f,
	    -1.0f,  1.0f, -1.0f, // Left
	    -1.0f,  -1.0f,  -1.0f,
	    -1.0f, -1.0f, 1.0f,
	    -1.0f, 1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f, // Right
	     1.0f,  -1.0f, -1.0f,
	     1.0f, -1.0f,  1.0f,
	     1.0f, 1.0f, 1.0f,
	    -1.0f, -1.0f,  -1.0f, // Top
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f, 1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f, -1.0f, // Bottom
	    -1.0f,  1.0f, 1.0f,
	     1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f
	};


	gIndex indexdata[] = {
	   // back
		 0,  2, 3,
		 0,  1, 3,
	   // front
		 4,  6, 7,
		 4,  5, 7,
	   // left
		 8,  9, 10,
		11,  8, 10,
	   // right
		12, 13, 14,
		15, 12, 14,
	   // top
		16, 17, 18,
		16, 19, 18,
	   // bottom
		20, 21, 22,
		20, 23, 22
	};


	int nv = (sizeof(vertexdata) / sizeof(vertexdata[0])) / 5;
	std::vector<gVertex> verticesb;
	for (int i=0; i<nv; i++) {
		gVertex v;
		v.position.x = vertexdata[(i * 5)];
		v.position.y = vertexdata[(i * 5) + 1];
		v.position.z = vertexdata[(i * 5) + 2];
		v.texcoords.x = vertexdata[(i * 5) + 3];
		v.texcoords.y = vertexdata[(i * 5) + 4];
		v.normal.x = normaldata[(i * 3)];
		v.normal.y = normaldata[(i * 3) + 1];
		v.normal.z = normaldata[(i * 3) + 2];
		verticesb.push_back(v);
	}

	int ni = sizeof(indexdata) / sizeof(indexdata[0]);
	std::vector<gIndex> indicesb;
	for (int i=0; i<ni; i++) {
		indicesb.push_back(indexdata[i]);
	}

	vbo.setVertexData(&verticesb[0], sizeof(gVertex), verticesb.size());
	if (indicesb.size() != 0) vbo.setIndexData(&indicesb[0], indicesb.size());
}


void gSkybox::renderCube() {
	G_PROFILE_ZONE_SCOPED_N("gSkybox::renderCube()");
    // initialize (if necessary)
    if (cubeVAO == 0) {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
    	cubeVAO = renderer->createVAO();
    	cubeVBO = renderer->genBuffers();
        // fill buffer
    	renderer->setVertexBufferData(cubeVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
    	renderer->bindVAO(cubeVAO);
		renderer->enableVertexAttrib(0);
    	renderer->setVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    	renderer->enableVertexAttrib(1);
    	renderer->setVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    	renderer->enableVertexAttrib(2);
    	renderer->setVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		renderer->unbindBuffer(GL_ARRAY_BUFFER);
    	renderer->unbindVAO();
    }
    // render Cube
	renderer->bindVAO(cubeVAO);
	renderer->drawArrays(GL_TRIANGLES, 36);
	renderer->unbindVAO();
}

void gSkybox::renderQuad() {
	G_PROFILE_ZONE_SCOPED_N("gSkybox::renderQuad()");
    if (quadVAO == 0) {
    	renderer->createQuad(quadVAO, quadVBO);
    }
	renderer->bindVAO(quadVAO);
	renderer->drawArrays(GL_TRIANGLES, 36);
	renderer->unbindVAO();
}


