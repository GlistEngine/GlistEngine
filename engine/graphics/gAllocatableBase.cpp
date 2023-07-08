//
// Created by Metehan Gezer on 7.07.2023.
//

#include "gAllocatableBase.h"
#include <vector>

std::vector<gAllocatableBase*> allocatedobjects;

gAllocatableBase::gAllocatableBase() {
    allocatedobjects.push_back(this);
}

gAllocatableBase::~gAllocatableBase() {
    auto end = std::remove(allocatedobjects.begin(), allocatedobjects.end(), this);
    allocatedobjects.erase(end, allocatedobjects.end());
}

void gAllocatableBase::allocate() {

}

void gAllocatableBase::deallocate() {

}

void gAllocatableBase::reallocate() {

}

void gAllocatableBase::reallocateAll() {
    gLogi("gAllocatableBase") << "Reallocating all...";
    for (const auto &item: allocatedobjects) {
        item->reallocate();
    }
    gLogi("gAllocatableBase") << "All reallocated!";
}