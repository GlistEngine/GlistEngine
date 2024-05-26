//
// Created by irrel on 21.12.2023.
//

#pragma once

#include "gObject.h"
#include "gRenderer.h"

template<typename T>
class gUbo : public gObject {
public:
	template<typename ...Args>
	gUbo(Args&&... args, int bindingpoint, int usage = GL_DYNAMIC_DRAW) : id(GL_NONE), size(sizeof(T)), bindingpoint(bindingpoint) {
		data = new T(std::forward<Args>(args)...);
		G_CHECK_GL(glGenBuffers(1, &id));
		bind();
		G_CHECK_GL(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, usage));
		unbind();
		// define the range of the buffer that links to a uniform binding point
		G_CHECK_GL(glBindBufferRange(GL_UNIFORM_BUFFER, bindingpoint, id, 0, size));
		update();
	}

	~gUbo() {
		if (id != GL_NONE) {
			G_CHECK_GL(glDeleteBuffers(1, &id));
		}
		delete data;
	}

	void bind() const {
		G_CHECK_GL(glBindBuffer(GL_UNIFORM_BUFFER, id));
	}

	void unbind() const {
		G_CHECK_GL(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	}

	void update() {
		update(0, size);
	}

	void update(int offset, int length) {
		void* ptr = static_cast<void*>(reinterpret_cast<char*>(data) + offset);
		bind();
		G_CHECK_GL(glBufferSubData(GL_UNIFORM_BUFFER, offset, length, ptr));
		unbind();
	}

	T* getData() {
		return data;
	}

	void setData(T* newObject) {
		data = newObject;
		update();
	}

	GLuint getId() const {
		return id;
	}

	int getBindingPoint() const {
		return bindingpoint;
	}

	int getSize() const {
		return size;
	}
private:
	GLuint id;
	int size;
	T* data;
	int bindingpoint;
};
