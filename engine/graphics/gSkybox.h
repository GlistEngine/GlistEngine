/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRAPHICS_GSKYBOX_H_
#define GRAPHICS_GSKYBOX_H_

#include <array>
#include <string>
#include "gNode.h"
#include "gVbo.h"
#include "gImage.h"

/**
 * gSkybox creates a 6-sided textured sky box around the scene.
 */
class gSkybox : public gNode {
public:
	gSkybox();
	virtual ~gSkybox();

	/**
	 * Loads skybox textures from the assets/textures folder. 6 textures needed for the 6 sided skybox. Due to OpenGL instructions,
	 * the textures should be in this order: right, left, top, bottom, front, back.
	 */
	unsigned int loadTextures(const std::vector<std::string>& texturePaths);

	/**
	 * Loads skybox textures from the given full paths. 6 textures needed for the 6 sided skybox. Due to OpenGL instructions,
	 * the textures should be in this order: right, left, top, bottom, front, back.
	 */
	unsigned int load(const std::vector<std::string>& fullPaths);

	unsigned int loadEquirectangular(const std::string& fullPath);
	unsigned int loadTextureEquirectangular(const std::string& texturePath);

	void loadFromData(std::array<int, 6> widths, std::array<int, 6> heights, std::array<void*, 6> rawdata, std::array<bool, 6> ishdr);

	/**
	 * Draws the skybox back the scene. Drawing the skybox after drawing all scene objects and disabling lights would give the most
	 * native visual result.
	 */
	void draw();

	void generatePbrMaps();
	void bindPbrMaps();

	void loadSkybox(gImage* images);
	void loadDataSkybox(std::string* data, int width, int height);
private:
	unsigned int id;
	// The six faces as ordinary 2D textures, for the Vulkan path. That backend has
	// no cube map here, so the sky is drawn as six quads instead and each one needs
	// a texture of its own. Zero where a face never loaded.
	std::array<unsigned int, 6> vkfaceids{};
	int width, height, nrChannels;
	void setupRenderData();
	// Uploads one face through the renderer's plain 2D texture path. hdr converts
	// float source pixels down to bytes first.
	unsigned int uploadVulkanFace(int faceWidth, int faceHeight, void* pixels, bool hdr = false);
	void drawVulkan();

	unsigned int cubeVAO, cubeVBO;
	unsigned int skyboxVAO, skyboxVBO;

	gShader* skyboxshader;

	gVbo vbo;
	int skymapint, skymapslot;

	//Equirectangular variables and objects
	gShader* pbrShader;
	gShader* equirectangularToCubemapShader;
	gShader* irradianceShader;
	gShader* prefilterShader;
	gShader* brdfShader;

	unsigned int captureFBO;
	unsigned int captureRBO;
	unsigned int irradianceMap;
	unsigned int prefilterMap;
	unsigned int brdfLUTTexture;
	unsigned int quadVAO;
	unsigned int quadVBO;

	void renderCube();
	void renderQuad();

	bool ishdr, ispbr;
};

#endif /* GRAPHICS_GSKYBOX_H_ */
