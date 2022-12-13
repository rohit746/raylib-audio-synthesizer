#include <math.h>
#include "raylib.h"

#define SAMPLE_RATE 44100
#define SAMPLE_DURATION (1.0f / SAMPLE_RATE)
#define STREAM_BUFFER_SIZE 1024
#define NUM_OSCILLATORS 16

typedef struct {
    float phase;
    float freq;
    float amplitude;
} Oscillator;


void updateOsc(Oscillator* osc, float freq_modulation)
{
    osc->phase += ((osc->freq + freq_modulation) * SAMPLE_DURATION);
    if(osc->phase >=  1.0f)
        osc->phase -= 1.0f;
    if(osc->phase < 0.0f)
        osc->phase += 1.0f;
}

void zeroSignal(float* signal)
{
    for (size_t t = 0; t < STREAM_BUFFER_SIZE; t++) {
        signal[t] = 0.0f;
    }
}

float sineWaveOsc(Oscillator* osc)
{
    return sinf(2.0f * PI * osc->phase) * osc->amplitude;
}

void accumulateSignal(float* signal, Oscillator* osc, Oscillator* lfo)
{
    for (size_t t = 0; t < STREAM_BUFFER_SIZE; t++) {
        updateOsc(lfo, 0.0f);
        updateOsc(osc, sineWaveOsc(lfo));
        signal[t] += sineWaveOsc(osc);
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

    Oscillator osc[NUM_OSCILLATORS] = {0};
    Oscillator lfo = {.phase = 0.0f};
    lfo.freq = 3.0f;
    lfo.amplitude = 50.0f;
    float signal[STREAM_BUFFER_SIZE];

    while (WindowShouldClose() == false)
    {
        Vector2 mouse_pos = GetMousePosition();
        float normalized_mouse_x = (mouse_pos.x / (float)screen_width);
        float base_freq = 25.0f + (normalized_mouse_x * 400.0f);

        if(IsAudioStreamProcessed(synthStream))
        {
            zeroSignal(signal);
            for (size_t i = 0; i < NUM_OSCILLATORS; ++i)
            {
                if(i % 2 != 0)
                {
                    //float normalized_index = (float)i / NUM_OSCILLATORS;
                    const float frequency = base_freq * (float)i;
                    osc[i].freq = base_freq * (float) i;
                    osc[i].amplitude = 1.0f / NUM_OSCILLATORS;
                    accumulateSignal(signal, &osc[i], &lfo);
                }
            }
            UpdateAudioStream(synthStream, signal, STREAM_BUFFER_SIZE);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        for (size_t i = 0; i < 1024; i++) {
            DrawPixel((int )i, (728/2) + (int)(signal[i] * 100), RED);
        }
        DrawFPS(10,10);
        EndDrawing();
    }

    CloseAudioStream(synthStream);
    CloseAudioDevice();
    CloseWindow();
}