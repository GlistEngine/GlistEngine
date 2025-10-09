/*
 * gSound.cpp
 *
 *  Created on: April 13, 2025
 *      Author: Metehan Gezer
 */
#include "gSound.h"
//
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static ma_engine engine;
static bool initialized = false;

ma_engine* gGetSoundEngine() {
    if(!initialized) {
        if(ma_engine_init(nullptr, &engine) == MA_SUCCESS) {
            initialized = true;
        }
    }
    return &engine;
}

gSound::gSound() : gBaseSound() {}

gSound::~gSound() {
    close();
}

int gSound::load(const std::string& fullPath) {
    close(); // in case previously loaded

    if(ma_sound_init_from_file(gGetSoundEngine(), fullPath.c_str(),
    		MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC,
			nullptr, nullptr, &sound) != MA_SUCCESS) {
        return 0;
    }

    filepath = fullPath;
    isloaded = true;
    ispaused = false;
    isplaying = false;
    volume = 0.5f;
    looptype = LOOPTYPE_DEFAULT;
    ma_uint64 temp;
    if(ma_sound_get_length_in_pcm_frames(&sound, &temp) != MA_SUCCESS) {
        close();
        return 0;
    }
    ma_sound_set_end_callback(&sound, [](void* pUserData, ma_sound* sound) {
        gSound* self = static_cast<gSound*>(pUserData);
        self->isplaying = false;
    }, this);

    duration = static_cast<int>((temp * 1000) /
                                ma_engine_get_sample_rate(gGetSoundEngine()));
    ma_sound_set_volume(&sound, volume);

    return 1;
}

int gSound::loadSound(const std::string& soundPath) {
    return load(gGetSoundsDir() + soundPath);
}

void gSound::play() {
    if(!isloaded) {
        gLogw("gSound") << "Tried to play a sound that was not loaded!";
        return;
    }
    ma_sound_start(&sound);
    isplaying = true;
    ispaused = false;
}

void gSound::startRecording(const std::string& filename, int quality) {
    if (recording) return;

    filepath = filename;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);

    // Apply bit depth and sample rate based on quality
    switch (quality) {
        case RECORDQUALITY_LOW:
            deviceConfig.capture.format   = ma_format_u8;   // 8-bit unsigned
            deviceConfig.capture.channels = 1;
            deviceConfig.sampleRate       = 22050;
            break;

        case RECORDQUALITY_MIDDLE:
            deviceConfig.capture.format   = ma_format_s16;  // 16-bit signed
            deviceConfig.capture.channels = 1;
            deviceConfig.sampleRate       = 44100;
            break;

        case RECORDQUALITY_HIGH:
            deviceConfig.capture.format   = ma_format_s24;  // 24-bit signed
            deviceConfig.capture.channels = 1;
            deviceConfig.sampleRate       = 48000;
            break;

        case RECORDQUALITY_PRO:
            deviceConfig.capture.format   = ma_format_f32;  // 32-bit float
            deviceConfig.capture.channels = 2;
            deviceConfig.sampleRate       = 96000;
            break;

        default:
            deviceConfig.capture.format   = ma_format_s16;
            deviceConfig.capture.channels = 1;
            deviceConfig.sampleRate       = 44100;
            break;
    }

    ma_encoder_config encoderConfig =
        ma_encoder_config_init(ma_encoding_format_wav,
                               deviceConfig.capture.format,
                               deviceConfig.capture.channels,
                               deviceConfig.sampleRate);

    if (ma_encoder_init_file(filename.c_str(), &encoderConfig, &encoder) != MA_SUCCESS) {
        gLoge("Mic") << "Failed to initialize encoder!";
        return;
    }

    deviceConfig.dataCallback = [](ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
        gSound* self = (gSound*)pDevice->pUserData;
        if (self && self->recording && !self->isrecordpaused) {
            ma_encoder_write_pcm_frames(&self->encoder, (const void*)pInput, frameCount, NULL);
        }
        (void)pOutput;
    };

    deviceConfig.pUserData = this;

    if (ma_device_init(NULL, &deviceConfig, &captureDevice) != MA_SUCCESS) {
        gLoge("Mic") << "Failed to initialize capture device!";
        ma_encoder_uninit(&encoder);
        return;
    }

    if (ma_device_start(&captureDevice) != MA_SUCCESS) {
        gLoge("Mic") << "Failed to start device!";
        ma_device_uninit(&captureDevice);
        ma_encoder_uninit(&encoder);
        return;
    }

    recording = true;
    isrecordpaused = false;
    gLogi("Mic") << "Recording started: " << filename
                 << " (Bit depth: "
                 << (quality == RECORDQUALITY_LOW ? "8"
                     : quality == RECORDQUALITY_MIDDLE ? "16"
                     : quality == RECORDQUALITY_HIGH ? "24" : "32f")
                 << " bits, Sample rate: " << deviceConfig.sampleRate << ")";
}


