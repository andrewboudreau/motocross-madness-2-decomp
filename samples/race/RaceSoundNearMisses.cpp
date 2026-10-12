// Near-miss racesnd.cpp candidates, kept out of src/reconstructed until they
// match. See docs/RACESOUND.md. The canonical file is included first so the
// TU-local declarations and globals are the same.
//
// 0x004e4d10 (2032 bytes, the half-buffer refill; 0x004e39b0 is exact in
// RaceSound.cpp): every block but two matches. (1) First half, next-sample
// path: retail reloads the remainder from its stack slot for the final
// `field_0x18 = rest` (so that store is cross-jumped with the other two
// arms' stores); VC6 here keeps the loaded remainder in eax across the two
// copies. (2) Second half, looping path: retail keeps half in ebp and the
// remainder in edx and spills the available byte count, VC6 here spills a
// temporary and computes the remainder in ebp. The second copy's destination
// reloads 11025 * bytesPerSample as retail does. Unchanged or worse: a `rest`
// local in (2) (before or after the first copy), the destination's
// association (sampleBuffer + avail + 11025 * bytesPerSample, a parenthesised
// offset, `+ half`), and block-scoped `avail`/`rest` in (1).

#include "../../src/reconstructed/RaceSound.cpp"

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

