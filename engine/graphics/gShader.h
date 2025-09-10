/*
 * gShader.h
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GSHADER_H_
#define ENGINE_GRAPHICS_GSHADER_H_

#include "gRenderer.h"
#include <string>
#include <unordered_map>
#include "gRenderObject.h"

template<typename T>
class gUbo;

class gShader : public gRenderObject {
public:
	gShader();
	gShader(const std::string& shaderFullPath);
	gShader(const std::string& vertexFullPath, const std::string& fragmentFullPath, const std::string& geometryFullPath = "");
	~gShader();

	void load(const std::string& shaderFullPath);
	void load(const std::string& vertexFullPath, const std::string& fragmentFullPath, const std::string& geometryFullPath = "");

	void loadShader(const std::string& shaderFilename);
	void loadShader(const std::string& vertexFileName, const std::string& fragmentFileName, const std::string& geometryFileName = "");

	void loadProgram(const std::string& vertexSource, const std::string& fragmentSource, const std::string& geometrySource = "");
	void loadProgram(const std::string& shaderSource);

	template<typename T>
	void attachUbo(const std::string& uboName, const gUbo<T>* ubo) {
		ubos[uboName] = ubo->getBindingPoint();
		use();
		renderer->attachUbo(id, ubo->getBindingPoint(), uboName);
	}

	bool loaded;
	GLuint id;

    void use() const;
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setUnsignedInt(const std::string &name, unsigned int value);
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, const glm::vec2 &value);
    void setVec2(const std::string &name, float x, float y);
    void setVec3(const std::string &name, const glm::vec3 &value);
    void setVec3(const std::string &name, float x, float y, float z);
    void setVec4(const std::string &name, const glm::vec4 &value);
    void setVec4(const std::string &name, float x, float y, float z, float w);
    void setMat2(const std::string &name, const glm::mat2 &mat);
    void setMat3(const std::string &name, const glm::mat3 &mat);
    void setMat4(const std::string &name, const glm::mat4 &mat);

	GLint getUniformLocation(const std::string& name);

private:
	enum class ShaderType {
		VERTEX,
		FRAGMENT,
		GEOMETRY
	};

	void loadProgramInternal(const char* vertexSource, const char* fragmentSource, const char* geometrySource);

	void checkCompileErrors(GLuint shader, const std::string& type);
	std::string preprocessShader(const std::string& shaderCode, std::unordered_map<std::string, std::string> defines);
	std::unordered_map<std::string, std::string> generateDefines(ShaderType type);

	std::unordered_map<std::string, GLint> uniformlocations;
	std::unordered_map<std::string, GLuint> ubos;
};

#endif /* ENGINE_GRAPHICS_GSHADER_H_ */
