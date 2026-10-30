// Group6_BikeRaceCreateRacers.cpp -- best-guess BikeRace::CreateRacers (0x00419970, the
// loader, ret 0x14), body taken from samples/race/BikeRaceNearMisses.cpp
// (unreviewed; not byte-matched: 194 of retail's 195 calls in order). BikeRace::Create
// (Group6_BikeRaceCreate.cpp) calls it. Preamble as Group6_BikeRaceCreate.cpp.

#include "../krusty2/broadphase/Terrain.h"
#include "GUIManagerCore.h"
#include "../krusty2/motion/D3DIMSoultreeCharacter.h"
#include "BikeRace.h"
#include "TrackOverlay.h"

#include <stdio.h>
#include <string.h>

#include "ArcadeObject.h"
#include "D3DIMSoulTree.h"
#include "DebugAlloc.h"
#include "EventManager.h"
#include "BaseQuarryEvent.h"
#include "../krusty2/vehicle/KrustyBikeCore.h"

#include <math.h>
#include <stdlib.h>

#include "CarProcedural.h"
#include "Krusty3DObjects.h"
#include "MemTag.h"
#include "OptionProcs.h"
#include "RaceSound.h"
#include "RaceStatus.h"
#include "SelectGamePicProcs.h"
#include "TextureMap.h"

// A truncating copy into a `size`-byte buffer (as DlgProcs.cpp); retail
// evaluates `source` once for the length and again for the copy.
#define COPY_TEXT(dest, source, size)                              \
    {                                                              \
        int length = strlen(source);                               \
        int copied = length > (size) - 1 ? (size) - 1 : length;    \
        strncpy(dest, source, copied);                             \
        (dest)[copied] = 0;                                        \
    }

// The four vector constants of src/krusty2/math/Math3D.h: 0x00578e60,
// 0x00578e70, 0x00578e80 and 0x00578e50, initialised by
// 0x0041cdf0..0x0041cf2b.
static const Vector3 kVec3Zero = Vector3(0.0f, 0.0f, 0.0f);
static const Vector3 kVec3XAxis = Vector3(1.0f, 0.0f, 0.0f);
static const Vector3 kVec3YAxis = Vector3(0.0f, 1.0f, 0.0f);
static const Vector3 kVec3ZAxis = Vector3(0.0f, 0.0f, 1.0f);
// Inline: the racers in the race, own and AI (a single player race) or
// own and remote.
static inline int UnknownFunctionRacerCount() {
    if (g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            return g_TrackGame->playerCount;
        }
        return g_TrackGame->mode.raceSettings.aiRacerCount + 1;
    }
    return g_TrackGame->mode.raceSettings.multiplayerAiRacerCount + g_TrackGame->playerCount;
}

