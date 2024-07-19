/*
 * gShader.h
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#ifndef ENGINE_GRAPHICS_GSHADER_H_
#define ENGINE_GRAPHICS_GSHADER_H_

#include "gObject.h"
#include "gRenderer.h"
#include <string>
#include <unordered_map>

template<typename T>
class gUbo;

class gShader : public gObject {
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
		unsigned int blockIndex;
		G_CHECK_GL2(blockIndex, glGetUniformBlockIndex(id, uboName.c_str()));
		G_CHECK_GL(glUniformBlockBinding(id, blockIndex, ubo->getBindingPoint()));
		/*if (blockIndex != GL_INVALID_INDEX) {
			GLint blockSize;
			G_CHECK_GL(glGetActiveUniformBlockiv(id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize));

			GLint blockAlignment;
			G_CHECK_GL(glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &blockAlignment));

			std::cout << "Uniform block alignment: " << blockAlignment << " bytes" << std::endl;

			// Now 'blockSize' contains the size of the uniform block in bytes
			std::cout << "Uniform block size: " << blockSize << " bytes" << std::endl;
			if (ubo->getSize() != blockSize) {
				std::cout << "Error: Uniform block size mismatch. actual size: " << ubo->getSize() << ", block size: " << blockSize << std::endl;
			}
		} else {
			std::cerr << "Error: Unable to find uniform block index." << std::endl;
		}*/
	}

	bool loaded;
	GLuint id;

    void use() const;
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
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
