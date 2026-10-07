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
// reloads 11025 * field_0x1298 as retail does.

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
    streamChannel->channelSound->UnknownFunction4bcd40(&play, &write);
    if (streamChannel->field_0x1c == 1) {
        if (write < (unsigned long)(16537 * field_0x1298))
            return;
        streamChannel->channelSound->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (streamChannel->field_0x20 == 0) {
            half = 11025 * field_0x1298;
            avail = streamChannel->field_0x14 - streamChannel->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->field_0x18, avail);
                memcpy((char*)streamChannel->sampleBuffer + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->field_0x18, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 += half;
            }
        } else {
            half = 11025 * field_0x1298;
            avail = streamChannel->field_0x14 - streamChannel->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->field_0x18, avail);
                switch (streamChannel->field_0x20) {
                case 1:
                    j = rand() % field_0x1128[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0x840[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xc00[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 1;
                    streamChannel->field_0x20 = 0;
                    break;
                case 3:
                    j = rand() % field_0x1158[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xa20[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xde0[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 3;
                    streamChannel->field_0x20 = 4;
                    break;
                case 4:
                    j = rand() % field_0x1170[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xb10[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xed0[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 4;
                    streamChannel->field_0x20 = 3;
                    break;
                }
                memcpy((char*)streamChannel->sampleBuffer + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer, (char*)streamChannel->streamSample + streamChannel->field_0x18, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 += half;
            }
        }
        streamChannel->channelSound->UnknownFunction4bd080(first, half, second, 0);
        streamChannel->field_0x1c = 2;
    } else if (streamChannel->field_0x1c == 2) {
        if (write < (unsigned long)(5512 * field_0x1298) || write > (unsigned long)(11025 * field_0x1298))
            return;
        streamChannel->channelSound->UnknownFunction4bd020(0, 0, &first, &firstBytes, &second, &secondBytes, 2);
        if (streamChannel->field_0x20 == 0) {
            half = 11025 * field_0x1298;
            avail = streamChannel->field_0x14 - streamChannel->field_0x18;
            if (half > avail) {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->field_0x18, avail);
                memcpy((char*)streamChannel->sampleBuffer + 11025 * field_0x1298 + avail, streamChannel->streamSample, half - avail);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 = half - avail;
            } else {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->field_0x18, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 += half;
            }
        } else {
            half = 11025 * field_0x1298;
            avail = streamChannel->field_0x14 - streamChannel->field_0x18;
            if (half > avail) {
                rest = half - avail;
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->field_0x18, avail);
                switch (streamChannel->field_0x20) {
                case 1:
                    j = rand() % field_0x1128[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0x840[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xc00[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 1;
                    streamChannel->field_0x20 = 0;
                    break;
                case 3:
                    j = rand() % field_0x1158[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xa20[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xde0[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 3;
                    streamChannel->field_0x20 = 4;
                    break;
                case 4:
                    j = rand() % field_0x1170[streamChannel->field_0x0c];
                    streamChannel->streamSample = field_0xb10[streamChannel->field_0x0c][j];
                    streamChannel->field_0x18 = 0;
                    streamChannel->field_0x14 = field_0xed0[streamChannel->field_0x0c][j];
                    streamChannel->field_0x10 = 4;
                    streamChannel->field_0x20 = 3;
                    break;
                }
                memcpy((char*)streamChannel->sampleBuffer + 11025 * field_0x1298 + avail, streamChannel->streamSample, rest);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 = rest;
            } else {
                memcpy((char*)streamChannel->sampleBuffer + half, (char*)streamChannel->streamSample + streamChannel->field_0x18, half);
                memcpy(first, streamChannel->sampleBuffer, 22050 * field_0x1298);
                streamChannel->field_0x18 += half;
            }
        }
        streamChannel->channelSound->UnknownFunction4bd080(first, 22050 * field_0x1298, second, 0);
        streamChannel->field_0x1c = 1;
    }
}

