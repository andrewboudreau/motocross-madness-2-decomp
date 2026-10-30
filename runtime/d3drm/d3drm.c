/*
 * Replacement d3drm.dll for the MCM2 demo executable.
 *
 * Retail imports exactly one function from d3drm.dll: D3DRMVectorRotate
 * (hint 17, called by FollowCamera). Windows Vista and later no longer ship
 * d3drm.dll, so without this module the demo does not start. Because the
 * loader maps it before WinMain runs, it is also where the runtime installs
 * its hooks (none yet).
 *
 * Writes mcm2_runtime.log next to the executable.
 */
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "runtime.h"
#include "sha256.h"

typedef struct D3DVector {
    float x, y, z;
} D3DVector;

/* SHA-256 of the demo build the reconstruction targets (docs/REBUILD_GAPS.md). */
static const char kRetailSha256[] =
    "31fde4cc686a5ee89ef9095b90235325b195596867ecacefe511263e1509b874";

static char g_logPath[MAX_PATH];
static LONG g_rotateCalls;
static int g_retailVerified;

void LogLine(const char* format, ...)
{
    char line[512];
    va_list args;
    SYSTEMTIME now;
    HANDLE file;
    DWORD written;
    int length;

    if (!g_logPath[0])
        return;
    GetLocalTime(&now);
    length = _snprintf(line, sizeof(line), "%02d:%02d:%02d.%03d ", now.wHour, now.wMinute,
                       now.wSecond, now.wMilliseconds);
    va_start(args, format);
    length += _vsnprintf(line + length, sizeof(line) - length - 2, format, args);
    va_end(args);
    if (length < 0 || length > (int)sizeof(line) - 2)
        length = sizeof(line) - 2;
    line[length++] = '\r';
    line[length++] = '\n';

    file = CreateFileA(g_logPath, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return;
    WriteFile(file, line, length, &written, NULL);
    CloseHandle(file);
}

static int HashFile(const char* path, char hex[65])
{
    static const char digits[] = "0123456789abcdef";
    static unsigned char buffer[65536];
    unsigned char digest[32];
    Sha256 hash;
    HANDLE file;
    DWORD got;
    int i;

    file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
        return 0;
    Sha256Init(&hash);
    while (ReadFile(file, buffer, sizeof(buffer), &got, NULL) && got)
        Sha256Update(&hash, buffer, got);
    CloseHandle(file);
    Sha256Final(&hash, digest);
    for (i = 0; i < 32; i++) {
        hex[i * 2] = digits[digest[i] >> 4];
        hex[i * 2 + 1] = digits[digest[i] & 15];
    }
    hex[64] = 0;
    return 1;
}

/*
 * TrackGameMode::FindDataDirectory (0x00523c90) refuses to start ("Installation
 * error", string 0x13b3) unless the installer's InstallType and
 * HardDriveRootPath values exist; InstallType "Full" also skips the CD check
 * (FindCdDirectory). When neither is present, record the executable's directory
 * as a full install. The executable has no manifest, so without elevation
 * Windows redirects these HKLM writes to the user's VirtualStore, where the
 * game's own reads find them.
 */
static void ProvisionInstallKeys(const char* exeDirectory)
{
    static const char kKey[] = "Software\\Microsoft\\Microsoft Games\\Motocross Madness 2 Trial";
    static const char kFull[] = "Full";
    char existing[MAX_PATH];
    DWORD size = sizeof(existing);
    HKEY key;
    LONG status;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, kKey, 0, KEY_READ, &key) == ERROR_SUCCESS) {
        status = RegQueryValueExA(key, "HardDriveRootPath", NULL, NULL, (BYTE*)existing, &size);
        RegCloseKey(key);
        if (status == ERROR_SUCCESS) {
            LogLine("install keys present (HardDriveRootPath %s)", existing);
            return;
        }
    }
    status = RegCreateKeyExA(HKEY_LOCAL_MACHINE, kKey, 0, "", 0, KEY_ALL_ACCESS, NULL, &key, NULL);
    if (status != ERROR_SUCCESS) {
        LogLine("could not create the install keys (error %ld)", status);
        return;
    }
    RegSetValueExA(key, "InstallType", 0, REG_SZ, (const BYTE*)kFull, sizeof(kFull));
    RegSetValueExA(key, "HardDriveRootPath", 0, REG_SZ, (const BYTE*)exeDirectory,
                   strlen(exeDirectory) + 1);
    RegCloseKey(key);
    LogLine("recorded a portable full install at %s", exeDirectory);
}

