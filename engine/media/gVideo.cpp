/*
 * gVideo.cpp
 *
 *  Created on: 14 Tem 2021
 *      Author: kayra
 */

#include "gVideo.h"
#include "gst/video/videooverlay.h"
#include "GLFW/glfw3native.h"

GLFWwindow *gVideo::window;
bool gVideo::isgstinitialized = false;

gVideo::gVideo() {
	if(!isgstinitialized) {
		gst_init(NULL, NULL);
		gVideo::isgstinitialized = true;
	}
	data.terminate = false;
}

gVideo::~gVideo() {
	// TODO Auto-generated destructor stub
}

void gVideo::load(std::string fullPath) {
}

void gVideo::loadVideo(std::string videoPath) {
	load(gGetVideosDir() + videoPath);
}

void gVideo::update() {
	if(data.terminate) {
		gst_element_set_state(data.pipeline, GST_STATE_NULL);
		close();
	}
}

void gVideo::play() {
	GstStateChangeReturn ret;

	/* Create the empty pipeline */
	data.pipeline = gst_pipeline_new("pipeline");
	data.source = gst_element_factory_make("uridecodebin", "source");
	/* Create the elements */
	data.convert = gst_element_factory_make("audioconvert", "convert");
	data.resample = gst_element_factory_make("audioresample", "resample");
	data.sink = gst_element_factory_make("autoaudiosink", "sink");

	data.vconvert = gst_element_factory_make("videoconvert", "vsource");
	data.vsink = gst_element_factory_make("d3dvideosink", "vsink");

	if (!data.pipeline || !data.source || !data.convert || !data.resample ||
		!data.sink || !data.vconvert || !data.vsink) {
		g_printerr("Not all elements could be created.\n");
		return;
	}

	/* Build the pipeline. Note that we are NOT linking the source at this
	 * point. We will do it later. */
	gst_bin_add_many(GST_BIN(data.pipeline), data.source, data.convert,
			data.resample, data.sink, data.vconvert, data.vsink, NULL);
	if (!gst_element_link_many(data.convert, data.resample, data.sink, NULL)) {
		g_printerr("Audio elements could not be linked.\n");
		gst_object_unref(data.pipeline);
		return;
	}
	if (!gst_element_link_many(data.vconvert, data.vsink, NULL)) {
		g_printerr("Video elements could not be linked.\n");
		gst_object_unref(data.pipeline);
		return;
	}
	/* Set the URI to play */
	g_object_set(data.source,
			"uri", ("file:///" + gGetVideosDir() + "sintel_trailer-480p.mp4").c_str(),
			NULL);

	/* Connect to the pad-added signal */
	g_signal_connect(data.source,
			"pad-added", G_CALLBACK (pad_added_handler), &data);
	/* Listen to the bus */
	data.bus = gst_pipeline_get_bus(GST_PIPELINE(data.pipeline));

	gst_bus_add_watch(data.bus, (GstBusFunc)message_handler, &data);

	gst_bus_set_sync_handler(data.bus, (GstBusSyncHandler) bus_sync_handler, &data, NULL);

	// TODO: Implement state changing into another function
	ret = gst_element_set_state(data.pipeline, GST_STATE_READY);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the ready state.\n");
		gst_object_unref(data.pipeline);
		return;
	}
	ret = gst_element_set_state(data.pipeline, GST_STATE_PLAYING);
	if (ret == GST_STATE_CHANGE_FAILURE) {
		g_printerr("Unable to set the pipeline to the playing state.\n");
		gst_object_unref(data.pipeline);
		return;
	}
}

void gVideo::setPaused(bool isPaused) {
}

void gVideo::stop() {
}

void gVideo::close() {
	/* Free resources */
	gst_object_unref(data.bus);
	gst_element_set_state(data.pipeline, GST_STATE_NULL);
	gst_object_unref(data.pipeline);
}

void gVideo::setVolume(float volume) {
}

