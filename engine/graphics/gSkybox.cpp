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
#include <algorithm>
#include <cmath>
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

static const float gSkyboxPi = 3.14159265358979323846f;

static glm::vec3 gSkyboxFaceDirection(int face, float u, float v) {
	const float x = u * 2.0f - 1.0f;
	const float y = 1.0f - v * 2.0f;
	switch(face) {
	case 0: return glm::normalize(glm::vec3( 1.0f, y, -x)); // right
	case 1: return glm::normalize(glm::vec3(-1.0f, y,  x)); // left
	case 2: return glm::normalize(glm::vec3( x,  1.0f, -y)); // top
	case 3: return glm::normalize(glm::vec3( x, -1.0f,  y)); // bottom
	case 4: return glm::normalize(glm::vec3( x,  y,  1.0f)); // front
	default:return glm::normalize(glm::vec3(-x,  y, -1.0f)); // back
	}
}

static glm::vec3 gSampleEquirectangular(const void* pixels, int width, int height,
		int components, bool hdr, const glm::vec3& direction) {
	const float u = std::atan2(direction.z, direction.x) / (2.0f * gSkyboxPi) + 0.5f;
	const float v = std::asin(glm::clamp(direction.y, -1.0f, 1.0f)) / gSkyboxPi + 0.5f;
	const float sourcex = u * width - 0.5f;
	const float sourcey = v * height - 0.5f;
	const int x0 = static_cast<int>(std::floor(sourcex));
	const int y0 = std::max(0, std::min(height - 1, static_cast<int>(std::floor(sourcey))));
	const int x1 = (x0 + 1 + width) % width;
	const int wrappedx0 = (x0 % width + width) % width;
	const int y1 = std::min(height - 1, y0 + 1);
	const float tx = sourcex - std::floor(sourcex);
	const float ty = sourcey - std::floor(sourcey);

	auto read = [&](int x, int y) {
		const size_t offset = (static_cast<size_t>(y) * width + x) * components;
		auto component = [&](int index) {
			if(index >= components) return 0.0f;
			return hdr ? static_cast<const float*>(pixels)[offset + index]
					: static_cast<const unsigned char*>(pixels)[offset + index] / 255.0f;
		};
		return glm::vec3(component(0), component(1), component(2));
	};

	return glm::mix(glm::mix(read(wrappedx0, y0), read(x1, y0), tx),
			glm::mix(read(wrappedx0, y1), read(x1, y1), tx), ty);
}

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
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(size_t i = 0; i < std::min<size_t>(6, fullPaths.size()); i++) {
			unsigned char* data = stbi_load(fullPaths[i].c_str(), &width, &height, &nrChannels, 3);
			if(data != nullptr) vkfaceids[i] = uploadVulkanFace(width, height, data);
			else gLoge("gSkyBox") << "Cubemap tex failed to load at path: " << fullPaths[i];
			stbi_image_free(data);
		}
		id = vkfaceids[0];
		return id;
	}

	skymapslot = 0;
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
	renderer->unbindSkyTexture();

	return id;
}

