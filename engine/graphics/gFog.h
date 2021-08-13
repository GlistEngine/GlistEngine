/*
 * gFog.h
 *
 *  Created on: 12 Aðu 2021
 *      Author: furka
 */

#ifndef GRAPHICS_GFOG_H_
#define GRAPHICS_GFOG_H_

#include <gRenderObject.h>
#include <gColor.h>



class gFog : public gRenderObject{
public:
	gFog();
	virtual ~gFog();

	void enable();
	void disable();
	void setColor(float r, float g, float b);
	void setDensity(float d);
	void setGradient(float g);
	gColor getColor();
	float getDensity();
	float getGradient();

private:

};



#endif /* GRAPHICS_GFOG_H_ */
