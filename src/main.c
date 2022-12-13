#include <math.h>
#include "raylib.h"

#define SAMPLE_RATE 44100
#define STREAM_BUFFER_SIZE 1024
#define NUM_OSCILLATORS 16
#define SAMPLE_DURATION (1.0f / SAMPLE_RATE)

typedef struct {
    float phase;
    float phase_stride;
} Oscillator;

void setOscFrequency(Oscillator* osc, float frequency)
{
    osc->phase_stride = frequency * SAMPLE_DURATION;
}

void updateOsc(Oscillator* osc)
{
    osc->phase += osc->phase_stride;
    if(osc->phase >=  1.0f)
        osc->phase -= 1.0f;
}

void zeroSignal(float* signal)
{
    for (size_t t = 0; t < STREAM_BUFFER_SIZE; t++) {
        signal[t] = 0.0f;
    }
}

float sineWaveOsc(Oscillator* osc)
{
    return sinf(2.0f * PI * osc->phase);
}

void accumulateSignal(float* signal, Oscillator* osc, float amplitude)
{
    for (size_t t = 0; t < STREAM_BUFFER_SIZE; t++) {
        updateOsc(osc);
        signal[t] += sineWaveOsc(osc) * amplitude;
    }
}

int main()
{
    const int screen_width = 1024;
    const int screen_height = 768;

    InitWindow(screen_width, screen_height, "Synth");
    SetTargetFPS(60);
    InitAudioDevice();

    unsigned int sample_rate = SAMPLE_RATE;
    SetAudioStreamBufferSizeDefault(STREAM_BUFFER_SIZE);
    AudioStream synthStream = InitAudioStream(sample_rate, sizeof(float) * 8, 1);
    SetAudioStreamVolume(synthStream, 0.05f);
    PlayAudioStream(synthStream);

    float frequency = 440.0f;
    float sample_duration = (1.0f/(float)sample_rate);

    Oscillator osc[NUM_OSCILLATORS] = {0};
    Oscillator lfo = {.phase = 0.0f};
    setOscFrequency(&lfo, 60.0f * 1024);

    float signal[STREAM_BUFFER_SIZE];

    while (WindowShouldClose() == false)
    {
        Vector2 mouse_pos = GetMousePosition();
        float normalized_mouse_x = (mouse_pos.x / (float)screen_width);

        if(IsAudioStreamProcessed(synthStream))
        {
            zeroSignal(signal);
            updateOsc(&lfo);
            float base_freq = 25.0f + (normalized_mouse_x * 400.0f) + (sineWaveOsc(&lfo) * 50.0f);
            for (size_t i = 0; i < NUM_OSCILLATORS; ++i)
            {
                if(i % 2 != 0)
                {
                    //float normalized_index = (float)i / NUM_OSCILLATORS;
                    frequency = base_freq * (float)i;
                    float phase_stride = frequency * sample_duration;
                    osc[i].phase_stride = phase_stride;
                    accumulateSignal(signal, &osc[i], 1.0f / NUM_OSCILLATORS);
                }
            }
            UpdateAudioStream(synthStream, signal, STREAM_BUFFER_SIZE);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(TextFormat("Freq: %f", frequency), 100, 100, 20, RED);
        for (size_t i = 0; i < 1024; i++) {
            DrawPixel((int )i, (728/2) + (int)(signal[i] * 100), RED);
        }
        EndDrawing();
    }

    CloseAudioStream(synthStream);
    CloseAudioDevice();
    CloseWindow();
}