void gSkybox::loadSkybox(gImage* images) {
	if(renderer->isVulkan()) {
		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}
		for(int i = 0; i < 6; i++) {
			vkfaceids[i] = uploadVulkanFace(images[i].getWidth(), images[i].getHeight(), images[i].getImageData());
		}
		id = vkfaceids[0];
		return;
	}

	skymapslot = 0;
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
	renderer->unbindSkyTexture();
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

	skymapslot = 0;
	skymapint = 0;
	renderer->enableCubeMap();
	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);

	for (unsigned int i = 0; i < 6; i++) {
		if (ishdr[i]) {
			renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, widths[i], heights[i], GL_RGB, GL_FLOAT, rawdata[i]);
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
			vkfaceids[i] = uploadVulkanFace(width, height, (void*)decoded.data());
		}
		id = vkfaceids[0];
		return;
	}

	skymapslot = 0;
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
	if(renderer->isVulkan()) {
		int panoramawidth = 0;
		int panoramaheight = 0;
		int components = 0;
		const bool hdr = stbi_is_hdr(fullPath.c_str()) != 0;
		void* panorama = hdr
				? static_cast<void*>(stbi_loadf(fullPath.c_str(), &panoramawidth, &panoramaheight, &components, 0))
				: static_cast<void*>(stbi_load(fullPath.c_str(), &panoramawidth, &panoramaheight, &components, 0));
		if(panorama == nullptr || panoramawidth <= 0 || panoramaheight <= 0 || components < 3) {
			gLoge("gSkyBox") << "Equirectangular skybox failed to load at path: " << fullPath;
			stbi_image_free(panorama);
			return id;
		}

		for(unsigned int& faceid : vkfaceids) {
			if(faceid != 0) renderer->deleteTexture(faceid);
			faceid = 0;
		}

		constexpr int facesize = 512;
		std::vector<unsigned char> facepixels(static_cast<size_t>(facesize) * facesize * 3);
		for(int face = 0; face < 6; face++) {
			for(int y = 0; y < facesize; y++) {
				for(int x = 0; x < facesize; x++) {
					const glm::vec3 color = gSampleEquirectangular(panorama, panoramawidth,
							panoramaheight, components, hdr, gSkyboxFaceDirection(face,
									(x + 0.5f) / facesize, (y + 0.5f) / facesize));
					const size_t offset = (static_cast<size_t>(y) * facesize + x) * 3;
					facepixels[offset] = static_cast<unsigned char>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f);
					facepixels[offset + 1] = static_cast<unsigned char>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f);
					facepixels[offset + 2] = static_cast<unsigned char>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f);
				}
			}
			vkfaceids[face] = uploadVulkanFace(facesize, facesize, facepixels.data());
		}
		stbi_image_free(panorama);
		ishdr = hdr;
		id = vkfaceids[0];
		return id;
	}

	ishdr = true;
	skymapslot = 0;
	skymapint = 0;
	renderer->enableCubeMap();

	equirectangularToCubemapShader = renderer->getEquirectangularShader();
	captureFBO = renderer->createFramebuffer();
	captureRBO = renderer->createRenderbuffer();
	renderer->bindFramebuffer(captureFBO);
	renderer->bindRenderbuffer(captureRBO);
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 512, 512);
	renderer->attachRenderbufferToFramebuffer(GL_DEPTH_ATTACHMENT, captureRBO);

	id = renderer->createTextures();
	renderer->bindSkyTexture(id, skymapslot);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 512, 512, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	equirectangularToCubemapShader->use();
	equirectangularToCubemapShader->setInt("equirectangularMap", 0);
	equirectangularToCubemapShader->setMat4("projection", captureProjection);

	gTexture hdr;
	hdr.load(fullPath);
	renderer->bindTexture(hdr.getId(), 0);

	renderer->setViewport(0, 0, 512, 512);
	renderer->bindFramebuffer(captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		equirectangularToCubemapShader->setMat4("view", captureViews[i]);
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, id);
		renderer->clearScreen(true, true);
		renderCube();
	}
	renderer->bindDefaultFramebuffer();

	renderer->bindSkyTexture(id);
	renderer->generateSkyMipMap();

	if(ispbr) generatePbrMaps();
	renderer->setViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
	return id;
}

void gSkybox::generatePbrMaps() {
	if(renderer->isVulkan()) return;

	renderer->bindSkyTexture(id, skymapslot);

	pbrShader = renderer->getPbrShader();
	irradianceShader = renderer->getIrradianceShader();
	prefilterShader = renderer->getPrefilterShader();
	brdfShader = renderer->getBrdfShader();

	irradianceMap = renderer->createTextures();
	renderer->bindSkyTexture(irradianceMap);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 32, 32, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	renderer->bindFramebuffer(captureFBO);
	renderer->bindRenderbuffer(captureRBO);
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 32, 32);

	irradianceShader->use();
	irradianceShader->setInt("environmentMap", 0);
	irradianceShader->setMat4("projection", captureProjection);
	renderer->bindSkyTexture(id, 0);

	renderer->setViewport(0, 0, 32, 32);
	renderer->bindFramebuffer(captureFBO);
	for (unsigned int i = 0; i < 6; ++i) {
		irradianceShader->setMat4("view", captureViews[i]);
		renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap);
		renderer->clearScreen(true, true);
		renderCube();
	}
	renderer->bindDefaultFramebuffer();

	prefilterMap = renderer->createTextures();
	renderer->bindSkyTexture(prefilterMap);
	for (unsigned int i = 0; i < 6; ++i) {
		renderer->texImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, GL_RGB32F, 128, 128, GL_RGB, GL_FLOAT, nullptr);
	}
	renderer->setWrappingAndFiltering(GL_TEXTURE_CUBE_MAP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	renderer->generateSkyMipMap();

	prefilterShader->use();
	prefilterShader->setInt("environmentMap", 0);
	prefilterShader->setMat4("projection", captureProjection);
	renderer->bindSkyTexture(id, 0);

	renderer->bindFramebuffer(captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		renderer->bindRenderbuffer(captureRBO);
		renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		renderer->setViewport(0, 0, mipWidth, mipHeight);

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

	brdfLUTTexture = renderer->createTextures();
	renderer->bindTexture(brdfLUTTexture);
	renderer->texImage2D(GL_TEXTURE_2D, GL_RG32F, 512, 512, GL_RG, GL_FLOAT, 0);
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	captureFBO = renderer->createFramebuffer();
	captureRBO = renderer->createRenderbuffer();
	renderer->setRenderbufferStorage(GL_DEPTH_COMPONENT24, 512, 512);
	renderer->attachTextureToFramebuffer(GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture);

	renderer->setViewport(0, 0, renderer->getScreenWidth(), renderer->getScreenHeight());
	brdfShader->use();
	renderer->clearScreen(true, true);
	renderQuad();

	renderer->bindDefaultFramebuffer();

	pbrShader->use();
	pbrShader->setInt("irradianceMap", 0);
	pbrShader->setInt("prefilterMap", 1);
	pbrShader->setInt("brdfLUT", 2);

	glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)renderer->getScreenWidth() / (float)renderer->getScreenHeight(), 0.1f, 100.0f);
	pbrShader->use();
	pbrShader->setMat4("projection", projection);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	renderer->unbindSkyTexture(0);
	ispbr = true;
}

