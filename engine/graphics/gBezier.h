/*
 * gBezier.h
 *
 *  Created On: August 7, 2026
 *  Author: Ahmet Kahraman
 *
 */
#ifndef GRAPHICS_GBEZIER_H_
#define GRAPHICS_GBEZIER_H_

#include "gRenderObject.h"
#include <glm/glm.hpp>

class gBezier : public gRenderObject {
public:
	gBezier();
	gBezier(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	virtual ~gBezier();

	void setPoint(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);
	glm::vec2 getPoint(float t) const;
	void setResolution(int res);
	void draw() const;
private:
	glm::vec2 p0, p1, p2, p3;
	int resolution;
};

#endif /* GRAPHICS_GBEZIER_H_ */
