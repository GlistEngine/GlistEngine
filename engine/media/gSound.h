/*
 * gSound.h
 *
 *  Created on: April 13, 2025
 *      Author: Metehan Gezer
 */

#ifndef GLISTENGINE_GSOUND_H
#define GLISTENGINE_GSOUND_H

#include "gBaseSound.h"
#include "miniaudio.h"


ma_engine* gGetSoundEngine();

/**
 * @class gSound
 * @brief A sound class using miniaudio with ma_engine and ma_sound.
 *
 * This class wraps a miniaudio ma_sound object and provides functions for loading,
 * playing, pausing, stopping, seeking, and controlling volume and looping.
 * Multiple gSound instances can be played simultaneously.
 */
class gSound : public gBaseSound {
public:

    enum {RECORDQUALITY_LOW, RECORDQUALITY_MIDDLE, RECORDQUALITY_HIGH, RECORDQUALITY_ULTRA};

    gSound();
    virtual ~gSound();

    /**
     * @brief Loads a sound file from the full file system path.
     *
     * @param fullPath Full absolute path to the audio file.
     * @return int 1 if successful, 0 otherwise.
     */
    int load(const std::string& fullPath) override;

    /**
     * @brief Loads the sound file from the given project-relative path.
     *
     * @param soundPath Project-relative path to the sound file.
     * @return int 1 if successful, 0 otherwise.
     */
    int loadSound(const std::string& soundPath) override;

    /**
     * @brief Starts sound playback and resets the position to the beginning.
     */
    void play() override;

    void startRecording(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);

    void startRecordingSound(const std::string& filename, int quality = RECORDQUALITY_MIDDLE);

    void stopRecording ();

    bool isRecording();

    bool setRecordingPaused(bool pauseRecording);

    bool isRecordingPaused();


    bool isLoaded() override;

    bool isPlaying() override;

    bool isPaused() override;

    /**
     * @brief Pauses or resumes the playback.
     *
     * @param isPaused True to pause, false to resume.
     */
    void setPaused(bool isPaused) override;

    /**
     * @brief Stops playback and resets position to the beginning.
     */
    void stop() override;

    /**
     * @brief Unloads and frees the sound resource.
     */
    void close() override;

    /**
     * @brief Returns the total duration of the sound in milliseconds.
     *
     * @return Duration in ms.
     */
    int getDuration() override;

    /**
     * @brief Returns the current playback position in milliseconds.
     *
     * @return Position in ms.
     */
    int getPosition() override;

    /**
     * @brief Sets the current playback position in milliseconds.
     *
     * @param position New position in ms.
     */
    void setPosition(int position) override;

    LoopType getLoopType() override;

    /**
     * @brief Sets the loop type for playback.
     *
     * @param loopType Looping behavior.
     */
    void setLoopType(LoopType loopType) override;

    /**
     * @brief Gets the current volume level.
     *
     * @return Volume between 0.0 (mute) and 1.0 (max).
     */
    float getVolume() override;

    /**
     * @brief Sets the volume level.
     *
     * @param volume Volume between 0.0 (mute) and 1.0 (max).
     */
    void setVolume(float volume) override;

    /**
     * @brief Gets the path to the currently loaded file.
     *
     * @return File path as a string reference.
     */
    const std::string& getPath() override;

private:
    ma_sound sound;
    bool isloaded = false;
    bool isplaying = false;
    bool ispaused = false;
    float volume = 0.5f; // Current volume level [0-1].
    LoopType looptype = LOOPTYPE_DEFAULT;
    std::string filepath; // Path to the loaded audio file.
    int duration = 0; // Duration of the sound in milliseconds.
    int lastposition = 0; // Last known position, only used for pause/resume logic.

    ma_device captureDevice;
    ma_encoder encoder;
    bool recording = false;
    bool isrecordpaused = false;
};


#endif //GLISTENGINE_GSOUND_H
