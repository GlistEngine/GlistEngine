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

#ifndef UTILS_GASSETLOADER_H_
#define UTILS_GASSETLOADER_H_

#include <map>
#include <queue>
#include "gThread.h"
#include "gImage.h"
#include "gModel.h"
#include "gFont.h"
#include "gFmodSound.h"


class gAssetLoader: public gThread {
public:
	static const short SIGNAL_NONE = 0;
	static const int TYPE_IMAGE = 0, TYPE_TEXTURE = 1, TYPE_MODEL = 2, TYPE_FONT = 3, TYPE_SOUND = 4;

	gAssetLoader();
	virtual ~gAssetLoader();

    void load(gImage& image, std::string fullPath, short signal = SIGNAL_NONE);
    void loadImage(gImage& image, std::string imagePath, short signal = SIGNAL_NONE);

    void threadFunction();

    int getLoadedAssetNum();
	short getSignal();
	void resetSignal();
	int getCounter();
	void reset();
	bool isUpdateNeeded();

	void update();


private:

    struct assetToLoad {
        assetToLoad() {
            image = nullptr;
			texture = nullptr;
			model = nullptr;
			font = nullptr;
			sound = nullptr;
			filename = "";
			name = "";
			assetsignal = SIGNAL_NONE;
			fontsize = 0;
			modeloptimize = false;
			type = TYPE_IMAGE;
        }

		assetToLoad(gImage& image, std::string filename, short signal) {
			this->image = &image;
            this->filename = filename;
            this->name = filename;
            type = TYPE_IMAGE;
			assetsignal = signal;
		}

		assetToLoad(gTexture& texture, std::string filename, short signal) {
			this->texture = &texture;
            this->filename = filename;
            this->name = filename;
            type = TYPE_TEXTURE;
			assetsignal = signal;
		}

		assetToLoad(gModel& model, std::string filename, bool optimize, short signal) {
			this->model = &model;
            this->filename = filename;
            this->name = filename;
            this->modeloptimize = optimize;
            type = TYPE_MODEL;
			assetsignal = signal;
		}

		assetToLoad(gFont& font, std::string filename, int size, short signal) {
			this->font = &font;
            this->filename = filename;
            this->name = filename;
            this->fontsize = size;
            type = TYPE_FONT;
			assetsignal = signal;
		}

		assetToLoad(gFmodSound& sound, std::string filename, short signal) {
			this->sound = &sound;
			this->filename = filename;
			this->name = filename;
			type = TYPE_SOUND;
			assetsignal = signal;
		}

        gImage* image;
		gTexture* texture;
		gModel* model;
		gFont* font;
		gFmodSound* sound;

		std::string filename;
		std::string name;
		int type;
        bool modeloptimize;
        int fontsize;

        short assetsignal;
    };


    int loadedassetnum;
	short signal;
	int counter;

	std::queue<assetToLoad> queue, queue2;
	bool transferfinished;
	bool isupdateneeded;


	template<typename T>
	bool send(T& value) {
		std::unique_lock<std::mutex> lock(mutex);
		queue.push(std::move(value));
	    conditionvariable.notify_one();
		return true;
	}

	template<typename T>
	bool sendToUpdate(T& value) {
		std::unique_lock<std::mutex> lock(mutex);
		queue2.push(std::move(value));
	    conditionvariable.notify_one();
		return true;
	}

	template<typename T>
	bool receive(T& sentValue){
		std::unique_lock<std::mutex> lock(mutex);
        while(queue.empty() && !transferfinished){
			conditionvariable.wait(lock);
		}
		if(!transferfinished){
			std::swap(sentValue, queue.front());
			queue.pop();
			return true;
		}else{
			return false;
		}
	}

	template<typename T>
	bool receiveToUpdate(T& sentValue){
		std::unique_lock<std::mutex> lock(mutex);
        while(queue2.empty() && !transferfinished){
			conditionvariable.wait(lock);
		}
		if(!transferfinished){
			std::swap(sentValue, queue2.front());
			queue2.pop();
			return true;
		}else{
			return false;
		}
	}


};

#endif /* UTILS_GASSETLOADER_H_ */
