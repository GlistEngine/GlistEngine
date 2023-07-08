//
// Created by Metehan Gezer on 7.07.2023.
//

#ifndef GALLOCATABLEBASE_H
#define GALLOCATABLEBASE_H

#include "gObject.h"

/**
 * This is a base class for renderer allocated object that can be reallocated.
 * For example textures...
 */
class gAllocatableBase {
public:
    gAllocatableBase();
    virtual ~gAllocatableBase();

    virtual void allocate();
    virtual void deallocate();
    virtual void reallocate();

    static void reallocateAll();
};


#endif //GALLOCATABLEBASE_H
