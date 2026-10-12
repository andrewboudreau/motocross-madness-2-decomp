/* Minimal SHA-256 (FIPS 180-4) for checking the executable at load time. */
#ifndef MCM2_RUNTIME_SHA256_H
#define MCM2_RUNTIME_SHA256_H

typedef struct Sha256 {
    unsigned long state[8];
    unsigned long bitsLow, bitsHigh;
    unsigned char block[64];
    unsigned int used;
} Sha256;

static const unsigned long kSha256Round[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

#define SHA256_ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static void Sha256Block(Sha256* h, const unsigned char* p)
{
    unsigned long w[64], a, b, c, d, e, f, g, k, t1, t2;
    int i;

    for (i = 0; i < 16; i++)
        w[i] = ((unsigned long)p[i * 4] << 24) | ((unsigned long)p[i * 4 + 1] << 16) |
               ((unsigned long)p[i * 4 + 2] << 8) | p[i * 4 + 3];
    for (i = 16; i < 64; i++) {
        t1 = SHA256_ROR(w[i - 2], 17) ^ SHA256_ROR(w[i - 2], 19) ^ (w[i - 2] >> 10);
        t2 = SHA256_ROR(w[i - 15], 7) ^ SHA256_ROR(w[i - 15], 18) ^ (w[i - 15] >> 3);
        w[i] = t1 + w[i - 7] + t2 + w[i - 16];
    }
    a = h->state[0]; b = h->state[1]; c = h->state[2]; d = h->state[3];
    e = h->state[4]; f = h->state[5]; g = h->state[6]; k = h->state[7];
    for (i = 0; i < 64; i++) {
        t1 = k + (SHA256_ROR(e, 6) ^ SHA256_ROR(e, 11) ^ SHA256_ROR(e, 25)) +
             ((e & f) ^ (~e & g)) + kSha256Round[i] + w[i];
        t2 = (SHA256_ROR(a, 2) ^ SHA256_ROR(a, 13) ^ SHA256_ROR(a, 22)) +
             ((a & b) ^ (a & c) ^ (b & c));
        k = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    h->state[0] += a; h->state[1] += b; h->state[2] += c; h->state[3] += d;
    h->state[4] += e; h->state[5] += f; h->state[6] += g; h->state[7] += k;
}

static void Sha256Init(Sha256* h)
{
    h->state[0] = 0x6a09e667; h->state[1] = 0xbb67ae85;
    h->state[2] = 0x3c6ef372; h->state[3] = 0xa54ff53a;
    h->state[4] = 0x510e527f; h->state[5] = 0x9b05688c;
    h->state[6] = 0x1f83d9ab; h->state[7] = 0x5be0cd19;
    h->bitsLow = h->bitsHigh = 0;
    h->used = 0;
}

static void Sha256Update(Sha256* h, const unsigned char* data, unsigned long size)
{
    unsigned long before = h->bitsLow;
    h->bitsLow += size << 3;
    h->bitsHigh += (size >> 29) + (h->bitsLow < before);
    while (size--) {
        h->block[h->used++] = *data++;
        if (h->used == 64) {
            Sha256Block(h, h->block);
            h->used = 0;
        }
    }
}

static void Sha256Final(Sha256* h, unsigned char digest[32])
{
    unsigned long low = h->bitsLow, high = h->bitsHigh;
    unsigned char pad = 0x80, zero = 0, length[8];
    int i;

    Sha256Update(h, &pad, 1);
    while (h->used != 56)
        Sha256Update(h, &zero, 1);
    for (i = 0; i < 4; i++) {
        length[i] = (unsigned char)(high >> (24 - i * 8));
        length[4 + i] = (unsigned char)(low >> (24 - i * 8));
    }
    Sha256Update(h, length, 8);
    for (i = 0; i < 32; i++)
        digest[i] = (unsigned char)(h->state[i >> 2] >> (24 - (i & 3) * 8));
}

#endif
