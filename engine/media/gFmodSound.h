/*
 * gFmodSound.h
 *
 *  Created on: 18 Kas 2020
 *      Author: Noyan Culum
 */

#ifndef MEDIA_GFMODSOUND_H_
#define MEDIA_GFMODSOUND_H_

#include "gBaseSound.h"
#include "fmod.h"
#include "fmod_common.h"

/**
 * @class gFmodSound
 * @brief A sound player class using the FMOD audio engine.
 *
 * Inherits from gBaseSound and provides basic functionality for loading,
 * playing, pausing, stopping, and managing sound properties such as volume,
 * loop type, and playback position.
 */
class gFmodSound : public gBaseSound {
public:
    gFmodSound();
    ~gFmodSound();

    /**
     * @brief Loads the sound file from the given full filesystem path.
     *
     * @param fullPath The full absolute path to the sound file.
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
     * @brief Starts or resumes sound playback.
     */
    void play() override;

    /**
     * @brief Pauses or resumes playback.
     *
     * @param isPaused True to pause, false to resume.
     */
    void setPaused(bool isPaused) override;

    /**
     * @brief Stops playback and resets the position to the beginning.
     */
    void stop() override;

    /**
     * @brief Frees FMOD resources related to this sound.
     */
    void close() override;

    bool isPlaying() override;

    bool isPaused() override;

    bool isLoaded() override {
        return isloaded;
    }

    LoopType getLoopType() override;

    /**
     * @brief Sets the loop type for the sound.
     *
     * @param loopType The loop type to use.
     */
    void setLoopType(LoopType loopType) override;

    /**
     * @brief Returns the duration of the sound in milliseconds.
     *
     * @return Duration in ms.
     */
    int getDuration() override;

    /**
     * @brief Sets the current playback position in milliseconds.
     *
     * @param position The new position in ms.
     */
    void setPosition(int position) override;

    /**
     * @brief Gets the current playback position in milliseconds.
     *
     * @return Position in ms.
     */
    int getPosition() override;

    /**
     * @brief Gets the current volume level.
     *
     * @return Volume between 0.0 and 1.0.
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
    bool isplaying, isloaded;
    unsigned int position;
    unsigned int duration;
    bool ispaused;
    float volume;
    LoopType looptype;
    std::string filepath;

    FMOD_SYSTEM* system = nullptr;
	FMOD_SOUND* sound1 = nullptr;
    FMOD_CHANNEL* channel = nullptr;
    FMOD_RESULT result;
    unsigned int version;
    void* extradriverdata = nullptr;
};


#endif /* MEDIA_GFMODSOUND_H_ */
