/*
 * Copyright (C) 2016 Nitra Games Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef UI_GGUICONTROL_H_
#define UI_GGUICONTROL_H_

#include "gBaseGUIObject.h"

/**
 * Base class for gui controls.
 *
 * Each gui control should extend gGUIControl class and put into engine/ui directory.
 *
 * For now, gGUIButton, gGUIText and gGUITextbox are implemented as example for gui developers.
 */
class gGUIControl: public gBaseGUIObject {
public:

	gGUIControl();
	virtual ~gGUIControl();

	virtual void set(gBaseApp* root, gBaseGUIObject* topParentGUIObject, gBaseGUIObject* parentGUIObject, int parentSlotLineNo, int parentSlotColumnNo, int x, int y, int w, int h);

	virtual int getCursor(int x, int y);

	virtual void update();
	virtual void draw();

	virtual int calculateContentHeight() {
		return height;
	}

	virtual bool countAsSpace();
	void setCountAsSpace(bool isSpace);

protected:
	bool countasspace;
	int oldwidth, oldheight;
};

#endif /* UI_GGUICONTROL_H_ */
