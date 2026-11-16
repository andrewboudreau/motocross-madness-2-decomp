/*
 * Repeatable runs, so that retail and a source build can be compared frame by
 * frame (docs/BUILD_TO_RACE.md items 10 and 11).
 *
 * Fixed clock (MCM2_FIXED_CLOCK=<milliseconds per frame>). Every game timer
 * reads the millisecond clock at 0x4bfa80, whose only import call is
 * timeGetTime, and every srand in the game is seeded from that clock. With the
 * variable set, timeGetTime returns a virtual time instead:
 *   - each presented frame moves it to start + frame * step;
 *   - within a frame, every 256th call from the game thread adds a
 *     millisecond, so loops that wait for the clock still finish;
 *   - calls from other threads read it without moving it.
 * Two builds that make the same calls on the game thread therefore see the
 * same times and the same random numbers. Wall-clock pacing (MCM2_FPS) is
 * separate and only affects how long the run takes.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "runtime.h"

#define CLOCK_START 1000000UL
#define CALLS_PER_TICK 256

static int s_clockState; /* 0 unread, 1 on, 2 off */
static DWORD s_clockStep;
static DWORD s_clockNow = CLOCK_START;
static DWORD s_clockFrames;
static DWORD s_clockCalls;
static DWORD s_gameThread;

void SetGameThread(void)
{
    s_gameThread = GetCurrentThreadId();
}

int FixedClockEnabled(void)
{
    if (s_clockState == 0) {
        char value[16];
        s_clockState = 2;
        if (GetEnvironmentVariableA("MCM2_FIXED_CLOCK", value, sizeof(value))) {
            s_clockStep = (DWORD)atoi(value);
            if (s_clockStep) {
                s_clockState = 1;
                LogLine("clock: fixed, %lu ms a frame from %lu (MCM2_FIXED_CLOCK)", s_clockStep,
                        CLOCK_START);
            }
        }
    }
    return s_clockState == 1;
}

DWORD FixedClockNow(void)
{
    if (GetCurrentThreadId() == s_gameThread && ++s_clockCalls % CALLS_PER_TICK == 0)
        s_clockNow++;
    return s_clockNow;
}

void FixedClockFrame(void)
{
    DWORD target;
    if (!FixedClockEnabled())
        return;
    s_clockFrames++;
    s_clockCalls = 0;
    target = CLOCK_START + s_clockFrames * s_clockStep;
    if ((LONG)(target - s_clockNow) > 0)
        s_clockNow = target;
}

DWORD FixedClockFrames(void)
{
    return s_clockFrames;
}

/* ---- game symbols ----
 *
 * A source build is read through the mcm2.map beside it; retail through the
 * addresses below, which tests/test_runtime_replay.py checks against the
 * repository's *.bindings.json. */

typedef struct RetailSymbol {
    const char* name;
    DWORD address;
} RetailSymbol;

static const RetailSymbol kRetailSymbols[] = {
    {"?g_TrackGame@@3PAVTrackGame@@A", 0x0056e26c},
    {"?g_LoadingEventLoaded@@3HA", 0x0059ae84},
    {"?SelectSeries@TrackGameMode@@QAEXH@Z", 0x005240e0},
    {"?LoadControllerChoice@TrackGameMode@@QAEXXZ", 0x00523000},
    {"?FindInputDialog@GUIManager@@QAEPAVUIDialog@@XZ", 0x00485df0},
    {"?LeaveMenusForRace@@YAXXZ", 0x004536e0},
    {"?LeaveRaceForMenu@EventManager@@QAEXH@Z", 0x0045e710},
};

DWORD GameSymbol(const char* name)
{
    char line[1024], path[MAX_PATH], *slash, *at;
    size_t length = strlen(name);
    DWORD address = 0;
    FILE* map;
    int i;

    if (IsRetailDemo()) {
        for (i = 0; i < sizeof(kRetailSymbols) / sizeof(kRetailSymbols[0]); i++)
            if (!strcmp(kRetailSymbols[i].name, name))
                return kRetailSymbols[i].address;
        return 0;
    }
    GetModuleFileNameA(NULL, path, sizeof(path));
    slash = strrchr(path, 0x5c);
    lstrcpyA(slash ? slash + 1 : path, "mcm2.map");
    map = fopen(path, "r");
    if (!map)
        return 0;
    while (!address && fgets(line, sizeof(line), map)) {
        /* " 0001:00012340       ?Name@@...      00413340 f   Object.obj" */
        at = strstr(line, name);
        if (!at || at == line || at[-1] != ' ' || (at[length] != ' ' && at[length] != 9))
            continue;
        at += length;
        while (*at == ' ' || *at == 9)
            at++;
        address = (DWORD)strtoul(at, NULL, 16);
    }
    fclose(map);
    return address;
}

