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

/**
 * @class gSoundRecorder
 * @brief A sound recorder class using miniaudio for microphone input capture.
 *
 * This class records audio from the default input device and writes it to a file
 * using ma_device and ma_encoder. Recording can be started, paused, resumed, and stopped.
 * Quality presets control the audio fidelity and file size.
 */
class gSoundRecorder : public gObject {
public:

    enum {RECORDQUALITY_LOW, RECORDQUALITY_MIDDLE, RECORDQUALITY_HIGH, RECORDQUALITY_ULTRA};

    gSoundRecorder();
    virtual ~gSoundRecorder();

    /**
     * @brief Starts recording and saves microphone input to a file.
     *
     * @param filename Path of the output file.
     * @param quality Recording quality preset.
     */
    void startRecording(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);

    /**
     * @brief Starts recording using a project-relative path.
     *
     * @param filename Relative path of the output file.
     * @param quality Recording quality preset.
     */
    void startRecordingSound(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);

    /**
     * @brief Stops recording and finalizes the output file.
     */
    void stopRecording();

    /**
     * @brief Checks if recording is active.
     *
     * @return True if recording, false otherwise.
     */
    bool isRecording();

    /**
     * @brief Pauses or resumes the recording process.
     *
     * @param recordingPaused True to pause, false to resume.
     * @return Current pause state.
     */
    bool setRecordingPaused(bool recordingPaused);

    /**
     * @brief Checks if recording is currently paused.
     *
     * @return True if paused, false otherwise.
     */
    bool isRecordingPaused();

private:
    ma_device capturedevice;
    ma_encoder encoder;
    bool recording = false;
    bool isrecordpaused = false;
    std::string filepath;
};

#endif // GLISTENGINE_GSOUNDRECORDER_H
