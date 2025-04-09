/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All Rights Reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUDIOOUTPUT__
#define __AUDIOOUTPUT__

#include <cci_configuration>
#include <portaudio.h>
#include <scp/report.h>
#include <systemc>
#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"

#include "async_event.h"

static int paGlobalCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags,
                            void* userData);

SC_MODULE (audiooutput) {
    SCP_LOGGER();

    tlm_utils::simple_target_socket<audiooutput> socket;

    cci::cci_param<double> p_sample_rate;
    cci::cci_param<uint64_t> p_frames_per_buffer;
    cci::cci_param<uint64_t> p_buffer_frames;

    std::queue<std::pair<double, double>> frames;

    struct noteinfo {
        double frequency;
        int channel;
        int note;
    };
    std::vector<noteinfo> notes;

    struct channel {
        std::unordered_map<int, struct noteinfo> notes;
    };
    std::unordered_map<int, struct channel> channels;

    void noteOn(bool on, int midinote, int midichannel)
    {
        if (on) {
            if (channels[midichannel].notes.count(midinote)) {
                return;
            }

            float a = 442; // at least it is in my orchestra!
            float f = (a / 32) * pow(2, ((midinote - 9) / 12.0));
            SCP_INFO(())("Note {} channel {} on", midinote, midichannel);
            channels[midichannel].notes[midinote] = { f, midichannel, midinote };
        } else {
            SCP_INFO(())("Note {} channel {} off", midinote, midichannel);
            channels[midichannel].notes.erase(midinote);
        }
    }

    void b_transport(tlm::tlm_generic_payload & trans, sc_core::sc_time & delay)
    {
        auto midi = *reinterpret_cast<std::tuple<bool, int, int>*>(trans.get_data_ptr());
        noteOn(std::get<0>(midi), std::get<1>(midi), std::get<2>(midi));
    }

    PaStream* stream = nullptr;

    gs::async_event draining;

    void handle_err(PaError err)
    {
        if (err != paNoError) {
            Pa_Terminate();
            SCP_ERR(())(
                "An error occurred while using the portaudio stream\n"
                "Error number: {}\n"
                "Error message: {}",
                err, Pa_GetErrorText(err));
            throw std::runtime_error("pa error");
        }
    }
    SC_CTOR (audiooutput)
      : p_sample_rate("sample_rate", 44100, "frames per second")
      ,  p_frames_per_buffer("frames_per_buffer", 1024, "frames per frame")
      ,  p_buffer_frames("buffer_frames", 10, "Number of frames to buffer")
      {
          SCP_TRACE(())("audio output constructor");

          handle_err(Pa_Initialize());
          handle_err(Pa_OpenDefaultStream(&stream, 0,                         /* no input channels */
                                          2,                                  /* stereo output */
                                          paFloat32,                          /* 32 bit floating point output */
                                          p_sample_rate, p_frames_per_buffer, /* frames per buffer */
                                          paGlobalCallback, this));

          SC_THREAD(sampler);

          SC_METHOD(drain_control);
          sensitive << draining;
          dont_initialize();

          socket.register_b_transport(this, &audiooutput::b_transport);
      }

    ~audiooutput()
    {
        handle_err(Pa_StopStream(stream));
        handle_err(Pa_CloseStream(stream));
        Pa_Terminate();
    }

    void drain_control()
    {
        if (frames.size() > (p_buffer_frames * p_frames_per_buffer)) {
            if (Pa_IsStreamStopped(stream) == 1) {
                handle_err(Pa_StartStream(stream));
            }
            sc_core::sc_suspend_all();
        } else {
            sc_core::sc_unsuspend_all();
        }
    }
    void sampler()
    {
        while (1) {
            wait(1.0f / p_sample_rate, sc_core::SC_SEC);

            double time = sc_time_stamp().to_seconds();
            double frequency = 440;
            double amplitude = 0.1;
            double v = 0;
            for (auto c : channels) {
                for (auto n : c.second.notes) {
                    v += amplitude * std::sin(2 * M_PI * n.second.frequency * time);
                }
            }
            frames.push({ v, v });
            draining.notify();
        }
    }

public:
    /* This routine will be called by the PortAudio engine when audio is needed.
     * It may called at interrupt level on some machines so don't do anything
     * that could mess up the system like calling malloc() or free().
     */
    int paCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags)
    {
        float* out = (float*)outputBuffer;
        (void)inputBuffer; /* Prevent unused variable warning. */

        if (frames.size() < framesPerBuffer) {
            draining.notify();
            return 1;
        }
        for (auto i = 0; i < framesPerBuffer; i++) {
            assert(frames.size() >= 1);
            auto sample = frames.front();
            frames.pop();
            *out++ = sample.first;
            *out++ = sample.second;
        }
        draining.notify();
        return 0;
    }
};

static int paGlobalCallback(const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
    audiooutput* a = static_cast<audiooutput*>(userData);
    return a->paCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

#endif