// BikeRace.cpp's TU-local vector helpers.
static inline Vector3 operator+(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline Vector3 operator-(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

#include "EventManager.h"
#include "FollowCamera.h"

// best guess: the TU-local helpers and the body below are the sample's, unreviewed.
// The only change: the sample's nine `new(__FILE__, line) UnknownBikeRaceRacer(1)`
// constructions go through UnknownBikeRaceNewRacer (Group6_KrustyBikeCtor.cpp), which
// constructs the canonical KrustyBike (0x0048fa60) and returns its primary pointer; the
// provisional view has no constructor definition of its own.
UnknownBikeRaceRacer* UnknownBikeRaceNewRacer(const char* file, int line);

// A racer's skill, written into `skill`: `base` moved up or down by at most
// `range`, closer to `base` more often.
static inline void UnknownRandomSkill(float& skill, const float& base, const float& range) {
    float r = rand() * (1.0f / 32768.0f);
    skill = (float)((1.0 - exp(-r * r)) * range);
    if (rand() * (1.0f / 32768.0f) > 0.5f)
        skill = -skill;
    skill += base;
}

// The network view of start grid entry i (BikeRace reads +0x5 as the AI
// flag); the loader reads it through g_TrackGame each time.
static inline UnknownBikeRaceNetPlayer& GridPlayer(int i) {
    return ((UnknownBikeRaceNetPlayer*)g_TrackGame->mode.raceSettings.gridOrder)[i];
}

// Event AI racer n's record in the event manager; read through g_TrackGame
// each time (retail keeps only the byte offset n * 0x50 live).
static inline UnknownBikeRaceEventAi& EventAi(int n) {
    return ((UnknownBikeRaceEventAi*)g_TrackGame->eventManager->field_0x444)[n];
}

// The front end's bike, manufacturer and rider tables, read through
// g_TrackGame at each use as retail does.
static inline UnknownBikeRaceUiChoice& UiBike(int i) {
    return ((UnknownBikeRaceUiChoice*)g_TrackGame->ui->bikes)[i];
}

static inline UnknownBikeRaceUiModel& UiManufacturer(int i) {
    return ((UnknownBikeRaceUiModel*)g_TrackGame->ui->manufacturers)[i];
}

static inline UnknownBikeRaceUiModel& UiRider(int i) {
    return ((UnknownBikeRaceUiModel*)g_TrackGame->ui->riders)[i];
}

// 0x00419970
int BikeRace::CreateRacers(void* owner, LightManager* lights, UnknownTextureFormatChoice* textures,
                                    UnknownBikeRaceProjector* terrain, UnknownBikeRaceNodeOwner* event) {
    char path[0x104];
    char message[0x184];
    char bikeFile[0x104];
    char riderFile[0x104];
    char bikeTexture[0x40];
    char riderTexture[0x40];
    char name[0x10];
    char ownBikeTexture[0x40];
    char ownRiderTexture[0x40];
    char empty[0x104];
    int order[11];
    int ranking[11];
    Vector3 position;
    Vector3 direction;
    UnknownBikeRaceBikeSetup setup;
    int playerId = 0x7fffffff;
    int recordIndex = 0;
    int id = 0;
    int i;
    float skill = aiSettings[0];
    Vector3 up(0.0f, 1.0f, 0.0f);
    empty[0] = 0;

    UnknownTextureStream* stream = new(__FILE__, 0x3b6) UnknownTextureStream(g_ResourceManager);
    sprintf(path, "%s\\rider.mcf", "Res");
    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)riderFile)) {
        sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
        delete stream;
        Release();
        return 0;
    }

    if (g_TrackGame->playingReplay) {
        if (g_TrackGame->mode.raceSettings.networkPlayerCount) {
            racerCount = g_TrackGame->mode.raceSettings.networkPlayerCount +
                          g_TrackGame->mode.raceSettings.multiplayerAiRacerCount;
            g_TrackGame->mode.raceSettings.aiRacerCount = racerCount - 1;
            g_TrackGame->mode.raceSettings.networkPlayerCount = 0;
        } else {
            racerCount = g_TrackGame->mode.raceSettings.aiRacerCount + g_TrackGame->playerCount;
        }
    } else {
        if (g_TrackGame->mode.raceSettings.raceMode == 4)
            racerCount = 1;
        else
            racerCount = UnknownFunctionRacerCount();
        recordIndex = 0;
        if (replayVcr)
            replayVcr->racerCount = racerCount;
    }
    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 2) {
            RankByPoints(ranking);
            for (int i = 0; i < racerCount; i++) {
                for (int j = 0; j < racerCount; j++) {
                    if (ranking[j] == i + 1) {
                        order[i] = j + 1;
                        break;
                    }
                }
            }
        } else {
            for (int i = 0; i < racerCount; i++)
                order[i] = i + 1;
        }
    }

    if (g_TrackGame->InProCircuitCareer()) {
        // The pro circuit: the player's racer, then the computer racers of
        // the career.
        TrackGame* game = g_TrackGame;
        UnknownTrackGameObject3444* circuit = game->proCircuit;
        setup = *(UnknownBikeRaceBikeSetup*)game->mode.garageOptions;
        setup.engineSize = circuit->racers[0].engineSize;
        setup.fourStroke = circuit->racers[0].fourStroke;
        int bikeClass = BikeClassOf(setup.engineSize);
        setup.rpmLowerLimit = game->ui->rpmLowerLimit[bikeClass];
        setup.rpmUpperLimit = game->ui->rpmUpperLimit[bikeClass];
        setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
        setup.weight = game->ui->weight[bikeClass];
        WriteRelativePositions(&position, &direction, event, order[0]);
        sprintf(path, "%s\\%s", "Res", circuit->racers[0].bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        char riderPath[0x104];
        char circuitRiderFile[0x104];
        if (circuit->careerFlags & 2) {
            sprintf(riderPath, "%s\\%s", "Res", "riderd.mcf");
        } else {
            COPY_TEXT(riderPath, riderFile, 0x104);
        }
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, riderPath, "rb",
                                                                       (int)circuitRiderFile)) {
            sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        localRacer = UnknownBikeRaceNewRacer(__FILE__, 0x42a);
        localRacer->Load(owner, "", bikeFile, circuitRiderFile, lights, textures, position,
                                           direction, up, terrain, g_TrackGame->controlInterface, 0, 0, this,
                                           g_TrackGame->mode.profileName, 0, 0, &setup,
                                           g_TrackGame->mode.gasGyro,
                                           g_TrackGame->mode.brakeGyro, particleManager, raceScene,
                                           replayVcr, 0, !(circuit->careerFlags & 2));
        AppendChild(localRacer, -1);
        g_TrackGame->ui->ResolveBikeFile(circuit->racers[0].bikeName, ownBikeTexture, 0x3f);
        localRacer->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, (int)textures);
        if (!(circuit->careerFlags & 2)) {
            g_TrackGame->ui->ResolveRiderFile(circuit->racers[0].riderModel, ownRiderTexture,
                                                             0x3f);
            localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, (int)textures);
        }
        if (g_TrackGame->mode.raceSettings.raceMode == 1) {
            id = 1;
            aiRacers = (UnknownBikeRaceRacer**)DebugMalloc(
                (g_TrackGame->proCircuit->racerCount - 1) * sizeof(UnknownBikeRaceRacer*), __FILE__,
                0x44f);
            for (int i = 0; i < g_TrackGame->proCircuit->racerCount - 1; i++)
                aiRacers[i] = 0;
            for (int n = 1; n < g_TrackGame->proCircuit->racerCount; n++) {
                UnknownProCircuitRacer* racer = &circuit->racers[n];
                WriteRelativePositions(&position, &direction, event, order[n]);
                sprintf(path, "%s\\%s", "Res", racer->bikeModel);
                if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
                    sprintf(message, "No PC Rider MCF found in resources.  Aborting.\n");
                    delete stream;
                    Release();
                    return 0;
                }
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = racer->engineSize;
                setup.fourStroke = racer->fourStroke;
                bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                UnknownRandomSkill(skill, aiSettings[0], aiSettings[1]);
                aiRacers[n - 1] = UnknownBikeRaceNewRacer(__FILE__, 0x47c);
                aiRacers[n - 1]->Load(owner, empty, bikeFile, riderFile, lights, textures,
                                                          position, direction, up, terrain, 0, 1, 0, this,
                                                          racer->racerName, 0, id, &setup, 1, 1, particleManager,
                                                          raceScene, replayVcr, skill, 1);
                AppendChild(aiRacers[n - 1], -1);
                aiRacers[n - 1]->modelNode->ReplaceMaterialTexture(0, racer->bikeName, (int)textures);
                aiRacers[n - 1]->riderCharacter->modelNode->ReplaceMaterialTexture(0, racer->riderModel,
                                                                                     (int)textures);
                id++;
            }
        }
        return 1;
    }

    int used[8];
    if (!g_TrackGame->playingReplay && g_TrackGame->playerCount > 1) {
        racerSlots = new(__FILE__, 0x4a3) UnknownBikeRaceRacer*[racerCount];
        memset(used, 0, sizeof(used));
    }
    g_TrackGame->ui->StepProgressBar();
    char ai = 0;
    int* bikes = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4af);
    int* riders = (int*)DebugCalloc(racerCount, 4, __FILE__, 0x4b0);
    const char** names = (const char**)DebugMalloc(racerCount * 4, __FILE__, 0x4b1);
    int plate;
    int engineSize;
    int fourStroke;
    if (g_TrackGame->playingReplay) {
        // A replay: the recorded player.
        char recordedBike[0x40];
        char recordedRider[0x40];
        recordIndex = 1;
        replayVcr->GetRacer(&id, &ai, 0, name, bikeFile, riderFile, recordedBike, recordedRider,
                                           &engineSize, &fourStroke, &plate);
        g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
    } else {
        sprintf(path, "%s\\%s", "Res", g_TrackGame->mode.racerChoice.bikeModel);
        if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, path, "rb", (int)bikeFile)) {
            sprintf(message, "No Rider MCF found in resources.  Aborting.\n");
            delete stream;
            Release();
            return 0;
        }
        g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                         bikeTexture, 0x3f);
        g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                         riderTexture, 0x3f);
        if (!g_TrackGame->mode.raceSettings.multiplayerAiRacerCount) {
            g_TrackGame->ui->PickRandomRacers(bikes, racerCount, riders);
            g_TrackGame->ui->MakeRandomNames(names, racerCount);
        }
        strcpy(name, g_TrackGame->mode.profileName);
        plate = g_TrackGame->mode.plateNumber;
    }

    if (g_TrackGame->playingReplay || g_TrackGame->playerCount == 1) {
        // The player's racer offline (or in a replay).
        WriteRelativePositions(&position, &direction, event, order[0]);
        localRacer = UnknownBikeRaceNewRacer(__FILE__, 0x4ea);
        if (!AppendChild(
                localRacer->Load(
                    owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                    g_TrackGame->controlInterface, 0, 0, this, name, 0, id,
                    (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                    g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro, particleManager,
                    raceScene, replayVcr, 0, 1),
                -1)) {
            sprintf(message, "Player Bike not loaded\n");
            if (riders)
                DebugFree(riders, __FILE__, 0x4ff);
            if (bikes)
                DebugFree(bikes, __FILE__, 0x500);
            if (names)
                DebugFree(names, __FILE__, 0x501);
            delete stream;
            Release();
            return 0;
        }
        localRacer->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
        localRacer->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
        if (!g_TrackGame->mode.raceSettings.treeCollision) {
            localRacer->collisionObject->ignoreVegetation = 1;
            localRacer->frontWheel->ignoreVegetation = 1;
            localRacer->rearWheel->ignoreVegetation = 1;
        }
        localRacer->engineSize = ((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions)->engineSize;
        localRacer->fourStroke = (char)((UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions)->fourStroke;
        localRacer->plateNumber = plate;
        if ((replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) ||
            g_TrackGame->mode.raceSettings.raceMode == 4) {
            replayVcr->SetRacer(0, 0, recordIndex++, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, localRacer->engineSize,
                                               (unsigned char)localRacer->fourStroke, plate);
        }
    } else {
        // A network race: this player's racer, the other players' and the
        // AI racers of other hosts.
        playerId = g_TrackGame->network->localPlayer;
        int k;
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId && !GridPlayer(i).isAi) {
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x53b);
                racerSlots[j] = racer;
                if (!AppendChild(
                        racer->Load(
                            owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain,
                            g_TrackGame->controlInterface, 0, 0, this, g_TrackGame->mode.profileName, 0,
                            playerId, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                            g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                            particleManager, raceScene, replayVcr, 0, 1),
                        -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                    if (riders)
                        DebugFree(riders, __FILE__, 0x550);
                    if (bikes)
                        DebugFree(bikes, __FILE__, 0x551);
                    if (names)
                        DebugFree(names, __FILE__, 0x552);
                    delete stream;
                    Release();
                    return 0;
                }
                g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racerChoice.bikeName,
                                                                 ownBikeTexture, 0x3f);
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racerChoice.riderName,
                                                                 ownRiderTexture, 0x3f);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, ownBikeTexture, (int)textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, ownRiderTexture, (int)textures);
                racerSlots[j]->plateNumber = g_TrackGame->mode.plateNumber;
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(playerId, 0, recordIndex++, g_TrackGame->mode.profileName,
                                                       bikeFile, riderFile, ownBikeTexture, ownRiderTexture,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
                }
                localRacer = racerSlots[j];
                targetRacer = j;
                break;
            }
        }
        for (i = 0; i < racerCount; i++) {
            if (GridPlayer(i).networkPlayerId == playerId || GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                if (GridPlayer(i).networkPlayerId == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex == 0) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            sprintf(path, "%s\\%s", "Res", g_TrackGame->mode.racers[j].bikeModelName);
            UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x5a3);
            racerSlots[j] = racer;
            if (!AppendChild(racer->Load(
                                           owner, empty, path, riderFile, lights, textures, position, direction,
                                           up, terrain, 0, 0, 1, this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId,
                                           &setup, 0, 1, particleManager, raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            g_TrackGame->ui->ResolveBikeFile(g_TrackGame->mode.racers[j].bikeName, bikeTexture, 0x3f);
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
            if (g_TrackGame->GetRegistryInt("MPR", 0) == 123 && g_TrackGame->mode.racers[j].hasMprSkin) {
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, "MPR.tga", (int)textures);
            } else {
                g_TrackGame->ui->ResolveRiderFile(g_TrackGame->mode.racers[j].riderName, riderTexture, 0x3f);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
            }
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, 0, recordIndex++, g_TrackGame->mode.racers[j].playerName, path,
                                                   riderFile, bikeTexture, riderTexture, racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
        for (i = 0; i < racerCount; i++) {
            int other = GridPlayer(i).networkPlayerId;
            if (other == playerId || !GridPlayer(i).isAi)
                continue;
            int j;
            for (j = 0; j < racerCount; j++) {
                if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                    used[j] = 1;
                    break;
                }
            }
            WriteRelativePositions(&position, &direction, event, i + 1);
            setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
            setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
            setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
            int bikeClass = BikeClassOf(setup.engineSize);
            setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
            setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
            for (k = 0; k < 11; k++)
                setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
            setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
            setup.weight = g_TrackGame->ui->weight[bikeClass];
            UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x61f);
            racerSlots[j] = racer;
            if (!AppendChild(racer->Load(
                                           owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures, position,
                                           direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 1, this,
                                           g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, &setup, 0, 1, particleManager,
                                           raceScene, replayVcr, 0, 1),
                                       -1)) {
                sprintf(message, "Net Bike%d not loaded\n", j);
            }
            racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, (int)textures);
            racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, (int)textures);
            if (!g_TrackGame->mode.raceSettings.treeCollision) {
                racerSlots[j]->collisionObject->ignoreVegetation = 1;
                racerSlots[j]->frontWheel->ignoreVegetation = 1;
                racerSlots[j]->rearWheel->ignoreVegetation = 1;
            }
            racerSlots[j]->Disable();
            racerSlots[j]->messageDelay = g_TrackGame->mode.racers[j].lowestMessageDelay;
            if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                   g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                   g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                   racerSlots[j]->engineSize,
                                                   (unsigned char)racerSlots[j]->fourStroke, g_TrackGame->mode.racers[j].plateNumber);
            }
        }
    }

    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);
    ((FollowCamera*)raceCamera)->AppendTableEntry(position, 140.0f, 5.0f, 5.0f, 2.5f, 2.3f);

    int fromEvent = 0;
    if ((g_TrackGame->playingReplay && replayVcr->racerCount > 1) || g_TrackGame->playerCount == 1) {
        if (g_TrackGame->mode.raceSettings.raceMode == 4) {
            // The ghost.
            aiRacers = new(__FILE__, 0x674) UnknownBikeRaceRacer*[1];
            WriteRelativePositions(&position, &direction, event, order[0]);
            sprintf(bikeFile, "%s\\%s", "Res", "Ghost.mcf");
            sprintf(riderFile, "%s\\GhostRider.mcf", "Res");
            UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x67c);
            aiRacers[0] = racer;
            if (!AppendChild(
                    racer->Load(
                        owner, empty, bikeFile, riderFile, lights, textures, position, direction, up, terrain, 0, -1,
                        0, this, "Ghost", 0, 0, (UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions,
                        g_TrackGame->mode.gasGyro, g_TrackGame->mode.brakeGyro,
                        particleManager, raceScene, replayVcr, 0, 1),
                    -1)) {
                sprintf(message, "Ghost Bike not loaded\n");
            }
            replayVcr->SetRacer(0, 0, recordIndex, g_TrackGame->mode.profileName, bikeFile,
                                               riderFile, bikeTexture, riderTexture, aiRacers[0]->engineSize,
                                               (unsigned char)aiRacers[0]->fourStroke,
                                               g_TrackGame->mode.plateNumber);
            aiRacers[0]->UnknownRacerVirtualSlot50(1, 0, 1);
            aiRacers[0]->Disable();
        } else if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            // The AI racers.
            id = 1;
            aiRacers = new(__FILE__, 0x6a5) UnknownBikeRaceRacer*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (int i = 0; i < g_TrackGame->mode.raceSettings.aiRacerCount; i++)
                aiRacers[i] = 0;
            for (int n = 0; n < g_TrackGame->mode.raceSettings.aiRacerCount; n++) {
                char aiName[0x10];
                if (g_TrackGame->playingReplay) {
                    char recordedBike[0x40];
                    char recordedRider[0x40];
                    replayVcr->GetRacer(&id, &ai, recordIndex, aiName, bikeFile, riderFile,
                                                       recordedBike, recordedRider, &engineSize, &fourStroke,
                                                       &plate);
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                    g_TrackGame->ui->ResolveBikeFile(recordedBike, bikeTexture, 0x3f);
                    g_TrackGame->ui->ResolveRiderFile(recordedRider, riderTexture, 0x3f);
                } else {
                    if (g_TrackGame->mode.raceSettings.races > 0 &&
                        g_TrackGame->eventManager->racesRun > 0) {
                        fromEvent = 1;
                        COPY_TEXT(aiName, g_TrackGame->eventManager->results[n + 1].name, 0x10);
                        COPY_TEXT(bikeFile, EventAi(n).bikeModelPath, 0x104);
                    } else {
                        COPY_TEXT(aiName, names[n], 0x10);
                        sprintf(bikeFile, "%s\\%s", "Res", UiManufacturer(UiBike(bikes[n]).manufacturer).modelFile);
                    }
                    ai = 1;
                    if (!g_TrackGame->sceneObject->OpenArchiveFile(stream, bikeFile, "rb",
                                                                                   (int)bikeFile)) {
                        sprintf(message, "No Net Bike MCF found in resources.  Aborting.\n");
                        delete stream;
                        Release();
                        return 0;
                    }
                    setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                    if (fromEvent) {
                        engineSize = EventAi(n).engineSize;
                        fourStroke = EventAi(n).fourStroke;
                        skill = EventAi(n).skill;
                    } else {
                        engineSize = UiBike(bikes[n]).engineSize;
                        fourStroke = UiBike(bikes[n]).fourStroke;
                        if (g_TrackGame->mode.raceSettings.eventType == 0)
                            UnknownRandomSkill(skill, aiSettings[5], aiSettings[6]);
                        else
                            UnknownRandomSkill(skill, aiSettings[0], aiSettings[1]);
                    }
                    float r = rand() * (1.0f / 32768.0f);
                    plate = 101 - (int)(r * -898.0f);
                }
                WriteRelativePositions(&position, &direction, event, order[n + 1]);
                setup.engineSize = engineSize;
                setup.fourStroke = fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x718);
                aiRacers[n] = racer;
                if (!AppendChild(racer->Load(
                                               owner, empty, bikeFile, riderFile, lights, textures, position,
                                               direction, up, terrain, 0, ai, 0, this, aiName, 0, id, &setup, 1, 1,
                                               particleManager, raceScene, replayVcr, skill, 1),
                                           -1)) {
                    sprintf(message, "AI Bike%d not loaded\n", n);
                }
                if (g_TrackGame->playingReplay) {
                    char text[0x40];
                    COPY_TEXT(text, bikeTexture, 0x40);
                    g_TrackGame->ui->ResolveBikeFile(text, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    COPY_TEXT(text, riderTexture, 0x40);
                    g_TrackGame->ui->ResolveRiderFile(text, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                    recordIndex++;
                } else if (fromEvent) {
                    g_TrackGame->ui->ResolveBikeFile(EventAi(n).bikeTexture, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->ResolveRiderFile(EventAi(n).riderTexture, riderTexture, 0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                } else {
                    g_TrackGame->ui->ResolveBikeFile(UiBike(bikes[n]).bikeName, bikeTexture, 0x3f);
                    aiRacers[n]->modelNode->ReplaceMaterialTexture(0, bikeTexture, (int)textures);
                    g_TrackGame->ui->ResolveRiderFile(UiRider(riders[n]).modelFile, riderTexture,
                                                                     0x3f);
                    aiRacers[n]->riderCharacter->modelNode->ReplaceMaterialTexture(0, riderTexture, (int)textures);
                    aiRacers[n]->plateNumber = plate;
                }
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    aiRacers[n]->collisionObject->ignoreVegetation = 1;
                    aiRacers[n]->frontWheel->ignoreVegetation = 1;
                    aiRacers[n]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(id, 1, recordIndex++, aiName, path, riderFile, bikeTexture,
                                                       riderTexture, aiRacers[n]->engineSize,
                                                       (unsigned char)aiRacers[n]->fourStroke, plate);
                }
                id++;
                if (g_TrackGame->mode.raceSettings.raceMode == 2 &&
                    g_TrackGame->mode.raceSettings.races > 0 &&
                    !g_TrackGame->eventManager->racesRun) {
                    strcpy(EventAi(n).bikeTexture, bikeTexture);
                    strcpy(EventAi(n).riderTexture, riderTexture);
                    strcpy(EventAi(n).bikeModelPath, bikeFile);
                    EventAi(n).engineSize = engineSize;
                    EventAi(n).fourStroke = fourStroke;
                    EventAi(n).skill = skill;
                }
            }
        }
    } else {
        // A network race: the AI racers this host runs.
        id = 1;
        if (g_TrackGame->mode.raceSettings.aiRacerCount > 0) {
            aiRacers = new(__FILE__, 0x770) UnknownBikeRaceRacer*[g_TrackGame->mode.raceSettings.aiRacerCount];
            for (i = 0; i < racerCount; i++) {
                int other = GridPlayer(i).networkPlayerId;
                if (other != playerId || !GridPlayer(i).isAi)
                    continue;
                int j;
                for (j = 0; j < g_TrackGame->mode.racerCount; j++) {
                    if (other == g_TrackGame->mode.racers[j].networkPlayerId && g_TrackGame->mode.racers[j].racerIndex != 0 && !used[j]) {
                        used[j] = 1;
                        break;
                    }
                }
                WriteRelativePositions(&position, &direction, event, i + 1);
                setup = *(UnknownBikeRaceBikeSetup*)g_TrackGame->mode.garageOptions;
                setup.engineSize = g_TrackGame->mode.racers[j].engineSize;
                setup.fourStroke = g_TrackGame->mode.racers[j].fourStroke;
                int bikeClass = BikeClassOf(setup.engineSize);
                setup.rpmLowerLimit = g_TrackGame->ui->rpmLowerLimit[bikeClass];
                setup.rpmUpperLimit = g_TrackGame->ui->rpmUpperLimit[bikeClass];
                for (int k = 0; k < 11; k++)
                    setup.presetCurve[k] = g_TrackGame->ui->standardCurves[bikeClass][0][k];
                setup.rpmStep = (setup.rpmUpperLimit - setup.rpmLowerLimit) / 10;
                setup.weight = g_TrackGame->ui->weight[bikeClass];
                UnknownBikeRaceRacer* racer = UnknownBikeRaceNewRacer(__FILE__, 0x78f);
                racerSlots[j] = racer;
                if (!AppendChild(racer->Load(
                                               owner, empty, g_TrackGame->mode.racers[j].bikeModelName, riderFile, lights, textures,
                                               position, direction, up, terrain, 0, (char)g_TrackGame->mode.racers[j].racerIndex, 0,
                                               this, g_TrackGame->mode.racers[j].playerName, 0, g_TrackGame->mode.racers[j].networkPlayerId, &setup, 0, 1,
                                               particleManager, raceScene, replayVcr, 0, 1),
                                           -1)) {
                    sprintf(message, "Net Bike%d not loaded\n", j);
                }
                racerSlots[j]->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].bikeName, (int)textures);
                racerSlots[j]->riderCharacter->modelNode->ReplaceMaterialTexture(0, g_TrackGame->mode.racers[j].riderName, (int)textures);
                if (!g_TrackGame->mode.raceSettings.treeCollision) {
                    racerSlots[j]->collisionObject->ignoreVegetation = 1;
                    racerSlots[j]->frontWheel->ignoreVegetation = 1;
                    racerSlots[j]->rearWheel->ignoreVegetation = 1;
                }
                if (replayVcr && g_TrackGame->useReplayTape && !g_TrackGame->playingReplay) {
                    replayVcr->SetRacer(g_TrackGame->mode.racers[j].networkPlayerId, (char)g_TrackGame->mode.racers[j].racerIndex, recordIndex++,
                                                       g_TrackGame->mode.racers[j].playerName, g_TrackGame->mode.racers[j].bikeModelName, riderFile,
                                                       g_TrackGame->mode.racers[j].bikeName, g_TrackGame->mode.racers[j].riderName,
                                                       racerSlots[j]->engineSize,
                                                       (unsigned char)racerSlots[j]->fourStroke,
                                                       g_TrackGame->mode.racers[j].plateNumber);
                }
                aiRacers[id - 1] = racerSlots[j];
                id++;
            }
        }
        // Every racer's collisions against every other racer's.
        for (int a = 0; a < racerCount; a++) {
            if (!racerSlots[a]->isRemote)
                continue;
            for (int b = 0; b < racerCount; b++) {
                if (a == b)
                    continue;
                if (racerSlots[b]->isRemote) {
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->frontWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->rearWheel);
                    racerSlots[a]->bikeAndRiderCollision->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                }
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->frontWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->rearWheel->AddIgnoredOwner(racerSlots[b]->wrecker->collisionModel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->bikeAndRiderCollision);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->frontWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(racerSlots[b]->rearWheel);
                racerSlots[a]->wrecker->collisionModel->AddIgnoredOwner(
                    racerSlots[b]->wrecker->collisionModel);
            }
        }
    }
    if (riders)
        DebugFree(riders, __FILE__, 0x7f7);
    if (bikes)
        DebugFree(bikes, __FILE__, 0x7f8);
    if (names)
        DebugFree(names, __FILE__, 0x7f9);
    delete stream;
    return 1;
}
