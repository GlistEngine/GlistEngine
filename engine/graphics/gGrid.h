/*
 * gGrid.h
 *
 *  Created on: Jul 11, 2024
 *      Author: usif
 */

#ifndef GRAPHICS_GGRID_H_
#define GRAPHICS_GGRID_H_

#include "gRenderObject.h"
#include "gVbo.h"
#include "gShader.h"
#include "gCamera.h"

class gGrid : public gRenderObject {
public:
	static const int XY = 0, YZ = 1, XZ = 2;
	gGrid();
	virtual ~gGrid();

	void draw();
	void clear();

	void drawYZ();
	void drawXY();
	void drawXZ();

	void enable();
	void disable();
	bool isEnabled() const;

	void setEnableGrid(bool xy, bool yz, bool xz);
	void setEnableYZ(bool yz);
	void setEnableXY(bool xy);
	void setEnableXZ(bool xz);
	bool isYZEnabled() const;
	bool isXYEnabled() const;
	bool isXZEnabled() const;

	void drawAxisX();
	void drawAxisY();
	void drawAxisZ();

	void setEnableAxis(bool x, bool y, bool z);
	void setEnableAxisX(bool x);
	void setEnableAxisY(bool y);
	void setEnableAxisZ(bool z);
	bool isAxisXEnabled() const;
	bool isAxisYEnabled() const;
	bool isAxisZEnabled() const;

	void drawWireFrameYZ();
	void drawWireFrameXY();
	void drawWireFrameXZ();

	void setEnableWireFrame(bool xy, bool yz, bool xz);
	void setEnableWireFrameYZ(bool yz);
	void setEnableWireFrameXY(bool xy);
	void setEnableWireFrameXZ(bool xz);
	bool isWireFrameYZEnabled() const;
	bool isWireFrameXYEnabled() const;
	bool isWireFrameXZEnabled() const;

	void setColorAxisX(int r, int g, int b, int a);
	void setColorAxisY(int r, int g, int b, int a);
	void setColorAxisZ(int r, int g, int b, int a);
	void setColorAxisX(gColor* color);
	void setColorAxisY(gColor* color);
	void setColorAxisZ(gColor* color);

	void setColorWireFrameYZ(int r, int g, int b, int a);
	void setColorWireFrameXY(int r, int g, int b, int a);
	void setColorWireFrameXZ(int r, int g, int b, int a);
	void setColorWireFrameYZ(gColor* color);
	void setColorWireFrameXY(gColor* color);
	void setColorWireFrameXZ(gColor* color);

	void enableAutoClip();
	void disableAutoClip();
	bool isAutoClipEnabled() const;

	void setNearClip(float near);
	void setFarClip(float far);

	float getNearClip() const;
	float getFarClip() const;

	void setLineSpacing(float spacing);
	float getLineSpacing();

private:

    void drawStart();
    void drawVbo();
    void drawEnd();

	gVbo vbo;
    gShader *gridshader;
    const gCamera *camera;
	std::vector<float> gridvertices;
	float spacing;
	float farclip;
	float nearclip;
	bool isautoclipenabled;

	bool isenabled;
	bool isgridxyenabled;
	bool isgridyzenabled;
	bool isgridxzenabled;
	bool isaxisxenabled;
	bool isaxisyenabled;
	bool isaxiszenabled;
	bool iswireframexyenabled;
	bool iswireframeyzenabled;
	bool iswireframexzenabled;

	gColor coloraxisx;
	gColor coloraxisy;
	gColor coloraxisz;
	gColor colorwireframexy;
	gColor colorwireframeyz;
	gColor colorwireframexz;

};

#endif /* GRAPHICS_GGRID_H_ */
