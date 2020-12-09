/*
 * gTexture.h
 *
 *  Created on: May 10, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GTEXTURE_H_
#define ENGINE_GRAPHICS_GTEXTURE_H_

#include <string>
#include "gRenderObject.h"


class gTexture : public gRenderObject {
public:
	static const int TEXTURETYPE_DIFFUSE, TEXTURETYPE_SPECULAR, TEXTURETYPE_NORMAL, TEXTURETYPE_HEIGHT;

	gTexture();
	virtual ~gTexture();

	virtual unsigned int load(std::string filePath);

    void bind();
    void unbind();

    unsigned int getId();
    unsigned int getFormat();
    void setType(int textureType);
    int getType();
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
    void draw(glm::vec2 position, glm::vec2 size, float rotate = 0.0f);

    void drawSub(int x, int y, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate);
    void drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate = 0.0f);

protected:
    std::string fullpath, directory;
    unsigned int id, format;
    int type;
    std::string path;
    int width, height, componentnum;
    unsigned char* data;
    bool ismutable;

    void setData(unsigned char* textureData, bool isMutable = false);
    unsigned char* getData();
    bool isMutable();

    void setupRenderData();

	std::string getDirName(const std::string& fname);
	std::string getFileName(const std::string& fname);

private:
    std::string texturetype[4];

    unsigned int quadVAO;
    glm::mat4 imagematrix;
    void setupRenderData(int sx, int sy, int sw, int sh);
    void beginDraw();
    void endDraw();
    bool bsubpartdrawn;
};

#endif /* ENGINE_GRAPHICS_GTEXTURE_H_ */
