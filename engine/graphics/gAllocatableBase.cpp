//
// Created by Metehan Gezer on 7.07.2023.
//

#include "gAllocatableBase.h"
#include <vector>

std::vector<gAllocatableBase*> allocatedobjects;

gAllocatableBase::gAllocatableBase() {
    hasallocatedonce = false;
}

gAllocatableBase::~gAllocatableBase() {
	if(!hasallocatedonce) {
		return;
	}
    auto end = std::remove(allocatedobjects.begin(), allocatedobjects.end(), this);
    allocatedobjects.erase(end, allocatedobjects.end());
    hasallocatedonce = false;
}

void gAllocatableBase::allocate() {
    if(!hasallocatedonce) {
        allocatedobjects.push_back(this);
        hasallocatedonce = true;
    }
}

void gAllocatableBase::deallocate() {

}

void gAllocatableBase::reallocate() {

}

void gAllocatableBase::reallocateAll() {
//    gLogi("gAllocatableBase") << "Reallocating all...";
    for (const auto &item: allocatedobjects) {
        item->reallocate();
    }
//    gLogi("gAllocatableBase") << "All reallocated!";
}

void gAllocatableBase::allocateAll() {
//    gLogi("gAllocatableBase") << "Allocating all...";
    for (const auto &item: allocatedobjects) {
        item->allocate();
    }
//    gLogi("gAllocatableBase") << "All allocated!";
}

void gAllocatableBase::deallocateAll() {
//    gLogi("gAllocatableBase") << "Deallocating all...";
    for (const auto &item: allocatedobjects) {
        item->deallocate();
    }
//    gLogi("gAllocatableBase") << "All deallocated!";
}