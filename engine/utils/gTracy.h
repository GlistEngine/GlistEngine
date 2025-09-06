//
// Created by Metehan Gezer on 01/09/2025.
//

#ifndef MARTYR_GTRACY_H
#define MARTYR_GTRACY_H

// Tracy is a profiler for C++
// https://github.com/wolfpld/tracy

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#define G_PROFILE_FRAME_MARK() FrameMark
#define G_PROFILE_ZONE_SCOPED() ZoneScoped
#define G_PROFILE_ZONE_SCOPED_N(name) ZoneScopedN(name)
#define G_PROFILE_LOCKABLE(type, varname) TracyLockable(type, varname)
#define G_PROFILE_MESSAGE(text, size) TracyMessage(text, size)
#define G_PROFILE_MESSAGE_L(text) TracyMessageL(text)
#define G_PROFILE_ALLOC(ptr, size) TracyAlloc(ptr, size)
#define G_PROFILE_FREE(ptr) TracyFree(ptr)
#define G_PROFILE_ZONE_VALUE(value) ZoneValue(value)
#else
#define G_PROFILE_FRAME_MARK()
#define G_PROFILE_ZONE_SCOPED()
#define G_PROFILE_ZONE_SCOPED_N(name)
#define G_PROFILE_LOCKABLE(type, varname) type varname
#define G_PROFILE_MESSAGE(text, size)
#define G_PROFILE_MESSAGE_L(text)
#define G_PROFILE_ALLOC(ptr, size)
#define G_PROFILE_FREE(ptr)
#define G_PROFILE_ZONE_VALUE(value)
#endif

#endif //MARTYR_GTRACY_H