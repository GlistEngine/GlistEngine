/*
 * gSkybox.cpp
 *
 *  Created on: 18 Mar 2021
 *      Author: Acer
 */

#include "gSkybox.h"
//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#endif
#include "stb/stb_image.h"


gSkybox::gSkybox() {
	id = GL_NONE;

	width = 0;
	height = 0;
	nrChannels = 0;
	scale(200);
	setupRenderData();
}

gSkybox::~gSkybox() {}

unsigned int gSkybox::loadTextures(std::vector<std::string> texturePaths) {
	for (int i = 0; i < texturePaths.size(); i++) {
		texturePaths[i] = gGetTexturesDir() + texturePaths[i];
	}
	return load(texturePaths);
}

unsigned int gSkybox::load(std::vector<std::string> fullPaths) {
	skymapslot = GL_TEXTURE0;
	skymapint = 0;
	glActiveTexture(skymapslot);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	for (unsigned int i = 0; i < fullPaths.size(); i++) {
        unsigned char *data = stbi_load(fullPaths[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap tex failed to load at path: " << fullPaths[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	renderer->getSkyboxShader()->use();
	renderer->getSkyboxShader()->setInt("skymap", skymapint);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glActiveTexture(GL_TEXTURE0);

    return id;
}

void gSkybox::draw() {
	skyboxshader = renderer->getSkyboxShader();
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxshader->use();
	skyboxshader->setMat4("projection", renderer->getProjectionMatrix());
	skyboxshader->setMat4("view", renderer->getViewMatrix());
	skyboxshader->setMat4("model", localtransformationmatrix);

//	skyboxshader->setInt("skymap", skymapint);

	glActiveTexture(skymapslot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    vbo.bind();
    glDrawElements(GL_TRIANGLES, vbo.getIndicesNum(), GL_UNSIGNED_INT, 0);
    vbo.unbind();
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

/*
	// skybox cube
	glActiveTexture(skymapslot);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glActiveTexture(0);
//	glDepthFunc(GL_LESS); // set depth function back to default
*/
}

void gSkybox::setupRenderData() {

	float vertexdata[]= {
	    // x,   y,   z,  s,  t,
		-1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Back
		 1.0f,  1.0f, -1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f, // Front
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 1.0f, // Left
		-1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
		-1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		-1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 1.0f, // Right
		 1.0f, -1.0f, -1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, -1.0f, 0.0f, 1.0f, // Top
		-1.0f, -1.0f,  1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 1.0f, 0.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, 1.0f,
		-1.0f,  1.0f, -1.0f, 0.0f, 0.0f, // Bottom
		-1.0f,  1.0f,  1.0f, 0.0f, 1.0f,
		 1.0f,  1.0f,  1.0f, 1.0f, 1.0f,
		 1.0f,  1.0f, -1.0f, 1.0f, 0.0f
    };

	float normaldata[] = {
		-1.0f,  1.0f, -1.0f, // Back
	     1.0f,  1.0f, -1.0f,
	    -1.0f, -1.0f, -1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f,  1.0f, // Front
	     1.0f,  1.0f,  1.0f,
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f,  1.0f,
	    -1.0f,  1.0f, -1.0f, // Left
	    -1.0f,  -1.0f,  -1.0f,
	    -1.0f, -1.0f, 1.0f,
	    -1.0f, 1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f, // Right
	     1.0f,  -1.0f, -1.0f,
	     1.0f, -1.0f,  1.0f,
	     1.0f, 1.0f, 1.0f,
	    -1.0f, -1.0f,  -1.0f, // Top
	    -1.0f, -1.0f,  1.0f,
	     1.0f, -1.0f, 1.0f,
	     1.0f, -1.0f, -1.0f,
	    -1.0f,  1.0f, -1.0f, // Bottom
	    -1.0f,  1.0f, 1.0f,
	     1.0f,  1.0f,  1.0f,
	     1.0f,  1.0f,  -1.0f
	};


	unsigned int indexdata[] = {
	   // back
		 0,  2, 3,
		 0,  1, 3,
	   // front
		 4,  6, 7,
		 4,  5, 7,
	   // left
		 8,  9, 10,
		11,  8, 10,
	   // right
		12, 13, 14,
		15, 12, 14,
	   // top
		16, 17, 18,
		16, 19, 18,
	   // bottom
		20, 21, 22,
		20, 23, 22
	};


	int nv = (sizeof(vertexdata) / sizeof(vertexdata[0])) / 5;
	std::vector<gVertex> verticesb;
	for (int i=0; i<nv; i++) {
		gVertex v;
		v.position.x = vertexdata[(i * 5)];
		v.position.y = vertexdata[(i * 5) + 1];
		v.position.z = vertexdata[(i * 5) + 2];
		v.texcoords.x = vertexdata[(i * 5) + 3];
		v.texcoords.y = vertexdata[(i * 5) + 4];
		v.normal.x = normaldata[(i * 3)];
		v.normal.y = normaldata[(i * 3) + 1];
		v.normal.z = normaldata[(i * 3) + 2];
		verticesb.push_back(v);
	}

	int ni = sizeof(indexdata) / sizeof(indexdata[0]);
	std::vector<unsigned int> indicesb;
	for (int i=0; i<ni; i++) {
		indicesb.push_back(indexdata[i]);
	}

	vbo.setVertexData(&verticesb[0], sizeof(gVertex), verticesb.size());
	if (indicesb.size() != 0) vbo.setIndexData(&indicesb[0], indicesb.size());

/*
	    float skyboxVertices[] = {
	        // positions
	        -1.0f,  1.0f, -1.0f,
	        -1.0f, -1.0f, -1.0f,
	         1.0f, -1.0f, -1.0f,
	         1.0f, -1.0f, -1.0f,
	         1.0f,  1.0f, -1.0f,
	        -1.0f,  1.0f, -1.0f,

	        -1.0f, -1.0f,  1.0f,
	        -1.0f, -1.0f, -1.0f,
	        -1.0f,  1.0f, -1.0f,
	        -1.0f,  1.0f, -1.0f,
	        -1.0f,  1.0f,  1.0f,
	        -1.0f, -1.0f,  1.0f,

	         1.0f, -1.0f, -1.0f,
	         1.0f, -1.0f,  1.0f,
	         1.0f,  1.0f,  1.0f,
	         1.0f,  1.0f,  1.0f,
	         1.0f,  1.0f, -1.0f,
	         1.0f, -1.0f, -1.0f,

	        -1.0f, -1.0f,  1.0f,
	        -1.0f,  1.0f,  1.0f,
	         1.0f,  1.0f,  1.0f,
	         1.0f,  1.0f,  1.0f,
	         1.0f, -1.0f,  1.0f,
	        -1.0f, -1.0f,  1.0f,

	        -1.0f,  1.0f, -1.0f,
	         1.0f,  1.0f, -1.0f,
	         1.0f,  1.0f,  1.0f,
	         1.0f,  1.0f,  1.0f,
	        -1.0f,  1.0f,  1.0f,
	        -1.0f,  1.0f, -1.0f,

	        -1.0f, -1.0f, -1.0f,
	        -1.0f, -1.0f,  1.0f,
	         1.0f, -1.0f, -1.0f,
	         1.0f, -1.0f, -1.0f,
	        -1.0f, -1.0f,  1.0f,
	         1.0f, -1.0f,  1.0f
	    };

	    // skybox VAO
	    glGenVertexArrays(1, &skyboxVAO);
	    glGenBuffers(1, &skyboxVBO);
	    glBindVertexArray(skyboxVAO);
	    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	    glEnableVertexAttribArray(0);
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
*/
}
