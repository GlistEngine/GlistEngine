/*
 * gFbo.h
 *
 *  Created on: 23 Feb 2021
 *      Author: Noyan
 */

#ifndef GRAPHICS_GFBO_H_
#define GRAPHICS_GFBO_H_

#include "gRenderObject.h"
#include "gTexture.h"


class gFbo : public gRenderObject {
public:
	gFbo();
	virtual ~gFbo();

	void allocate(int width, int height, bool isDepthMap = false, bool useDepthTexture = false);
	unsigned int getId() const;
	int getWidth() const;
	int getHeight() const;
	unsigned int getTextureId() const;
	unsigned int getDepthTextureId() const;
	gTexture& getTexture();

	void bind();
	void unbind();

	void draw(int x, int y);
	void draw(int x, int y, int w, int h);
    void draw(int x, int y, int w, int h, float rotate);
    void draw(glm::vec2 position, glm::vec2 size, float rotate = 0.0f);

    void drawSub(int x, int y, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh);
    void drawSub(int x, int y, int w, int h, int sx, int sy, int sw, int sh, float rotate);
    void drawSub(glm::vec2 pos, glm::vec2 size, glm::vec2 subpos, glm::vec2 subsize, float rotate = 0.0f);
    void drawSub(const gRect& src, const gRect& dst, float rotate = 0.f);

    static int defaultfbo;

private:
    unsigned int framebuffer;
    unsigned int rbo;
    gTexture* texture;
    gTexture* depthtexture;
    int width, height;
    bool isdepthmap;
    bool usedepthtexture;
    bool isallocated;

};

#endif /* GRAPHICS_GFBO_H_ */
