/*
 * gTypes.h
 *
 *  Created on: August 6, 2021
 *      Author: user
 */

#ifndef ENGINE_UTILS_GTYPES_H_
#define ENGINE_UTILS_GTYPES_H_

// variable types
typedef unsigned char gByte;
typedef unsigned int gUInt;
// string
#include <string>

typedef std::string gString;

// vector
#include <vector>

template<typename T>
using gVector = std::vector<T>;

// map
#include <map>

template<typename K, typename V>
using gMap = std::map<K, V>;

// smart pointer
#include <memory>

template<typename T>
using gSharedPtr = std::shared_ptr<T>;

template<typename T>
using gUniquePtr = std::unique_ptr<T>;

template<typename T>
static gSharedPtr<T> gMakeShared(T value) {
    return std::make_shared<T>(value);  
}

static gUniquePtr<T> gMakeUnique(T value) {
    return std::make_unique<T>(value);
}

// set
#include <set>

template<typename K>
using gSet = std::set<K>;

#endif /* ENGINE_UTILS_GTYPES_H_ */
