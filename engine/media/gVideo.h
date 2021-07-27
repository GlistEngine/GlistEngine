/*
 * gVideo.h
 *
 *  Created on: 14 Tem 2021
 *      Author: kayra
 */

#ifndef MEDIA_GVIDEO_H_
#define MEDIA_GVIDEO_H_

#include "gObject.h"
#include "gst/gst.h"

#if defined(WIN32)
#include <windef.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(LINUX)
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif

#include "GLFW/glfw3.h"

class gVideo : public gObject {
public:
	gVideo();
	virtual ~gVideo();

	void load(std::string fullPath);
	void loadVideo(std::string videoPath);

	void update();

	void play();
	void setPaused(bool isPaused);
	void stop();
	void close();

	void setVolume(float volume);

	static void setWindow(GLFWwindow** window);
private:
	struct CustomData {
	  GstElement *pipeline;
	  GstElement *source;
	  GstElement *convert, *vconvert;
	  GstElement *resample;
	  GstElement *sink, *vsink;

	  GstBus *bus;

	  bool terminate;
	} data;

	static GLFWwindow* window;
	static bool isgstinitialized;

	void setup();
	void buildPipeline();

	static void pad_added_handler (GstElement *src, GstPad *new_pad, CustomData *data);
	static GstBusSyncReply bus_sync_handler (GstBus *src, GstMessage *message, CustomData *data);
	static bool message_handler(GstBus *bus, GstMessage *message, CustomData *data);
};

#endif /* MEDIA_GVIDEO_H_ */
