// Execute the unchanged source FilePrintf body against the native VC6 CRT.
// The seven format families are those of its two observed callers. No retail
// instructions or FILE internals are copied into this executable.
#include "FilePrintf.h"
#include <string.h>
#include <stdlib.h>

static int checks;

static void Check(FILE* actual, FILE* expected, int result, int reference) {
    if (result != reference || result < 0 || fflush(actual) || fflush(expected))
        exit(2);
    long actualLength = ftell(actual);
    long expectedLength = ftell(expected);
    if (actualLength != expectedLength || actualLength != result)
        exit(3);
    rewind(actual);
    rewind(expected);
    for (long i = 0; i < actualLength; ++i)
        if (fgetc(actual) != fgetc(expected))
            exit(4);
    if (fgetc(actual) != EOF || fgetc(expected) != EOF ||
        ferror(actual) || ferror(expected))
        exit(5);
    ++checks;
}

int main(int argc, char** argv) {
    if (argc != 2)
        return 6;
    for (int seed = 0; seed < 32; ++seed) {
        int index = seed - 16;
        double x = (seed - 17) * 0.125;
        double y = (seed + 3) * 0.25;
        double z = (seed - 29) * 0.5;
        double radius = seed * 0.0625;
        double height = (seed + 1) * 0.03125;
        for (int kind = 0; kind < 7; ++kind) {
            FILE* actual = tmpfile();
            FILE* expected = tmpfile();
            if (!actual || !expected)
                return 7;
            int a, b;
            switch (kind) {
            case 0:
                a = FilePrintf(actual, "----------------------------------------------------------------------\n");
                b = fprintf(expected, "----------------------------------------------------------------------\n");
                break;
            case 1:
                a = FilePrintf(actual, "Texture Format: %d  Cache Textures: %d  Mgd Textures: %d\n", index, seed, 2 * seed);
                b = fprintf(expected, "Texture Format: %d  Cache Textures: %d  Mgd Textures: %d\n", index, seed, 2 * seed);
                break;
            case 2:
                a = FilePrintf(actual, "%03d: %03d x %03d, Max Req'd=%.2f", index, seed, 2 * seed, x);
                b = fprintf(expected, "%03d: %03d x %03d, Max Req'd=%.2f", index, seed, 2 * seed, x);
                break;
            case 3:
                a = FilePrintf(actual, ", Allocd=%d unhappy=%.2f", index, x);
                b = fprintf(expected, ", Allocd=%d unhappy=%.2f", index, x);
                break;
            case 4:
                a = FilePrintf(actual, "\n");
                b = fprintf(expected, "\n");
                break;
            case 5:
                a = FilePrintf(actual, "%i\n", index);
                b = fprintf(expected, "%i\n", index);
                break;
            default:
                a = FilePrintf(actual, "%i,%f,%f,%f,%f,%f\n", index, x, y, z, radius, height);
                b = fprintf(expected, "%i,%f,%f,%f,%f,%f\n", index, x, y, z, radius, height);
                break;
            }
            Check(actual, expected, a, b);
            fclose(actual);
            fclose(expected);
        }
    }

    FILE* actual = tmpfile();
    FILE* expected = tmpfile();
    if (!actual || !expected)
        return 8;
    Check(actual, expected, FilePrintf(actual, ""), fprintf(expected, ""));
    fclose(actual);
    fclose(expected);

    actual = tmpfile();
    expected = tmpfile();
    if (!actual || !expected)
        return 8;
    int written = -1;
    int referenceWritten = -1;
    int a = FilePrintf(actual, "%s%c%n%*.*f", "prefix", 0, &written, 8, 3, 1.125);
    int b = fprintf(expected, "%s%c%n%*.*f", "prefix", 0, &referenceWritten, 8, 3, 1.125);
    if (written != 7 || referenceWritten != 7)
        return 9;
    Check(actual, expected, a, b);
    fclose(actual);
    fclose(expected);

    // An actual read-only FILE, not a fabricated FILE layout or mocked error.
    actual = fopen(argv[1], "wb");
    if (!actual || fputs("unchanged", actual) == EOF || fclose(actual))
        return 10;
    actual = fopen(argv[1], "rb");
    if (!actual || setvbuf(actual, 0, _IONBF, 0))
        return 11;
    if (FilePrintf(actual, "%s", "blocked") >= 0 || !ferror(actual))
        return 12;
    fclose(actual);
    actual = fopen(argv[1], "rb");
    char content[16] = {0};
    if (!actual || fread(content, 1, sizeof(content), actual) != 9 ||
        strcmp(content, "unchanged") || fclose(actual))
        return 13;
    ++checks;

    printf("FilePrintf: %d real CRT output, return and stream-error cases pass\n", checks);
    return 0;
}
