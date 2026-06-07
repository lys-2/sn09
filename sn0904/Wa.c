#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <math.h>
#include "sn.h"
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib") 

HWAVEOUT wave_out;
#define SAMPLING_RATE 44100
#define CHUNK_SIZE 2222
WAVEHDR header[2] = { 0 };
short chunks[2][CHUNK_SIZE];
char chunk_swap = 0;
float frequency = 659.25, vol = .0;
double wave_position = 0;
float wave_step;
WAVEFORMATEX format = {
            .wFormatTag = WAVE_FORMAT_PCM,
            .nChannels = 1,
            .nSamplesPerSec = SAMPLING_RATE,
            .wBitsPerSample = 16,
            .cbSize = 0,
};

void CALLBACK WaveOutProc(HWAVEOUT wave_out_handle, UINT message,
    DWORD_PTR instance, DWORD_PTR param1, DWORD_PTR param2) {
    switch (message) {
    case WOM_CLOSE: printf("WOM_CLOSE\n"); break;
    case WOM_OPEN:  printf("WOM_OPEN\n");  break;
    case WOM_DONE: {
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            chunks[chunk_swap][i] = sound2();
            wave_position += wave_step;
        }
        waveOutWrite(wave_out, &header[chunk_swap],
            sizeof(header[chunk_swap]));
        chunk_swap = !chunk_swap;
    } break;
    }
}

void waudio() {
    format.nBlockAlign = format.nChannels * format.wBitsPerSample / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    waveOutOpen(&wave_out, WAVE_MAPPER, &format,
        (DWORD_PTR)WaveOutProc, (DWORD_PTR)NULL, CALLBACK_FUNCTION);

    waveOutSetVolume(wave_out, 0xFFFFFFFF);
    wave_step = 6.283 / ((float)SAMPLING_RATE / 333.);
    for (int i = 0; i < 2; ++i) {

        header[i].lpData = (CHAR*)chunks[i];
        header[i].dwBufferLength = CHUNK_SIZE * 2;
        waveOutPrepareHeader(wave_out, &header[i], sizeof(header[i]));
        for (int i = 0; i < CHUNK_SIZE; ++i) {
            chunks[chunk_swap][i] = sin(wave_position) * i * vol;
            wave_position += wave_step;
        }
        waveOutWrite(wave_out, &header[i], sizeof(header[i]));

    }

}