#include <math.h>
#include "raylib.h"

#define SAMPLE_RATE 44100
#define STREAM_BUFFER_SIZE 1024

typedef struct {
    float phase;
    float phase_stride;
} Oscillator;

void updateSignal(float* signal, Oscillator* osc)
{
    for (size_t t = 0; t < STREAM_BUFFER_SIZE; t++) {
        osc->phase += osc->phase_stride;
        if(osc->phase >=  1.0f)
            osc->phase -= 1.0f;
        signal[t] = sinf(2.0f * PI * osc->phase);
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

    float frequency = 5.0f;
    float sample_duration = (1.0f/(float)sample_rate);

    Oscillator osc = {.phase = 0.0f, .phase_stride = frequency * sample_duration};
    Oscillator lfo = {.phase = 0.0f, .phase_stride = frequency * sample_duration};

    float signal[STREAM_BUFFER_SIZE];

    while (WindowShouldClose() == false)
    {
        if(IsAudioStreamProcessed(synthStream))
        {
            updateSignal(signal, &osc);
            UpdateAudioStream(synthStream, signal, STREAM_BUFFER_SIZE);
            frequency += 0.7f;
            osc.phase_stride = frequency * sample_duration;
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