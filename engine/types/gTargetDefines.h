/*
 * gTargetDefines.h
 *
 *  Created on: Sep 20, 2026
 *      Author: cogwright
 *
 * The one place the engine's short build target names are derived from the
 * macros the compiler defines: WIN32, WIN64, APPLE, ANDROID, LINUX, EMSCRIPTEN,
 * and the GLIST_* flags built on top of them. Compile time only, and unrelated
 * to gBasePlatform, which is the runtime platform abstraction.
 *
 * The build force includes this into every translation unit, so a header can
 * guard on these names without having to include anything first. See the
 * GLIST_TARGET_DEFINES block in engine/CMakeLists.txt.
 *
 * Names only. Anything that needs the GL headers includes gRenderer.h, which is
 * where those live and where their per target paths are handled.
 */

#pragma once

#ifndef TYPES_GTARGETDEFINES_H_
#define TYPES_GTARGETDEFINES_H_

#if defined(__EMSCRIPTEN__)
    // Web. Checked first because Emscripten also defines __unix__, so the unix
    // arm at the end of this chain would otherwise swallow it.
	#ifndef EMSCRIPTEN
		#define EMSCRIPTEN 1
	#endif
#elif defined(_WIN64)
   //define something for Windows (64-bit)
	#ifndef WIN64
		#define WIN64 _WIN32
	#endif
#elif _WIN32
   //define something for Windows (32-bit)
	#ifndef WIN32
		#define WIN32 _WIN32
	#endif
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        // define something for simulator
    #elif TARGET_OS_IPHONE
        // define something for iphone
    #else
		#ifndef TARGET_OS_OSX
			#define TARGET_OS_OSX 1
		#endif
		#ifndef APPLE
			#define APPLE __APPLE__
		#endif
        // define something for OSX
    #endif
#elif defined(__ANDROID__)
    // Android
	#ifndef ANDROID
		#define ANDROID 1
	#endif
#elif __linux
    // Linux
	#ifndef LINUX
		#define LINUX __linux__
	#endif
#elif __unix // all unices not caught above
    // Unix
#elif __posix
    // POSIX
#endif

#if defined(ANDROID) || defined(__ANDROID__) || TARGET_OS_IPHONE || TARGET_OS_SIMULATOR || EMSCRIPTEN
#ifndef GLIST_OPENGLES
#define GLIST_OPENGLES 1
#endif
#endif

#if EMSCRIPTEN
#ifndef GLIST_WEB
#define GLIST_WEB 1
#endif
#endif
#if TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
#ifndef GLIST_IOS
#define GLIST_IOS 1
#endif
#endif
#if defined(ANDROID) || defined(__ANDROID__)
#ifndef GLIST_ANDROID
#define GLIST_ANDROID 1
#endif
#endif

#endif /* TYPES_GTARGETDEFINES_H_ */
