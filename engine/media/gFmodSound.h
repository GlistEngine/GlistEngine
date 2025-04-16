/*
 * gFmodSound.h
 *
 *  Created on: 16 Nis 2025
 *      Author: Admin
 */

#ifndef MEDIA_GFMODSOUND_H_
#define MEDIA_GFMODSOUND_H_

#include "gSound.h"

class gFmodSound : public gSound{
public:
	gFmodSound();
	virtual ~gFmodSound();

    /**
     * @brief Loads a sound file from the full file system path.
     *
     * @param fullPath Full absolute path to the audio file.
     * @return int 1 if successful, 0 otherwise.
     */
    int load(const std::string& fullPath);

    /**
     * @brief Loads the sound file from the given project-relative path.
     *
     * @param soundPath Project-relative path to the sound file.
     * @return int 1 if successful, 0 otherwise.
     */
    int loadSound(const std::string& soundPath);

    /**
     * @brief Starts sound playback and resets the position to the beginning.
     */
    void play();

    bool isLoaded();

    bool isPlaying();

    bool isPaused();

    /**
     * @brief Pauses or resumes the playback.
     *
     * @param isPaused True to pause, false to resume.
     */
    void setPaused(bool isPaused);

    /**
     * @brief Stops playback and resets position to the beginning.
     */
    void stop();

    /**
     * @brief Unloads and frees the sound resource.
     */
    void close();

    /**
     * @brief Returns the total duration of the sound in milliseconds.
     *
     * @return Duration in ms.
     */
    int getDuration();

    /**
     * @brief Returns the current playback position in milliseconds.
     *
     * @return Position in ms.
     */
    int getPosition();

    /**
     * @brief Sets the current playback position in milliseconds.
     *
     * @param position New position in ms.
     */
    void setPosition(int position);

    /**
     * @brief Sets the loop type for playback.
     *
     * @param loopType Looping behavior.
     */
    void setLoopType(LoopType loopType);

    /**
     * @brief Gets the current volume level.
     *
     * @return Volume between 0.0 (mute) and 1.0 (max).
     */
    float getVolume();

    /**
     * @brief Sets the volume level.
     *
     * @param volume Volume between 0.0 (mute) and 1.0 (max).
     */
    void setVolume(float volume);

    /**
     * @brief Gets the path to the currently loaded file.
     *
     * @return File path as a string reference.
     */
    const std::string& getPath();


private:
	gSound sound;
};

#endif /* MEDIA_GFMODSOUND_H_ */
