/*
 * gPolygon.h
 *
 * Created on: Aug 19, 2026
 * Author: merve
 */

#ifndef GRAPHICS_PRIMITIVES_GPOLYGON_H_
#define GRAPHICS_PRIMITIVES_GPOLYGON_H_

#include "gMesh.h"

class gPolygon : public gMesh {
public:
    gPolygon();
    virtual ~gPolygon();

    void addPoint(float x, float y);
    void addPoint(const glm::vec2& point);

    void endPolygon();
    void clearPoints();

    void setFilled(bool isFilled);
    bool isFilled() const;

    void setColor(const gColor& color);

    size_t getPointCount() const;

    void draw() override;
    void draw(float x, float y);

private:
    void updateMeshBuffers();
    bool isfilled;
};

#endif /* GRAPHICS_PRIMITIVES_GPOLYGON_H_ */
