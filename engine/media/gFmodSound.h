/*
 * gFmodSound.h
 *
 *  Created on: 16 Nis 2025
 *      Author: Admin
 */

#ifndef MEDIA_GFMODSOUND_H_
#define MEDIA_GFMODSOUND_H_

#include "gSound.h"

// gFmodSound inherits from gSound. ıt is a drop-in replacement for gFmodSound.
// Only here for backwards compatability, you can consider this is an alias type.
class gFmodSound : public gSound {
public:
	gFmodSound() {
		gLoge("gFmodSound") << "gFmodSound is deprecated. Use gSound instead.\n";
	}
};

#endif /* MEDIA_GFMODSOUND_H_ */