/* ---- layout ----
 *
 * Byte offsets read and called through below. tests/test_runtime_replay.py
 * compiles offsetof() of each from the reconstructed headers with VC6 and
 * compares it with these values. */

#define TRACKGAME_PLAYER_COUNT 0x18          /* Game::playerCount */
#define TRACKGAME_SUPERCROSS_EVENT 0x558     /* first of the five event owners */
#define TRACKGAME_EVENT_COUNT 5
#define TRACKGAME_UI 0x56c                   /* TrackGame::ui */
#define TRACKGAME_EVENT_MANAGER 0x570        /* TrackGame::eventManager */
#define TRACKGAME_MODE 0x578                 /* TrackGame::mode */
#define TRACKGAME_RACE_SETTINGS 0x2d70       /* TrackGame::mode.raceSettings */
#define RACE_SETTINGS_SIZE 0x1ec             /* sizeof(RaceSettings) */
#define TRACKGAME_RACE_MODE 0x2d70           /* mode.raceSettings.raceMode */
#define TRACKGAME_EVENT_TYPE 0x2d74          /* mode.raceSettings.eventType */
#define TRACKGAME_AI_RACER_COUNT 0x2d94      /* mode.raceSettings.aiRacerCount */
#define TRACKGAME_UI_BLOCKED 0x3430          /* TrackGame::uiInteractionBlocked */
#define UI_GUI 0x2c                          /* KrustyUI::gui */
#define UI_CURRENT_MENU 0x3c                 /* KrustyUI::currentMenu */
#define EVENT_RACE_VIEW 0x34                 /* TrackGameViewOwner::raceView */
#define EVENT_ELAPSED_MINUTES 0x70           /* TrackGameViewOwner::elapsedMinutes */
#define EVENT_ELAPSED_SECONDS 0x74           /* TrackGameViewOwner::elapsedSeconds */
#define RACE_LOCAL_RACER 0x38                /* BikeRace::localRacer */
#define RACE_AI_RACERS 0x40                  /* BikeRace::aiRacers */
#define BIKE_POSITION 0x0c                   /* SoultreePhysicsBaseObject::position */
#define BIKE_VELOCITY 0x64                   /* SoultreePhysicsBaseObject::velocity */
#define BIKE_LINEAR_SPEED 0xbc               /* SoultreePhysicsBaseObject::linearSpeed */
#define BIKE_TOTAL_RUNNING_TIME 0x754        /* KrustyBike::totalRunningTime */
#define BIKE_FINISH_POSITION 0x784           /* KrustyBike::finishPosition */
#define BIKE_GATES_PASSED 0x790              /* KrustyBike::gatesPassed */
#define BIKE_LAP_COUNT 0x7a0                 /* KrustyBike::lapCount (short) */
#define GAMEOBJECT_MARK_PENDING_RELEASE 26   /* GameObject vtable slot */
#define MENU_MAIN 100                        /* kMenuMain */
#define MENU_LOADING 104                     /* kMenuLoading */

#define AT(base, offset, type) (*(type*)((BYTE*)(base) + (offset)))

/* ---- retail autorace ----
 *
 * The same steps as src/bringup/Group9_AutoRace.cpp, which a source build
 * runs itself: retail has no AutoRaceTick. Called once per presented frame
 * on the game thread with MCM2_AUTORACE set (MCM2_AUTORACE_PORT=1 uses it
 * on a source build as well). */

typedef void(__fastcall* ThisVoid)(void* self, void* unused);
typedef void(__fastcall* ThisInt)(void* self, void* unused, int value);
typedef void*(__fastcall* ThisPointer)(void* self, void* unused);
typedef void(__cdecl* PlainVoid)(void);

static void AppendAutoRaceLog(const char* text)
{
    FILE* log = fopen("autorace.log", "a");
    if (log) {
        fputs(text, log);
        fclose(log);
    }
}

