/*
 * gParticles.h
 *
 *  Created on: 22 Aug 2022
 *      Author: noyan
 */

#ifndef SRC_GPARTICLES_H_
#define SRC_GPARTICLES_H_

#include "gRenderObject.h"
#include "gPlane.h"


class gParticle : public gRenderObject {
public:
	gColor currentcolor;	//Current color of the particle
	gColor fadecolor;		//Increment the color by

	glm::vec3 direction; //Change the position by
	glm::vec3 gravity;	//Change the position by

	glm::vec3 topleft;		//Upper Left corner to the square
	glm::vec3 bottomright;		//Lower right corner of the square

	int frameno;			//How many times has the particle been displayed
	float age;			//How many "seconds" elapsed (for gravity calculations)

	float life;
	float gravityfactor;
	float fadelife;
	int particleno;

public:
	gParticle();
	void draw();
	void advance();

	class gBaseParticles* pengine;
};


class gBaseParticles : public gRenderObject {
public:
	gBaseParticles();
	virtual ~gBaseParticles();

	virtual void init(int particleNum);
	virtual void resetParticle(int particleNo);

	void loadTexture(std::string texturePath);
	void setTexture(gTexture* texture);
	void draw();
	void destroy();

	void setLimit(float fLimitL, float fLimitR, float fLimitT, float fLimitB);
	void setLimit(bool bLimit);
	void setCurrentColor(int nParticle, float fRed, float fGreen, float fBlue, float fAlpha);
	void setFadeColor(int nParticle, float fRed, float fGreen, float fBlue, float fAlpha);
	void setDirection(int nParticle, float fX, float fY, float fZ);
	void setGravity(int nParticle, float fX, float fY, float fZ);
	void setLR(int nParticle, float fX, float fY, float fZ);
	void setUL(int nParticle, float fX, float fY, float fZ);
	void setAge(int nParticle, float fAge);
	void setFrame(int nParticle, int nFrame);
	void setGravityFactor(int nParticle, float fGravityFactor);
	void setLife(int nParticle, float fLife);
	void setFadeLife(int nParticle, float fFadeLife);
	void setImgPath(std::string szImgPath);
	void setInitialColor(float fRed, float fGreen, float fBlue, float fAlpha);

	gPlane gp;

protected:
	bool m_bLimit;
	gParticle *particle;
	bool m_bEngineActive;
	int particlenum;
	int m_nAge;

	float m_fLimitR;
	float m_fLimitL;
	float m_fLimitT;
	float m_fLimitB;

	std::string imagepath;

	gTexture* g;
	gColor m_clrParticleColor;

	int particleno;
	bool previousalpha;
};

#endif /* SRC_GIPPARTICLES_H_ */
