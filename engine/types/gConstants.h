/*
 * gUtils.h
 *
 *  Created on: 22 Feb 2021
 *      Author: Noyan
 */

#pragma once

#ifndef TYPES_GCONSTANTS_H_
#define TYPES_GCONSTANTS_H_

#define G_WINDOWMODE_NONE -1
#define G_WINDOWMODE_GAME 0
#define G_WINDOWMODE_FULLSCREEN 1
#define G_WINDOWMODE_APP 2
#define G_WINDOWMODE_FULLSCREENGUIAPP 3
#define G_WINDOWMODE_GUIAPP 4

// No scaling, the given resolution will be used as is
#define G_SCREENSCALING_NONE 0
#define G_SCREENSCALING_MIPMAP 1
// Screen will be scaled dynamically, providing the same unit resolution but on different resolutions.
#define G_SCREENSCALING_AUTO 2
// Screen will be scaled dynamically, but unlike AUTO it makes sure the initial scale is kept and unit resolution is updated automatically.
#define G_SCREENSCALING_AUTO_ONCE 3

#define G_LOOPMODE_NORMAL 1
#define G_LOOPMODE_NONE 0


#endif /* TYPES_GCONSTANTS_H_ */
