/*
 * gRecorder.h
 *
 *  Created on: October 13, 2025
 *      Author: Engin Kutlu
 */

#ifndef GLISTENGINE_GRECORD_H
#define GLISTENGINE_GRECORD_H

#include <string>
#include "miniaudio.h"

class gRecorder {
public:

    enum {RECORDQUALITY_LOW, RECORDQUALITY_MIDDLE, RECORDQUALITY_HIGH, RECORDQUALITY_ULTRA};

    gRecorder();
    virtual ~gRecorder();

    void startRecording(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);
    void startRecordingSound(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);
    void stopRecording();

    bool isRecording();
    bool setRecordingPaused(bool pauseRecording);
    bool isRecordingPaused();

private:
    ma_device capturedevice;
    ma_encoder encoder;
    bool recording = false;
    bool isrecordpaused = false;
    std::string filepath;
};

#endif // GLISTENGINE_GRECORD_H