void gSound::startRecordingSound(const std::string& filename, int quality) {
    // automatically saves under assets/sounds
    startRecording(gGetSoundsDir() + filename, gSound::RECORDQUALITY_MIDDLE);
}

void gSound::stopRecording() {
    if (!recording) return;

    ma_device_stop(&captureDevice);
    ma_device_uninit(&captureDevice);
    ma_encoder_uninit(&encoder);

    recording = false;
    gLogi("Mic") << "Record is stopped: " << filepath;
}

bool gSound::isRecording() {
    return recording;
}


void gSound::setPaused(bool paused) {
    if(!isloaded) {
        gLogw("gSound") << "Tried to pause a sound that was not loaded!";
        return;
    }
    if(paused && !ispaused) {
        lastposition = getPosition(); // store current time
        ma_sound_stop(&sound);
        ispaused = true;
    } else if(!paused && ispaused) {
        ma_sound_start(&sound);
        setPosition(lastposition); // resume from stored time
        ispaused = false;
    }
}

void gSound::stop() {
    if(!isloaded) return;
    ma_sound_stop(&sound);
    ma_sound_seek_to_pcm_frame(&sound, 0);
    isplaying = false;
}
bool gSound::pauseRecording() {
    if (!recording) return false;

    if (!isrecordpaused) {
        ma_device_stop(&captureDevice);
        isrecordpaused = true;
        gLogi("Mic") << "Recording is paused.";
    } else {
        ma_device_start(&captureDevice);
        isrecordpaused = false;
        gLogi("Mic") << "Recording is resumed.";
    }

    return isrecordpaused;
}

bool gSound::isRecordingPaused() {
    return isrecordpaused;
}

void gSound::close() {
    if(isloaded) {
        ma_sound_uninit(&sound);
        isloaded = false;
        isplaying = false;
        ispaused = false;
    }
}

void gSound::setPosition(int posMs) {
    if(!isloaded) {
        return;
    }
    ma_uint64 frame = (ma_engine_get_sample_rate(gGetSoundEngine()) * posMs) / 1000;
    ma_sound_seek_to_pcm_frame(&sound, frame);
}

int gSound::getPosition() {
    if(!isloaded) {
        return 0;
    }
    ma_uint64 temp;
    if(ma_sound_get_length_in_pcm_frames(&sound, &temp) != MA_SUCCESS) {
        return 0;
    }
    return static_cast<int>((temp * 1000) / ma_engine_get_sample_rate(gGetSoundEngine()));
}

void gSound::setLoopType(LoopType loopType) {
    this->looptype = loopType;
    if(isloaded && loopType != LOOPTYPE_DEFAULT) {
        ma_sound_set_looping(&sound, loopType == LOOPTYPE_NORMAL);
    }
}

void gSound::setVolume(float vol) {
    volume = vol;
    if(isloaded) {
        ma_sound_set_volume(&sound, vol);
    }
}

bool gSound::isLoaded() {
    return isloaded;
}

bool gSound::isPlaying() {
    return isloaded && isplaying && !ispaused;
}

bool gSound::isPaused() {
    return ispaused;
}

int gSound::getDuration() {
    return duration;
}

gSound::LoopType gSound::getLoopType() {
    return looptype;
}

float gSound::getVolume() {
    return volume;
}

const std::string& gSound::getPath() {
    return filepath;
}