void gSkybox::bindPbrMaps() {
	if(renderer->isVulkan()) return;

	pbrShader->use();
	pbrShader->setMat4("view", renderer->getViewMatrix());
	pbrShader->setVec3("camPos", renderer->getCameraPosition());
	renderer->bindSkyTexture(irradianceMap, 0);
	renderer->bindSkyTexture(prefilterMap, 1);
	renderer->bindTexture(brdfLUTTexture, 2);
}

unsigned int gSkybox::uploadVulkanFace(int faceWidth, int faceHeight, void* pixels, bool hdr) {
	if(pixels == nullptr || faceWidth <= 0 || faceHeight <= 0) return 0;

	std::vector<unsigned char> converted;
	void* source = pixels;

	if(hdr) {
		const float* fpixels = static_cast<const float*>(pixels);
		size_t pixelCount = static_cast<size_t>(faceWidth) * faceHeight;
		converted.resize(pixelCount * 4);

		for(size_t i = 0; i < pixelCount; i++) {
			converted[i * 4 + 0] = static_cast<unsigned char>(glm::clamp(fpixels[i * 3 + 0], 0.0f, 1.0f) * 255.0f);
			converted[i * 4 + 1] = static_cast<unsigned char>(glm::clamp(fpixels[i * 3 + 1], 0.0f, 1.0f) * 255.0f);
			converted[i * 4 + 2] = static_cast<unsigned char>(glm::clamp(fpixels[i * 3 + 2], 0.0f, 1.0f) * 255.0f);
			converted[i * 4 + 3] = 255;
		}
		source = converted.data();
	}

	unsigned int texture = renderer->createTextures();
	renderer->bindTexture(texture);
	renderer->texImage2D(GL_TEXTURE_2D, GL_RGBA, faceWidth, faceHeight, GL_RGBA, GL_UNSIGNED_BYTE, source);
	renderer->setWrappingAndFiltering(GL_TEXTURE_2D, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR);

	return texture;
}

void gSkybox::drawVulkan() {
	if(id == GL_NONE) return;

	const glm::vec3 c = renderer->getCameraPosition();
	const float s = 200.0f;
	const glm::vec3 corners[8] = {
		c + glm::vec3(-s, -s, -s), c + glm::vec3(s, -s, -s), c + glm::vec3(s, s, -s), c + glm::vec3(-s, s, -s),
		c + glm::vec3(-s, -s, s), c + glm::vec3(s, -s, s), c + glm::vec3(s, s, s), c + glm::vec3(-s, s, s)
	};

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
	renderer->enableDepthTestEqual();
	skyboxshader->use();
	skyboxshader->setInt("aIsHDR", ishdr);
	skyboxshader->setMat4("projection", renderer->getProjectionMatrix());
	skyboxshader->setMat4("view", renderer->getViewMatrix());
	skyboxshader->setMat4("model", localtransformationmatrix.back());

	renderer->bindSkyTexture(id, skymapslot);
	vbo.bind();
	renderer->drawElements(GL_TRIANGLES, vbo.getIndicesNum());
	vbo.unbind();
	renderer->unbindSkyTexture();
}

void gSkybox::setupRenderData() {
	float vertexdata[]= {
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 0.0f
    };

	float normaldata[] = {
		-1.0f,  1.0f, -1.0f,
	     1.0f,  1.0f, -1.0f,
	    -1.0f, -1.0f, -1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f,  1.0f,
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f,  1.0f,
	    -1.0f,  1.0f, -1.0f,
	    -1.0f, -1.0f, -1.0f,
	    -1.0f, -1.0f,  1.0f,
	    -1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f, -1.0f,
	     1.0f, -1.0f, -1.0f,
	     1.0f, -1.0f,  1.0f,
	     1.0f,  1.0f,  1.0f,
	    -1.0f, -1.0f, -1.0f,
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f, -1.0f,
	    -1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f, -1.0f
	};

	gIndex indexdata[] = {
		 0,  2, 3,  0,  1, 3,
		 4,  6, 7,  4,  5, 7,
		 8,  9, 10, 11,  8, 10,
		12, 13, 14, 15, 12, 14,
		16, 17, 18, 16, 19, 18,
		20, 21, 22, 20, 23, 22
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
    if (cubeVAO == 0) {
        float vertices[] = {
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f
        };
    	cubeVAO = renderer->createVAO();
    	cubeVBO = renderer->genBuffers();
    	renderer->setVertexBufferData(cubeVBO, sizeof(vertices), vertices, GL_STATIC_DRAW);
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
