/*
 * gShader.h
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GSHADER_H_
#define ENGINE_GRAPHICS_GSHADER_H_

#include "gObject.h"
#include <string>
#if defined(WIN32) || defined(LINUX)
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#ifdef ANDROID
#include <GLES3/gl3.h>
#endif


class gShader : public gObject {
public:
	gShader();
	gShader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	virtual ~gShader();

	void load(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	void loadProgram(const std::string vertexShaderStr, const std::string fragmentShaderStr, const std::string geometryShaderStr = "");

    GLuint id;

    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w);
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;


private:
	void checkCompileErrors(GLuint shader, std::string type);
};

#endif /* ENGINE_GRAPHICS_GSHADER_H_ */
