/*
 * gShader.cpp
 *
 *  Created on: May 7, 2020
 *      Author: noyan
 */

#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include "gShader.h"
#include "gRenderer.h"


gShader::gShader() {
	id = 0;
	loaded = false;
}

gShader::~gShader() {
    if(loaded) {
        glDeleteShader(id);
    }
}

gShader::gShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath) {
	load(vertexPath, fragmentPath);
}

void gShader::loadShader(const std::string& vertexFileName, const std::string& fragmentFileName, const std::string& geometryFileName) {
	load(gGetShadersDir() + vertexFileName, gGetShadersDir() + fragmentFileName, gGetShadersDir() + geometryFileName);
}

void gShader::load(const std::string& vertexFullPath, const std::string& fragmentFullPath, const std::string& geometryFullPath) {
    if(loaded) {
        glDeleteShader(id);
    }
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::string geometryCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    std::ifstream gShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    gShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try  {
        // open files
        vShaderFile.open(vertexFullPath);
        fShaderFile.open(fragmentFullPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
        // if geometry shader path is present, also load a geometry shader
        geometryCode = "";
        if(geometryFullPath != "") {
            gShaderFile.open(geometryFullPath);
            std::stringstream gShaderStream;
            gShaderStream << gShaderFile.rdbuf();
            gShaderFile.close();
            geometryCode = gShaderStream.str();
        }
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    loadProgram(vertexCode, fragmentCode, geometryCode);
}

void gShader::loadProgram(const std::string& vertexShaderStr, const std::string& fragmentShaderStr, const std::string& geometryShaderStr) {
    const char* vShaderCode = vertexShaderStr.c_str();
    const char * fShaderCode = fragmentShaderStr.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    G_CHECK_GL2(vertex, glCreateShader(GL_VERTEX_SHADER));
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");
    // fragment Shader
    G_CHECK_GL2(fragment, glCreateShader(GL_FRAGMENT_SHADER));
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");
    // if geometry shader is given, compile geometry shader
#if defined(WIN32) || defined(LINUX)
    unsigned int geometry;
    if(geometryShaderStr != "") {
        const char * gShaderCode = geometryShaderStr.c_str();
        geometry = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometry, 1, &gShaderCode, NULL);
        glCompileShader(geometry);
        checkCompileErrors(geometry, "GEOMETRY");
    }
#endif

    // shader Program
    id = glCreateProgram();
    loaded = true;
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
#if defined(WIN32) || defined(LINUX)
    if(geometryShaderStr != "") glAttachShader(id, geometry);
#endif
    glLinkProgram(id);
    checkCompileErrors(id, "PROGRAM");
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
#if defined(WIN32) || defined(LINUX)
    if(geometryShaderStr != "") glDeleteShader(geometry);
#endif
}

void gShader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        G_CHECK_GL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if(!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            gLoge("gShader") << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            gLoge("gShader") << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
#ifdef DEBUG
    assert(success);
#endif
}

// activate the shader
// ------------------------------------------------------------------------
void gShader::use() const {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUseProgram(id));
}

// utility uniform functions
// ------------------------------------------------------------------------
void gShader::setBool(const std::string &name, bool value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform1i(getUniformLocation(name), (int)value));
}

// ------------------------------------------------------------------------
void gShader::setInt(const std::string &name, int value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform1i(getUniformLocation(name), value));
}

// ------------------------------------------------------------------------
void gShader::setFloat(const std::string &name, float value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform1f(getUniformLocation(name), value));
}

// ------------------------------------------------------------------------
void gShader::setVec2(const std::string &name, const glm::vec2 &value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform2fv(getUniformLocation(name), 1, &value[0]));
}

void gShader::setVec2(const std::string &name, float x, float y) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform2f(getUniformLocation(name), x, y));
}

// ------------------------------------------------------------------------
void gShader::setVec3(const std::string &name, const glm::vec3 &value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform3fv(getUniformLocation(name), 1, &value[0]));
}

void gShader::setVec3(const std::string &name, float x, float y, float z) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform3f(getUniformLocation(name), x, y, z));
}

// ------------------------------------------------------------------------
void gShader::setVec4(const std::string &name, const glm::vec4 &value) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform4fv(getUniformLocation(name), 1, &value[0]));
}

void gShader::setVec4(const std::string &name, float x, float y, float z, float w) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniform4f(getUniformLocation(name), x, y, z, w));
}

// ------------------------------------------------------------------------
void gShader::setMat2(const std::string &name, const glm::mat2 &mat) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

// ------------------------------------------------------------------------
void gShader::setMat3(const std::string &name, const glm::mat3 &mat) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

// ------------------------------------------------------------------------
void gShader::setMat4(const std::string &name, const glm::mat4 &mat) {
#ifdef DEBUG
    assert(loaded);
#endif
    G_CHECK_GL(glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]));
}

GLint gShader::getUniformLocation(const std::string& name) {
	// Check if the location is already in the map
	auto it = uniformlocations.find(name);
	if (it != uniformlocations.end()) {
		return it->second;
	}

	// If not, get the location and store it in the map
	GLint location = glGetUniformLocation(id, name.c_str());
	uniformlocations[name] = location;
	return location;
}