void PortedAutoRaceTick(void)
{
    static int s_resolved, s_saved, s_started, s_mainMenuTicks, s_lastMenu = -1;
    static int s_raceTicks, s_left;
    static BYTE** s_trackGame;
    static int* s_loadingEventLoaded;
    static ThisInt s_selectSeries, s_leaveRaceForMenu;
    static ThisVoid s_loadControllerChoice;
    static ThisPointer s_findInputDialog;
    static PlainVoid s_leaveMenusForRace;
    BYTE* game;
    BYTE* ui;
    int menu;
    char text[32];
    FILE* f;

    if (!s_resolved) {
        s_resolved = 1;
        s_trackGame = (BYTE**)GameSymbol("?g_TrackGame@@3PAVTrackGame@@A");
        s_loadingEventLoaded = (int*)GameSymbol("?g_LoadingEventLoaded@@3HA");
        s_selectSeries = (ThisInt)GameSymbol("?SelectSeries@TrackGameMode@@QAEXH@Z");
        s_loadControllerChoice = (ThisVoid)GameSymbol("?LoadControllerChoice@TrackGameMode@@QAEXXZ");
        s_findInputDialog = (ThisPointer)GameSymbol("?FindInputDialog@GUIManager@@QAEPAVUIDialog@@XZ");
        s_leaveMenusForRace = (PlainVoid)GameSymbol("?LeaveMenusForRace@@YAXXZ");
        s_leaveRaceForMenu = (ThisInt)GameSymbol("?LeaveRaceForMenu@EventManager@@QAEXH@Z");
        if (!s_trackGame || !s_loadingEventLoaded || !s_selectSeries || !s_loadControllerChoice ||
            !s_findInputDialog || !s_leaveMenusForRace || !s_leaveRaceForMenu) {
            LogLine("autorace: a game symbol is missing; the ported tick stays off");
            s_trackGame = NULL;
        } else {
            LogLine("autorace: ported tick on (g_TrackGame cell at 0x%08lx)", (DWORD)s_trackGame);
        }
    }
    if (!s_trackGame)
        return;
    game = *s_trackGame;
    if (!game || !AT(game, TRACKGAME_UI, BYTE*))
        return;
    ui = AT(game, TRACKGAME_UI, BYTE*);
    menu = AT(ui, UI_CURRENT_MENU, int);
    if (menu != s_lastMenu) {
        s_lastMenu = menu;
        sprintf(text, "menu %d\n", menu);
        AppendAutoRaceLog(text);
    }
    if ((menu == MENU_LOADING || *s_loadingEventLoaded) && !s_saved) {
        s_saved = 1;
        f = fopen("autorace.bin", "rb");
        if (f) {
            fclose(f);
        } else if ((f = fopen("autorace.bin", "wb")) != NULL) {
            fwrite(game + TRACKGAME_RACE_SETTINGS, RACE_SETTINGS_SIZE, 1, f);
            fclose(f);
        }
    }
    if (s_started && !s_left && menu != MENU_MAIN && menu != MENU_LOADING) {
        const char* exitEnv = getenv("MCM2_AUTORACE_EXIT");
        if (exitEnv && ++s_raceTicks >= atoi(exitEnv) * 60) {
            s_left = 1;
            s_leaveRaceForMenu(AT(game, TRACKGAME_EVENT_MANAGER, void*), NULL, MENU_MAIN);
        }
    }
    if (menu == MENU_MAIN && !s_started) {
        void* mainDialog;
        if (++s_mainMenuTicks < 120)
            return;
        f = fopen("autorace.bin", "rb");
        if (!f)
            return;
        s_started = 1;
        AppendAutoRaceLog("starting\n");
        fread(game + TRACKGAME_RACE_SETTINGS, RACE_SETTINGS_SIZE, 1, f);
        fclose(f);
        s_selectSeries(game + TRACKGAME_MODE, NULL, AT(game, TRACKGAME_EVENT_TYPE, int));
        s_loadControllerChoice(game + TRACKGAME_MODE, NULL);
        mainDialog = s_findInputDialog(AT(ui, UI_GUI, void*), NULL);
        s_leaveMenusForRace();
        if (mainDialog)
            ((ThisVoid)(*(void***)mainDialog)[GAMEOBJECT_MARK_PENDING_RELEASE])(mainDialog, NULL);
    }
}

/* ---- state log (MCM2_STATE_LOG=1) ----
 *
 * One line per presented frame to statelog.txt beside the executable:
 *   F <frame> T <game clock ms, 0 without the fixed clock> M <menu>
 * and, while a race view exists,
 *   R <minutes> <seconds> N <racers>, then per racer (local first, then AI)
 *   | <x> <y> <z> <vx> <vy> <vz> <speed> <laps> <gates> <place> <total time>
 * Floats are written as their raw bits in hex, so equal lines mean equal
 * bits. tools/compare_state_logs.py finds the first frame two logs differ. */

