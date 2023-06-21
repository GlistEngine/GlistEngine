/*
 * gTexture.h
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#pragma once

#ifndef ENGINE_GRAPHICS_GTEXTURE_H_
#define ENGINE_GRAPHICS_GTEXTURE_H_

#include <string>
#include "gRenderObject.h"
#include "gRect.h"

class gTexture : public gRenderObject {
public:
	static const int TEXTURETYPE_DIFFUSE, TEXTURETYPE_SPECULAR, TEXTURETYPE_NORMAL, TEXTURETYPE_HEIGHT;
	static const int TEXTURETYPE_PBR_ALBEDO, TEXTURETYPE_PBR_ROUGHNESS, TEXTURETYPE_PBR_METALNESS, TEXTURETYPE_PBR_NORMAL, TEXTURETYPE_PBR_AO;
	static const int TEXTUREWRAP_REPEAT, TEXTUREWRAP_CLAMP, TEXTUREWRAP_CLAMPTOEDGE;
	static const int TEXTUREMINMAGFILTER_LINEAR, TEXTUREMINMAGFILTER_MIPMAPLINEAR, TEXTUREMINMAGFILTER_NEAREST;

	gTexture();
	gTexture(int w, int h, int format = GL_RGBA, bool isFbo = false);
	virtual ~gTexture();

	virtual unsigned int load(const std::string& fullPath);
	unsigned int loadTexture(const std::string& texturePath);

	/** Loads the mask texture/image to be applied to this instance.
	 *
	 * It requires full path of the texture/image file.
	 *
	 * The texture/image might be stored anywhere on the hard disk or other storage
	 * devices out of the project folder.
	 *
	 * @param fullPath The full path to the image or texture file. It should contain the
	 * full path of the folder where the image is located, the name of the image
	 * and its extension. It is case sensitive.
	 *
	 * @return Image id number generated by OpenGL.
	 */
	unsigned int loadMask(const std::string& fullPath);

	/** Loads the mask texture/image to be applied to this instance.
	 *
	 * It requires the name of the texture/image file (ex: "mask.png").
	 *
	 * The image might be stored anywhere on the hard disk or other storage
	 * devices out of the project folder.
	 *
	 * Project's default folder for masks is ProjectLocation/assets/textures directory.
	 *
	 * @param maskTexturePath The fullname of the texture/image stored under project's images folder.
	 * maskTexturePath should contain texture/image name and its extension. It is case sensitive.
	 *
	 * @return Image id number generated by OpenGL.
	 */
	unsigned int loadMaskTexture(const std::string& maskTexturePath);

	unsigned int loadData(unsigned char* textureData, int width, int height, int componentNum, bool isFont = false);

    void bind() const;
    void bind(int textureSlotNo) const;
    void unbind() const;

    unsigned int getId() const;
    bool isHDR() const;
    unsigned int getInternalFormat() const;
    unsigned int getFormat() const;
    void setType(int textureType);
    int getType() const;
    void setWrapping(int wrapS, int wrapT);
    void setFiltering(int minFilter, int magFilter);
    int getWrapS() const;
    int getWrapT() const;
    int getFilterMin() const;
    int getFilterMag() const;
    const std::string& getTypeName() const;
    const std::string& getTypeName(int textureType) const;
    const std::string& getFilename() const;
    const std::string& getDir() const;
    const std::string& getFullPath() const;

    int getWidth() const;
    int getHeight() const;
    int getComponentNum() const;

    void draw(int x, int y);
    void draw(int x, int y, int w, int h);
    void draw(int x, int y, int w, int h, float rotate);
    void draw(int x, int y, int w, int h, int pivotx, int pivoty, float rotate);
    void draw(glm::vec2 position, glm::vec2 size, float rotate = 0.0f);
    void draw(glm::vec2 position, glm::vec2 size, glm::vec2 pivotPointCoords, float rotate = 0.0f);

    void drawSub(int x, int y, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, int pivotx, int pivoty, float rotate);
    void drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate = 0.0f);
    void drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, glm::vec2 pivotPointCoords, float rotate = 0.0f);
    void drawSub(const gRect& src, const gRect& dst, float rotate = 0.f);
    void drawSub(const gRect& src, const gRect& dst, int pivotx, int pivoty, float rotate = 0.f);
    void drawSub(const gRect& src, const gRect& dst, glm::vec2 pivotPointCoords, float rotate = 0.f);

    void setData(unsigned char* textureData, bool isMutable = false);

    void setupRenderData();

protected:
    std::string fullpath, directory;
    unsigned int id, internalformat, format;
    int type;
    std::string path;
    int width, height, componentnum;
    unsigned char* data;
    bool ismutable;
    int wraps, wrapt, filtermin, filtermag;
    bool isfont;

    unsigned char* getData();
    bool isMutable();

	std::string getDirName(const std::string& fname);
	std::string getFileName(const std::string& fname);

    bool ishdr;
    float* datahdr;
    void setDataHDR(float* textureData, bool isMutable = false);
    float* getDataHDR();
    bool ismaskloaded;
    gTexture* masktexture;

private:
    std::string texturetype[4];

    unsigned int quadVAO, quadVBO;
    glm::mat4 imagematrix;
    void setupRenderData(int sx, int sy, int sw, int sh);
    void beginDraw();
    void endDraw();
    bool bsubpartdrawn;
    bool isfbo;
};

#endif /* ENGINE_GRAPHICS_GTEXTURE_H_ */