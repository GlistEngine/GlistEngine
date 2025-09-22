//
// Created by irrel on 21.12.2023.
//

#pragma once

#include "gObject.h"
#include "gRenderer.h"
#include "gRenderObject.h"

template<typename T>
class gUbo : public gRenderObject {
public:
	template<typename ...Args>
	gUbo(Args&&... args, int bindingpoint, int usage = GL_DYNAMIC_DRAW) : id(GL_NONE), size(sizeof(T)), bindingpoint(bindingpoint) {
		data = new T(std::forward<Args>(args)...);
		id = renderer->genBuffers();
		renderer->setBufferData(id, nullptr, size, usage);
		// define the range of the buffer that links to a uniform binding point
		renderer->setBufferRange(bindingpoint, id, 0, size);
		update();
	}

	~gUbo() {
		if (id != GL_NONE) {
			renderer->deleteBuffer(id);
		}
		delete data;
	}

	void bind() const {
		renderer->bindBuffer(GL_UNIFORM_BUFFER, id);
	}

	void unbind() const {
		renderer->unbindBuffer(GL_UNIFORM_BUFFER);
	}

	void update() {
		update(0, size);
	}

	void update(int offset, int length) {
		void* ptr = reinterpret_cast<char*>(data) + offset;
		renderer->bufSubData(id, offset, length, ptr);
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
