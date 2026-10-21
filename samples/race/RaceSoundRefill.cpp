// Complete race engine-stream refill candidate, retail 0x004e4d10..0x004e54ff.
// Nonexact VC6 reconstruction; standalone grouping does not establish original TU ownership.
#include "../../src/reconstructed/RaceSound.h"
#include <stdlib.h>
#include <string.h>

// 0x004e4d10: once the play cursor has crossed into the other half, refills
// the half just played from the sample, looping it (field_0x20 0) or going
// on to a random sample of the set field_0x20 names.
void RaceSound::RefillStream() {
    void* first;
    unsigned long write;
    void* second;
    unsigned long play;
    unsigned long secondBytes;
    unsigned long firstBytes;
    int half;
    int avail;
    int rest;
    int j;

    if (!streamChannel->channelSound)
        return;
    streamChannel->channelSound->GetBufferPosition(&play, &write);
    if (streamChannel->fillHalf == 1) {
        if (write < (unsigned long)(16537 * bytesPerSample))
            return;
        streamChannel->channelSound->LockBuffer(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (streamChannel->nextSampleKind == 0) {
            half = 11025 * bytesPerSample;
            avail = streamChannel->sampleBytes - streamChannel->sampleOffset;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->sampleOffset, avail);
                memcpy((char*)streamChannel->sampleBuffer + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->sampleOffset, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset += half;
            }
        } else {
            half = 11025 * bytesPerSample;
            avail = streamChannel->sampleBytes - streamChannel->sampleOffset;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->sampleOffset, avail);
                switch (streamChannel->nextSampleKind) {
                case 1:
                    j = rand() % idleCount[streamChannel->engine];
                    streamChannel->streamSample = idleSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = idleSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 1;
                    streamChannel->nextSampleKind = 0;
                    break;
                case 3:
                    j = rand() % powerBandCount[streamChannel->engine];
                    streamChannel->streamSample = powerBandSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = powerBandSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 3;
                    streamChannel->nextSampleKind = 4;
                    break;
                case 4:
                    j = rand() % sputterCount[streamChannel->engine];
                    streamChannel->streamSample = sputterSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = sputterSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 4;
                    streamChannel->nextSampleKind = 3;
                    break;
                }
                memcpy((char*)streamChannel->sampleBuffer + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->sampleOffset, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset += half;
            }
        }
        streamChannel->channelSound->UnlockBuffer(first, half, second, 0);
        streamChannel->fillHalf = 2;
    } else if (streamChannel->fillHalf == 2) {
        if (write < (unsigned long)(5512 * bytesPerSample) || write > (unsigned long)(11025 * bytesPerSample))
            return;
        streamChannel->channelSound->LockBuffer(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (streamChannel->nextSampleKind == 0) {
            half = 11025 * bytesPerSample;
            avail = streamChannel->sampleBytes - streamChannel->sampleOffset;
            if (half > avail) {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->sampleOffset, avail);
                memcpy((char*)streamChannel->sampleBuffer + 11025 * bytesPerSample + avail, streamChannel->streamSample, half - avail);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset = half - avail;
            } else {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->sampleOffset, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset += half;
            }
        } else {
            half = 11025 * bytesPerSample;
            avail = streamChannel->sampleBytes - streamChannel->sampleOffset;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->sampleOffset, avail);
                switch (streamChannel->nextSampleKind) {
                case 1:
                    j = rand() % idleCount[streamChannel->engine];
                    streamChannel->streamSample = idleSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = idleSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 1;
                    streamChannel->nextSampleKind = 0;
                    break;
                case 3:
                    j = rand() % powerBandCount[streamChannel->engine];
                    streamChannel->streamSample = powerBandSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = powerBandSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 3;
                    streamChannel->nextSampleKind = 4;
                    break;
                case 4:
                    j = rand() % sputterCount[streamChannel->engine];
                    streamChannel->streamSample = sputterSamples[streamChannel->engine][j];
                    streamChannel->sampleOffset = 0;
                    streamChannel->sampleBytes = sputterSampleBytes[streamChannel->engine][j];
                    streamChannel->sampleKind = 4;
                    streamChannel->nextSampleKind = 3;
                    break;
                }
                memcpy((char*)streamChannel->sampleBuffer + 11025 * bytesPerSample + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->sampleOffset, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * bytesPerSample);
                streamChannel->sampleOffset += half;
            }
        }
        streamChannel->channelSound->UnlockBuffer(first, 22050 * bytesPerSample, second, 0);
        streamChannel->fillHalf = 1;
    }
}
