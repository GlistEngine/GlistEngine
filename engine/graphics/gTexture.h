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

	virtual unsigned int load(std::string fullPath);
	unsigned int loadTexture(std::string texturePath);

	unsigned int loadData(unsigned char* textureData, int width, int height, int componentNum);

    void bind();
    void bind(int textureSlotNo);
    void unbind();

    unsigned int getId();
    bool isHDR();
    unsigned int getInternalFormat();
    unsigned int getFormat();
    void setType(int textureType);
    int getType();
    void setWrapping(int wrapS, int wrapT);
    void setFiltering(int minFilter, int magFilter);
    int getWrapS();
    int getWrapT();
    int getFilterMin();
    int getFilterMag();
    std::string getTypeName();
    std::string getTypeName(int textureType);
    std::string getFilename();
    std::string getDir();
    std::string getFullPath();

    int getWidth();
    int getHeight();
    int getComponentNum();

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

protected:
    std::string fullpath, directory;
    unsigned int id, internalformat, format;
    int type;
    std::string path;
    int width, height, componentnum;
    unsigned char* data;
    bool ismutable;
    int wraps, wrapt, filtermin, filtermag;

    void setData(unsigned char* textureData, bool isMutable = false);
    unsigned char* getData();
    bool isMutable();

    void setupRenderData();

	std::string getDirName(const std::string& fname);
	std::string getFileName(const std::string& fname);

    bool ishdr;
    float* datahdr;
    void setDataHDR(float* textureData, bool isMutable = false);
    float* getDataHDR();

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
