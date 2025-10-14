/*
 * gSoundRecorder.cpp
 *
 *  Created on: October 14, 2025
 *      Author: Engin Kutlu
 */
#include "gSoundRecorder.h"
#include "miniaudio.h"

gSoundRecorder::gSoundRecorder() {}

gSoundRecorder::~gSoundRecorder() {
    stopRecording();
}

void gSoundRecorder::startRecording(const std::string& filename, int quality) {
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

        case RECORDQUALITY_ULTRA:
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
    	gSoundRecorder* self = (gSoundRecorder*)pDevice->pUserData;
        if (self && self->recording && !self->isrecordpaused) {
            ma_encoder_write_pcm_frames(&self->encoder, (const void*)pInput, frameCount, NULL);
        }
        (void)pOutput;
    };

    deviceConfig.pUserData = this;

    if (ma_device_init(NULL, &deviceConfig, &capturedevice) != MA_SUCCESS) {
        gLoge("gSoundRecorder") << "Failed to initialize capture device!";
        ma_encoder_uninit(&encoder);
        return;
    }

    if (ma_device_start(&capturedevice) != MA_SUCCESS) {
        gLoge("gSoundRecorder") << "Failed to start device!";
        ma_device_uninit(&capturedevice);
        ma_encoder_uninit(&encoder);
        return;
    }

    recording = true;
    isrecordpaused = false;
    gLogi("gSoundRecorder") << "Recording started: " << filename
                 << " (Bit depth: "
                 << (quality == RECORDQUALITY_LOW ? "8"
                     : quality == RECORDQUALITY_MIDDLE ? "16"
                     : quality == RECORDQUALITY_HIGH ? "24" : "32f")
                 << " bits, Sample rate: " << deviceConfig.sampleRate << ")";
}

void gSoundRecorder::startRecordingSound(const std::string& filename, int quality) {
    // automatically saves under assets/sounds
    startRecording(gGetSoundsDir() + filename, gSoundRecorder::RECORDQUALITY_MIDDLE);
}

void gSoundRecorder::stopRecording() {
    if (!recording) return;

    isrecordpaused = false;

    ma_device_stop(&capturedevice);
    ma_device_uninit(&capturedevice);
    ma_encoder_uninit(&encoder);

    recording = false;
    gLogi("gSoundRecorder") << "Record is stopped: " << filepath;
}

bool gSoundRecorder::isRecording() {
    return recording;
}

bool gSoundRecorder::setRecordingPaused(bool recordingPaused) {
    if (!recording) return false;

    if (recordingPaused && !isrecordpaused) {
        // Pause
        ma_device_stop(&capturedevice);
        isrecordpaused = true;
        gLogi("gSoundRecorder") << "Recording is paused.";
    } else if (!recordingPaused && isrecordpaused) {
        // Resume
        ma_device_start(&capturedevice);
        isrecordpaused = false;
        gLogi("gSoundRecorder") << "Recording is resumed.";
    }

    return isrecordpaused;
}

bool gSoundRecorder::isRecordingPaused() {
    return isrecordpaused;
}