static void Startup(void)
{
    char exePath[MAX_PATH];
    char hex[65];
    char* slash;
    OSVERSIONINFOA os;

    GetModuleFileNameA(NULL, exePath, sizeof(exePath));
    strcpy(g_logPath, exePath);
    slash = strrchr(g_logPath, '\\');
    strcpy(slash ? slash + 1 : g_logPath, "mcm2_runtime.log");

    os.dwOSVersionInfoSize = sizeof(os);
    GetVersionExA(&os);
    LogLine("mcm2 runtime d3drm.dll loaded into %s (Windows reports %lu.%lu build %lu)", exePath,
            os.dwMajorVersion, os.dwMinorVersion, os.dwBuildNumber);

    if (!HashFile(exePath, hex)) {
        LogLine("could not read the executable to hash it");
        return;
    }
    g_retailVerified = strcmp(hex, kRetailSha256) == 0;
    LogLine("executable sha256 %s (%s)", hex,
            g_retailVerified ? "matches the demo build" : "UNKNOWN build; hooks stay off");
    /* A source-built executable has other addresses but the same imports; the
       import hooks and install keys still apply when asked for. */
    if (!g_retailVerified && !GetEnvironmentVariableA("MCM2_FORCE_RUNTIME", NULL, 0))
        return;

    slash = strrchr(exePath, '\\');
    if (slash)
        *slash = 0;
    ProvisionInstallKeys(exePath);
    InstallStartupTrace();
    InstallSampler();
}

static void Normalize(D3DVector* v)
{
    float length = (float)sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
    if (length > 0.0f) {
        v->x /= length;
        v->y /= length;
        v->z /= length;
    }
}

/*
 * Rotates v by theta radians about axis and returns the result in r, scaled
 * to unit length. The unit-length result is what FollowCamera relies on: it
 * multiplies the rotated direction by its follow distance afterwards.
 */
D3DVector* __stdcall D3DRMVectorRotate(D3DVector* r, D3DVector* v,
                                     D3DVector* axis, float theta)
{
    D3DVector k = *axis;
    D3DVector in = *v;
    D3DVector cross;
    float c, s, dot;

    if (InterlockedIncrement(&g_rotateCalls) == 1)
        LogLine("first D3DRMVectorRotate call (camera is running)");

    /* Rodrigues: v cos + (k x v) sin + k (k . v)(1 - cos). */
    Normalize(&k);
    c = (float)cos(theta);
    s = (float)sin(theta);
    dot = k.x * in.x + k.y * in.y + k.z * in.z;
    cross.x = k.y * in.z - k.z * in.y;
    cross.y = k.z * in.x - k.x * in.z;
    cross.z = k.x * in.y - k.y * in.x;
    r->x = in.x * c + cross.x * s + k.x * dot * (1.0f - c);
    r->y = in.y * c + cross.y * s + k.y * dot * (1.0f - c);
    r->z = in.z * c + cross.z * s + k.z * dot * (1.0f - c);
    Normalize(r);
    return r;
}

/* Stand-in for EBUEula.dll's entry point, used only when MCM2_AUTO_EULA is set
   (trace.c redirects the game's LoadLibraryA("EBUEula.dll") here): unattended
   test launches skip the dialog. The user opts in per launch; without the
   variable the game shows the real dialog. */
int __cdecl EBUEula(const char* key, const char* path, int a, int b)
{
    return 1;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
        Startup();
    } else if (reason == DLL_PROCESS_DETACH) {
        LogLine("unloading after %ld D3DRMVectorRotate calls", g_rotateCalls);
    }
    return TRUE;
}