static FILE* s_stateLog;
static DWORD s_stateFrames;

static void LogRacer(char* out, BYTE* bike)
{
    sprintf(out, " | %08lx %08lx %08lx %08lx %08lx %08lx %08lx %d %d %d %08lx",
            AT(bike, BIKE_POSITION, DWORD), AT(bike, BIKE_POSITION + 4, DWORD),
            AT(bike, BIKE_POSITION + 8, DWORD), AT(bike, BIKE_VELOCITY, DWORD),
            AT(bike, BIKE_VELOCITY + 4, DWORD), AT(bike, BIKE_VELOCITY + 8, DWORD),
            AT(bike, BIKE_LINEAR_SPEED, DWORD), (int)AT(bike, BIKE_LAP_COUNT, short),
            AT(bike, BIKE_GATES_PASSED, int), AT(bike, BIKE_FINISH_POSITION, int),
            AT(bike, BIKE_TOTAL_RUNNING_TIME, DWORD));
}

static void FormatState(char* line, BYTE* game)
{
    BYTE* ui = AT(game, TRACKGAME_UI, BYTE*);
    BYTE* event = NULL;
    BYTE* race;
    BYTE** ai;
    int i, count;

    sprintf(line, "F %lu T %lu M %d", s_stateFrames, FixedClockEnabled() ? s_clockNow : 0,
            ui ? AT(ui, UI_CURRENT_MENU, int) : -1);
    for (i = 0; i < TRACKGAME_EVENT_COUNT && !event; i++)
        event = AT(game, TRACKGAME_SUPERCROSS_EVENT + 4 * i, BYTE*);
    if (!event || !(race = AT(event, EVENT_RACE_VIEW, BYTE*)))
        return;
    if (AT(game, TRACKGAME_UI_BLOCKED, int) || AT(game, TRACKGAME_PLAYER_COUNT, int) != 1) {
        strcat(line, " R blocked");
        return;
    }
    count = AT(game, TRACKGAME_RACE_MODE, int) == 4 ? 1 : AT(game, TRACKGAME_AI_RACER_COUNT, int);
    ai = AT(race, RACE_AI_RACERS, BYTE**);
    if (!ai || count < 0 || count > 15)
        count = 0;
    sprintf(line + strlen(line), " R %08lx %08lx N %d", AT(event, EVENT_ELAPSED_MINUTES, DWORD),
            AT(event, EVENT_ELAPSED_SECONDS, DWORD), count + 1);
    if (AT(race, RACE_LOCAL_RACER, BYTE*))
        LogRacer(line + strlen(line), AT(race, RACE_LOCAL_RACER, BYTE*));
    else
        strcat(line, " | none");
    for (i = 0; i < count; i++) {
        if (ai[i])
            LogRacer(line + strlen(line), ai[i]);
        else
            strcat(line, " | none");
    }
}

void LogFrameState(void)
{
    static int s_state; /* 0 untried, 1 on, 2 off */
    static BYTE** s_trackGame;
    char line[2048];

    if (s_state == 0) {
        char path[MAX_PATH], *slash;
        s_state = 2;
        if (!GetEnvironmentVariableA("MCM2_STATE_LOG", NULL, 0))
            return;
        s_trackGame = (BYTE**)GameSymbol("?g_TrackGame@@3PAVTrackGame@@A");
        GetModuleFileNameA(NULL, path, sizeof(path));
        slash = strrchr(path, 0x5c);
        lstrcpyA(slash ? slash + 1 : path, "statelog.txt");
        s_stateLog = s_trackGame ? fopen(path, "w") : NULL;
        if (!s_stateLog) {
            LogLine("state log: off (%s)",
                    s_trackGame ? "cannot write statelog.txt" : "g_TrackGame not found");
            return;
        }
        LogLine("state log: writing %s (g_TrackGame cell at 0x%08lx)", path, (DWORD)s_trackGame);
        s_state = 1;
    }
    if (s_state != 1)
        return;
    s_stateFrames++;
    line[0] = 0;
    __try {
        if (*s_trackGame)
            FormatState(line, *s_trackGame);
        else
            sprintf(line, "F %lu no game", s_stateFrames);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        strcat(line, " fault");
    }
    fputs(line, s_stateLog);
    fputc(10, s_stateLog);
    fflush(s_stateLog); /* a crash keeps every frame before it */
}