void gVideo::pad_added_handler(GstElement *src, GstPad *new_pad,
		CustomData *data) {
	GstPad *sink_pad = gst_element_get_static_pad(data->convert, "sink");
	GstPad *vsink_pad = gst_element_get_static_pad(data->vconvert, "sink");
	GstPadLinkReturn ret;
	GstCaps *new_pad_caps = NULL;
	GstStructure *new_pad_struct = NULL;
	const gchar *new_pad_type = NULL;

	g_print("Received new pad '%s' from '%s':\n", GST_PAD_NAME(new_pad),
			GST_ELEMENT_NAME(src));

	/* If our converter is already linked, we have nothing to do here */
	if (gst_pad_is_linked(sink_pad) && gst_pad_is_linked(vsink_pad)) {
		g_print("We are already linked. Ignoring.\n");
		goto exit;
	}

	/* Check the new pad's type */
	new_pad_caps = gst_pad_get_current_caps(new_pad);
	new_pad_struct = gst_caps_get_structure(new_pad_caps, 0);
	new_pad_type = gst_structure_get_name(new_pad_struct);
	if (g_str_has_prefix(new_pad_type, "audio/x-raw")) {
		ret = gst_pad_link(new_pad, sink_pad);
	} else if (g_str_has_prefix(new_pad_type, "video/x-raw")) {
		ret = gst_pad_link(new_pad, vsink_pad);
	} else {
		g_print("It has type '%s' which is not raw audio. Ignoring.\n",
				new_pad_type);
		goto exit;
	}

	/* Attempt the link */
	if (GST_PAD_LINK_FAILED(ret)) {
		g_print("Type is '%s' but link failed.\n", new_pad_type);
	} else {
		g_print("Link succeeded (type '%s').\n", new_pad_type);
	}

	exit:
	/* Unreference the new pad's caps, if we got them */
	if (new_pad_caps != NULL)
		gst_caps_unref(new_pad_caps);

	/* Unreference the sink pad */
	if (sink_pad) {
		gst_object_unref(sink_pad);
	} else if (vsink_pad) {
		gst_object_unref(vsink_pad);
	}

}

GstBusSyncReply gVideo::bus_sync_handler(GstBus *src, GstMessage *message,
		CustomData *data) {

	if (!gst_is_video_overlay_prepare_window_handle_message(message)) {
		return GST_BUS_PASS;
	}

	guintptr winhandle;

#if defined(WIN32)
	winhandle = (guintptr) glfwGetWin32Window((GLFWwindow*) gVideo::window);
#elif defined(LINUX)
	winhandle = (guintptr) glfwGetX11Window((GLFWwindow*) gVideo::window);
#elif defined(APPLE)
	winhandle = (guintptr) glfwGetCocoaWindow((GLFWwindow*) gVideo::window)
#endif

	if (winhandle != 0) {

		GstVideoOverlay *overlay;

		overlay = GST_VIDEO_OVERLAY(GST_MESSAGE_SRC (message));
		gst_video_overlay_set_window_handle(overlay, winhandle);
	}
	gst_message_unref(message);

	return GST_BUS_DROP;
}

bool gVideo::message_handler(GstBus *bus, GstMessage *msg, CustomData *data) {
	if (msg != NULL) {
		GError *err;
		gchar *debug_info;

		switch (GST_MESSAGE_TYPE(msg)) {
		case GST_MESSAGE_ERROR:
			gst_message_parse_error(msg, &err, &debug_info);
			g_printerr("Error received from element %s: %s\n",
					GST_OBJECT_NAME(msg->src), err->message);
			g_printerr("Debugging information: %s\n",
					debug_info ? debug_info : "none");
			g_clear_error(&err);
			g_free(debug_info);
			data->terminate = true;
			break;
		case GST_MESSAGE_EOS:
			g_print("End-Of-Stream reached.\n");
			data->terminate = true;
			break;
		case GST_MESSAGE_STATE_CHANGED:
			/* We are only interested in state-changed messages from the pipeline */
			if (GST_MESSAGE_SRC (msg) == GST_OBJECT(data->pipeline)) {
				GstState old_state, new_state, pending_state;
				gst_message_parse_state_changed(msg, &old_state, &new_state,
						&pending_state);
				g_print("Pipeline state changed from %s to %s:\n",
						gst_element_state_get_name(old_state),
						gst_element_state_get_name(new_state));
			}
			break;
		default:
			/* We should not reach here */
			g_printerr("Unexpected message received.\n");
			break;
		}
		gst_message_unref(msg);
	}
}

void gVideo::setWindow(GLFWwindow **window) {
	gVideo::window = *window;
}
