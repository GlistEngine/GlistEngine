/*
 * gSoundRecorder.h
 *
 *  Created on: October 14, 2025
 *      Author: Engin Kutlu
 */

#ifndef GLISTENGINE_GSOUNDRECORDER_H
#define GLISTENGINE_GSOUNDRECORDER_H

#include "gObject.h"
#include "miniaudio.h"

class gSoundRecorder : public gObject {
public:

    enum {RECORDQUALITY_LOW, RECORDQUALITY_MIDDLE, RECORDQUALITY_HIGH, RECORDQUALITY_ULTRA};

    gSoundRecorder();
    virtual ~gSoundRecorder();

    void startRecording(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);
    void startRecordingSound(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);
    void stopRecording();

    bool isRecording();
    bool setRecordingPaused(bool recordinPaused);
    bool isRecordingPaused();

private:
    ma_device capturedevice;
    ma_encoder encoder;
    bool recording = false;
    bool isrecordpaused = false;
    std::string filepath;
};

#endif // GLISTENGINE_GSOUNDRECORDER_H
