/*
 * gParticles.cpp
 *
 *  Created on: 22 Aug 2022
 *      Author: noyan
 */

#include "gBaseParticles.h"


gBaseParticles::gBaseParticles() {
	m_bLimit = 0;
	m_bEngineActive = 0;
	m_nAge = 0;
	imagepath = "";
//	strcpy(m_szImgPath, "particle.bmp");
}

gBaseParticles::~gBaseParticles() {
	if(m_bEngineActive) {
		destroy();
	}
	m_bEngineActive = 0;
}

void gBaseParticles::loadTexture(std::string texturePath) {
	g = new gTexture();
	g->loadTexture(texturePath);
	gp.setTexture(g);
}

void gBaseParticles::setTexture(gTexture* texture) {
	g = texture;
	gp.setTexture(g);
}

void gBaseParticles::init(int particleNum) {
	m_bEngineActive = 1;
	particle = new gParticle[particleNum];
	particlenum = particleNum;


	int particleno = 0;
	while(particleno < particleNum) {
		particle[particleno].pengine = this;
		particle[particleno].particleno = particleno;
		resetParticle(particleno);
		particleno++;
	}
}


void gBaseParticles::draw() {
	particleno = 0;
	previousalpha = renderer->isAlphaBlendingEnabled();
	if(!previousalpha) renderer->enableAlphaBlending();
	while(particleno != particlenum) {
		particle[particleno].draw();
		particle[particleno].advance();
		if(particle[particleno].life < 0.0f) resetParticle(particleno);
		particleno++;
	}
	if(!previousalpha) renderer->disableAlphaBlending();

	if(m_bLimit) {
		particleno = 0;
		while(particleno != particlenum) {
			if(particle[particleno].topleft.x < m_fLimitL) {
				resetParticle(particleno);
			} else if(particle[particleno].bottomright.x > m_fLimitR) {
				resetParticle(particleno);
			} else if(particle[particleno].bottomright.y < m_fLimitB) {
				resetParticle(particleno);
			} else if(particle[particleno].topleft.y > m_fLimitT) {
				resetParticle(particleno);
			}

			particleno++;
		}
	}
	m_nAge++;

//	particleno = 0;
}

void gBaseParticles::destroy() {
	m_nAge = 0;
	delete[] particle;
	delete g;
}


void gBaseParticles::resetParticle(int particleNo) {
}


void gBaseParticles::setCurrentColor(int nParticle, float fRed, float fGreen, float fBlue, float fAlpha) {
	particle[nParticle].currentcolor.r = fRed;
	particle[nParticle].currentcolor.g = fGreen;
	particle[nParticle].currentcolor.b = fBlue;
	particle[nParticle].currentcolor.a = fAlpha;
}


void gBaseParticles::setImgPath(std::string szImgPath) {
	imagepath = szImgPath;
}

void gBaseParticles::setInitialColor(float fRed, float fGreen, float fBlue, float fAlpha) {
	m_clrParticleColor.a = fAlpha;
	m_clrParticleColor.b = fBlue;
	m_clrParticleColor.g = fGreen;
	m_clrParticleColor.r = fRed;
}

void gBaseParticles::setFadeColor(int nParticle, float fRed, float fGreen, float fBlue, float fAlpha) {
	particle[nParticle].fadecolor.r = fRed;
	particle[nParticle].fadecolor.g = fGreen;
	particle[nParticle].fadecolor.b = fBlue;
	particle[nParticle].fadecolor.a = fAlpha;
}


void gBaseParticles::setDirection(int nParticle, float fX, float fY, float fZ) {
	particle[nParticle].direction.x = fX;
	particle[nParticle].direction.y = fY;
	particle[nParticle].direction.z = fZ;
}

void gBaseParticles::setGravity(int nParticle, float fX, float fY, float fZ) {
	particle[nParticle].gravity.x = fX;
	particle[nParticle].gravity.y = fY;
	particle[nParticle].gravity.z = fZ;
}

void gBaseParticles::setLR(int nParticle, float fX, float fY, float fZ) {
	particle[nParticle].bottomright.x = fX;
	particle[nParticle].bottomright.y = fY;
	particle[nParticle].bottomright.z = fZ;
}

void gBaseParticles::setUL(int nParticle, float fX, float fY, float fZ) {
	particle[nParticle].topleft.x = fX;
	particle[nParticle].topleft.y = fY;
	particle[nParticle].topleft.z = fZ;
}

void gBaseParticles::setAge(int nParticle, float fAge) {
	particle[nParticle].age = fAge;
}

void gBaseParticles::setFrame(int nParticle, int nFrame) {
	particle[nParticle].frameno = nFrame;
}

void gBaseParticles::setGravityFactor(int nParticle, float fGravityFactor) {
	particle[nParticle].gravityfactor = fGravityFactor;
}

void gBaseParticles::setLife(int nParticle, float fLife) {
	particle[nParticle].life = fLife;
}


void gBaseParticles::setFadeLife(int nParticle, float fFadeLife) {
	particle[nParticle].fadelife = fFadeLife;
}

void gBaseParticles::setLimit(bool bLimit) {
	m_bLimit = bLimit;
}

void gBaseParticles::setLimit(float fLimitL, float fLimitR, float fLimitB, float fLimitT) {
	m_bLimit = 1;

	m_fLimitL = fLimitL;
	m_fLimitR = fLimitR;
	m_fLimitT = fLimitT;
	m_fLimitB = fLimitB;
}


/*  The Particle Object */
gParticle::gParticle() {
	//Assumed values if none are initiated
	age = 0;
	frameno = 0;
	life = 1.0f;
	gravityfactor=.025f;
}


//Draws the particle to the screen
void gParticle::draw() {
	//Set color
	renderer->setColor(currentcolor.r, currentcolor.g, currentcolor.b, currentcolor.a);

	//Dont display particle on first frame of it's life
	if(life == 1.0f) {
		renderer->setColor(0, 0, 0, 0);
	}

	//draw the square
	pengine->gp.setPosition(topleft.x, topleft.y, topleft.z);
	pengine->gp.draw();
}


void gParticle::advance() {

	//Move the quad in the direction it should be headed
	topleft.x  += direction.x;
	topleft.y  += direction.y;
	topleft.z  += direction.z;

	bottomright.x  += direction.x;
	bottomright.y  += direction.y;
	bottomright.z  += direction.z;

	//Add the force of gravity
	//The formula for the force of gravity (without factoring friction from
	//the air) is something like:  h = vit + 1/2 a t^2
	topleft.x += gravity.x * age *age;
	topleft.y += gravity.y* age *age;
	topleft.z += gravity.z* age *age;

	bottomright.x += gravity.x* age *age;
	bottomright.y += gravity.y* age *age;
	bottomright.z += gravity.z* age *age;

	//Slowly fade the color
	currentcolor.r+= fadecolor.r;
	currentcolor.g+= fadecolor.g;
	currentcolor.b+= fadecolor.b;
	currentcolor.a+= fadecolor.a;

	life+= fadelife;

	frameno++;
	age = frameno * gravityfactor;

}

