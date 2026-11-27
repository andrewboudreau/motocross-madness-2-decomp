#include <process.h>
#include <string.h>
#include <windows.h>

#include "Recorder.h"

#include "DebugAlloc.h"
#include "MatrixUtil.h"
#include "VCR.h"
#include "VCRfile.h"

// Closes a handle member once.
#define CLOSE_HANDLE(handle)          \
    if (handle) {                     \
        CloseHandle(handle);          \
        handle = 0;                   \
    }

// 0x004e6f80: the worker thread. It records queued ring records to the VCR
// file, plays them back into the ring and seeks, driven by the eleven events;
// `message` keeps the last status text (the retail literals) but is not
// otherwise used.
// Not exact (2100 of 2148 bytes: case 2's tail cross-jumps into case 4 and
// `name`/`headerSize` trade a register and a slot; docs/RECORDER_WORKER.md).
unsigned __stdcall RecorderWorkerThread(void* parameters) {
    RecorderThreadParameters* params = (RecorderThreadParameters*)parameters;
    VCRInterface* vcr;
    VCRFile* file;
    void* header;
    int headerSize;
    char* name;
    HANDLE events[11];
    char message[512];
    DWORD result;
    int handle = -1;

    strcpy(message, "VCR WaitForMultipleObjects Failure!\n");
    header = (void*)params->header;
    headerSize = params->headerSize;
    name = (char*)params->fileName;
    vcr = params->owner;
    file = vcr->vcrFile;
    events[0] = vcr->writeEvent;
    events[1] = vcr->createFileEvent;
    events[2] = vcr->reopenFileEvent;
    events[3] = vcr->readForwardEvent;
    events[4] = vcr->openPlaybackEvent;
    events[5] = vcr->readReverseEvent;
    events[6] = vcr->seekEvent;
    events[7] = vcr->rewindEvent;
    events[8] = vcr->stopReadingEvent;
    events[9] = vcr->closeFileEvent;
    events[10] = vcr->stopEvent;
    while ((result = WaitForMultipleObjects(11, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0: // write the queued records
            if (vcr->frameRing->HasSlotToWrite()) {
                void* data;
                int size;
                int index;

                data = 0;
                EnterCriticalSection(&vcr->lock);
                vcr->frameRing->TakeSlotToWrite(&data, &size, &index);
                LeaveCriticalSection(&vcr->lock);
                if (!file->Write(data, size, 1, handle)) {
                    strcpy(message, "VCRtape file write error\n");
                    goto done;
                }
                EnterCriticalSection(&vcr->lock);
                vcr->frameRing->ReleaseSlot(index);
                LeaveCriticalSection(&vcr->lock);
            }
            if (vcr->frameRing->HasSlotToWrite())
                SetEvent(vcr->writeEvent);
            break;
        case 1: // create the file and write the header
            EnterCriticalSection(&vcr->lock);
            vcr->fileReady = 1;
            LeaveCriticalSection(&vcr->lock);
            handle = file->Open(name, "wb");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->Write(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header write error\n");
                goto done;
            }
            goto opened;
        case 2: // reopen the file for appending
            EnterCriticalSection(&vcr->lock);
            vcr->fileReady = 1;
            LeaveCriticalSection(&vcr->lock);
            handle = file->Open(name, "rb+");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->Read(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header (reopen) read error\n");
                goto done;
            }
            file->Seek(handle, headerSize, SEEK_SET);
        opened:
            strcpy(message, name);
            EnterCriticalSection(&vcr->lock);
            vcr->isStopped = 0;
            vcr->workerAcknowledged = 1;
            LeaveCriticalSection(&vcr->lock);
            break;
        case 3: // read the next record into the ring
            EnterCriticalSection(&vcr->lock);
            if (vcr->isBusy) {
                LeaveCriticalSection(&vcr->lock);
                break;
            }
            LeaveCriticalSection(&vcr->lock);
            if (vcr->frameRing->HasRoomToRead()) {
                void* data;
                int position;
                unsigned int size;
                int index;

                data = 0;
                position = file->Tell(handle);
                if (!file->Read(&size, 4, 1, handle)) {
                    if (!file->IsEndOfFile(handle)) {
                        strcpy(message, "VCRtape file read error (size)\n");
                        goto done;
                    }
                    EnterCriticalSection(&vcr->lock);
                    vcr->frameRing->MarkForwardEnd();
                    LeaveCriticalSection(&vcr->lock);
                    break;
                }
                if (size > 0x400) {
                    strcpy(message, "data size exceeds buffer.\n");
                    goto done;
                }
                EnterCriticalSection(&vcr->lock);
                vcr->frameRing->ClaimReadSlot(size, &index, &data);
                LeaveCriticalSection(&vcr->lock);
                if (!file->Read(data, size, 1, handle)) {
                    if (!file->IsEndOfFile(handle)) {
                        strcpy(message, "VCRtape file read error (data)\n");
                        goto done;
                    }
                } else {
                    EnterCriticalSection(&vcr->lock);
                    vcr->frameRing->CommitReadSlot(position, index);
                    LeaveCriticalSection(&vcr->lock);
                }
                if (vcr->frameRing->HasRoomToRead())
                    SetEvent(vcr->readForwardEvent);
            }
            break;
        case 4: // open the file for playback
            EnterCriticalSection(&vcr->lock);
            vcr->fileReady = 1;
            LeaveCriticalSection(&vcr->lock);
            handle = file->Open(name, "rb");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->Read(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header read error\n");
                goto done;
            }
            vcr->frameRing->SetFirstPosition(file->Tell(handle));
            EnterCriticalSection(&vcr->lock);
            vcr->isStopped = 0;
            LeaveCriticalSection(&vcr->lock);
            SetEvent(vcr->readForwardEvent);
            strcpy(message, name);
            EnterCriticalSection(&vcr->lock);
            vcr->workerAcknowledged = 1;
            LeaveCriticalSection(&vcr->lock);
            break;
        case 5: // read the record at the next recorded position
            EnterCriticalSection(&vcr->lock);
            if (vcr->isBusy) {
                LeaveCriticalSection(&vcr->lock);
                break;
            }
            LeaveCriticalSection(&vcr->lock);
            if (vcr->frameRing->HasRoomToRead()) {
                void* data;
                int position;
                int extra;
                unsigned int size;
                int index;

                data = 0;
                position = vcr->frameRing->StepPositionBack(&extra);
                vcr->recordTimeMs = extra;
                if (position == 0) {
                    EnterCriticalSection(&vcr->lock);
                    vcr->frameRing->MarkReverseEnd();
                    LeaveCriticalSection(&vcr->lock);
                    break;
                }
                if (position == -1) {
                    EnterCriticalSection(&vcr->lock);
                    vcr->frameRing->ClaimReadSlot(1, &index, &data);
                    *(unsigned char*)data = 0xff;
                    vcr->frameRing->CommitReadSlot(-1, index);
                    LeaveCriticalSection(&vcr->lock);
                } else {
                    file->Seek(handle, position, SEEK_SET);
                    if (!file->Read(&size, 4, 1, handle)) {
                        if (!file->IsEndOfFile(handle)) {
                            strcpy(message, "VCRtape file read error (size)\n");
                            goto done;
                        }
                        EnterCriticalSection(&vcr->lock);
                        vcr->frameRing->MarkReverseEnd();
                        LeaveCriticalSection(&vcr->lock);
                        break;
                    }
                    if (size > 0x400) {
                        strcpy(message, "data size exceeds buffer.\n");
                        goto done;
                    }
                    EnterCriticalSection(&vcr->lock);
                    vcr->frameRing->ClaimReadSlot(size, &index, &data);
                    LeaveCriticalSection(&vcr->lock);
                    if (!file->Read(data, size, 1, handle)) {
                        if (!file->IsEndOfFile(handle)) {
                            strcpy(message, "VCRtape file read error (data)\n");
                            goto done;
                        }
                    } else {
                        EnterCriticalSection(&vcr->lock);
                        vcr->frameRing->CommitReadSlot(position, index);
                        LeaveCriticalSection(&vcr->lock);
                    }
                }
                if (vcr->frameRing->HasRoomToRead())
                    SetEvent(vcr->readReverseEvent);
            }
            break;
        case 6: { // seek back to the previous recorded position
            int extra;
            int position;

            ResetEvent(vcr->readForwardEvent);
            ResetEvent(vcr->readReverseEvent);
            position = vcr->frameRing->PeekPosition(&extra);
            if (position == -1)
                position = vcr->frameRing->PeekPosition(&extra);
            file->Seek(handle, position, SEEK_SET);
            EnterCriticalSection(&vcr->lock);
            vcr->frameRing->ResetSlots();
            LeaveCriticalSection(&vcr->lock);
            SetEvent(vcr->readForwardEvent);
            break;
        }
        case 7: { // rewind to the first recorded position
            int position;

            ResetEvent(vcr->readForwardEvent);
            ResetEvent(vcr->readReverseEvent);
            position = vcr->frameRing->GetFirstPosition();
            EnterCriticalSection(&vcr->lock);
            vcr->frameRing->ResetSlots();
            LeaveCriticalSection(&vcr->lock);
            file->Seek(handle, position, SEEK_SET);
            vcr->fileReady = 1;
            SetEvent(vcr->readForwardEvent);
            break;
        }
        case 8:
            ResetEvent(vcr->readForwardEvent);
            ResetEvent(vcr->readReverseEvent);
            EnterCriticalSection(&vcr->lock);
            vcr->readsReset = 1;
            vcr->fileReady = 1;
            LeaveCriticalSection(&vcr->lock);
            break;
        case 9: // rewrite the header and close the file
            file->Seek(handle, 0, SEEK_SET);
            if (!file->Write(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header write error\n");
                goto done;
            }
            file->Close(handle);
            EnterCriticalSection(&vcr->lock);
            vcr->workerAcknowledged = 1;
            LeaveCriticalSection(&vcr->lock);
            handle = -1;
            break;
        case 10:
            strcpy(message, "Received normal Kill message.\n");
            goto done;
        }
    }
done:
    EnterCriticalSection(&vcr->lock);
    vcr->isStopped = 0;
    LeaveCriticalSection(&vcr->lock);
    if (handle >= 0)
        file->Close(handle);
    _endthreadex(0);
    return 0;
}

// 0x004e77f0
VCRInterface::VCRInterface() : GameObject(1) {
    workerThread = 0;
    writeEvent = 0;
    createFileEvent = 0;
    reopenFileEvent = 0;
    openPlaybackEvent = 0;
    readReverseEvent = 0;
    seekEvent = 0;
    rewindEvent = 0;
    stopReadingEvent = 0;
    closeFileEvent = 0;
    stopEvent = 0;
    workerThreadId = 0;
    frameRing = 0;
    recordBuffer = 0;
    readingReverse = 0;
    recordPending = 0;
    recordType = 0;
    recordId = 0;
    recordPosition = 0;
    isBusy = 0;
    fileReady = 0;
    readsReset = 0;
    reverseReadRestarted = 0;
    sceneTimePending = 0;
    pendingSceneTime = 0;
    isStopped = 1;
    workerAcknowledged = 1;
}

// 0x004e78b0
VCRInterface::~VCRInterface() {
    if (workerThread) {
        SetEvent(stopEvent);
        WaitForSingleObject(workerThread, INFINITE);
        CLOSE_HANDLE(workerThread);
    }
    CLOSE_HANDLE(writeEvent);
    CLOSE_HANDLE(createFileEvent);
    CLOSE_HANDLE(reopenFileEvent);
    CLOSE_HANDLE(readForwardEvent);
    CLOSE_HANDLE(openPlaybackEvent);
    CLOSE_HANDLE(readReverseEvent);
    CLOSE_HANDLE(seekEvent);
    CLOSE_HANDLE(rewindEvent);
    CLOSE_HANDLE(stopReadingEvent);
    CLOSE_HANDLE(closeFileEvent);
    CLOSE_HANDLE(stopEvent);
    DeleteCriticalSection(&lock);
    if (frameRing)
        delete frameRing;
    if (recordBuffer)
        DebugFree(recordBuffer, __FILE__, 442);
}

// 0x004e7a00
void VCRInterface::SignalAndWait(HANDLE event, DWORD interval) {
    int waiting;

    EnterCriticalSection(&lock);
    workerAcknowledged = 0;
    LeaveCriticalSection(&lock);
    SetEvent(event);
    Sleep(interval);
    EnterCriticalSection(&lock);
    waiting = !workerAcknowledged;
    LeaveCriticalSection(&lock);
    while (waiting) {
        Sleep(interval);
        EnterCriticalSection(&lock);
        waiting = !workerAcknowledged;
        LeaveCriticalSection(&lock);
    }
}

// 0x004e7a90
int VCRInterface::Start(int callback, int header, int headerSize, int mode, int fileName,
                        unsigned int size, VCRFile* owner) {
    DWORD interval;

    frameRing = new (__FILE__, 477) VCR(size);
    if (!frameRing)
        return 0;
    vcrFile = owner;
    recordCallback = (RecorderCallbackProc)callback;
    writeEvent = CreateEventA(0, 0, 0, 0);
    createFileEvent = CreateEventA(0, 0, 0, 0);
    reopenFileEvent = CreateEventA(0, 0, 0, 0);
    openPlaybackEvent = CreateEventA(0, 0, 0, 0);
    readForwardEvent = CreateEventA(0, 0, 0, 0);
    readReverseEvent = CreateEventA(0, 0, 0, 0);
    seekEvent = CreateEventA(0, 0, 0, 0);
    rewindEvent = CreateEventA(0, 0, 0, 0);
    stopReadingEvent = CreateEventA(0, 0, 0, 0);
    closeFileEvent = CreateEventA(0, 0, 0, 0);
    stopEvent = CreateEventA(0, 0, 0, 0);
    if (!writeEvent || !stopEvent || !createFileEvent || !reopenFileEvent || !openPlaybackEvent || !readForwardEvent ||
        !readReverseEvent || !seekEvent || !rewindEvent || !stopReadingEvent || !closeFileEvent)
        return 0;
    recordMode = mode;
    threadParameters.header = header;
    threadParameters.headerSize = headerSize;
    threadParameters.fileName = fileName;
    threadParameters.owner = this;
    InitializeCriticalSection(&lock);
    workerThread = (HANDLE)_beginthreadex(0, 0, RecorderWorkerThread, &threadParameters, 0, &workerThreadId);
    if (!workerThread)
        return 0;
    interval = vcrFile->memory ? 10 : 200;
    if (mode == 0) {
        SignalAndWait(createFileEvent, interval);
        return 1;
    }
    if (mode == 2) {
        SignalAndWait(reopenFileEvent, interval);
        return 1;
    }
    if (mode == 1) {
        recordBuffer = DebugMalloc(0x400, __FILE__, 535);
        if (!recordBuffer)
            return 0;
        SignalAndWait(openPlaybackEvent, interval);
        frameRing->AppendPosition(-1, 0);
        frameRing->AppendPosition(-1, 0);
    }
    return 1;
}

// 0x004e7d60: per-frame playback step (mode 1); KrustyVCR shares it. Takes
// queued records from the ring and hands them to the callback at +0x80.
// Always returns 1.
// Not exact (276 of 2412 bytes in place, 2448 compiled: retail keeps 0 and 1
// in registers through the loops and assigns other stack slots;
// docs/RECORDER_TICK.md).
int VCRInterface::Tick(float frameTime) {
    int keep;
    int milliseconds;
    int value;
    int queued;
    unsigned int size;
    RecorderRecordHeader* record;
    int count;
    int result;

    EnterCriticalSection(&lock);
    if (!fileReady) {
        LeaveCriticalSection(&lock);
        return 1;
    }
    LeaveCriticalSection(&lock);
    if (recordMode != 1)
        return 1;
    if (readingReverse) {
        int flag;

        EnterCriticalSection(&lock);
        flag = readsReset;
        LeaveCriticalSection(&lock);
        if (flag) {
            EnterCriticalSection(&lock);
            readsReset = 0;
            reverseReadRestarted = 1;
            LeaveCriticalSection(&lock);
            RestartReverseRead();
            return 1;
        }
        EnterCriticalSection(&lock);
        flag = reverseReadRestarted;
        LeaveCriticalSection(&lock);
        if (!flag)
            return 1;
        for (;;) {
            result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 0);
            if (result == 0) {
                if (recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds) != 11) {
                    readingReverse = 0;
                    StopReading();
                    frameRing->SeekPosition(value);
                    Seek(milliseconds);
                    reverseReadRestarted = 0;
                    return 1;
                }
            } else if (result == 2) {
                recordCallback(-3, 0, 0, 0, &keep, recordTimeMs, &milliseconds);
                readingReverse = 0;
                EnterCriticalSection(&lock);
                reverseReadRestarted = 0;
                LeaveCriticalSection(&lock);
                recordPending = 0;
                Rewind();
                return 1;
            } else if (result == 3) {
                return 1;
            } else if (result == 4) {
                StopReading();
                return 1;
            }
        }
    }
    queued = 0;
    if (recordPending) {
        result = recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
        if (result == 3)
            return 1;
        if (result == 5) {
            recordPending = 1;
            return 1;
        }
        if (result == 12) {
            recordPending = 0;
            Rewind();
            return 1;
        }
        record = (RecorderRecordHeader*)recordBuffer;
        recordTimeMs = (int)(record->time * 1000.0f);
        count = record->count;
        recordPending = 0;
        while (count--) {
            result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
            if (result == 0) {
                recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                if (keep) {
                    queued = 1;
                    frameRing->AppendPosition(value, recordTimeMs);
                }
            } else {
                if (result == 1 &&
                    recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds) == 12) {
                    recordPending = 0;
                    Rewind();
                    return 1;
                }
                break;
            }
        }
        if (queued)
            frameRing->AppendPosition(-1, recordTimeMs);
        return 1;
    }
    recordPending = 0;
    result = TakeRecord(&recordType, &recordId, &recordPosition, recordBuffer, &size, 1);
    if (result == 1) {
        result = recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds);
        if (result == 11) {
            readingReverse = 1;
            StopReading();
            return 1;
        }
        if (result == 12)
            Rewind();
        return 1;
    }
    if (result == 4)
        return 1;
    if (recordType != -1)
        return 1;
    frameRing->AppendPosition(recordPosition, recordTimeMs);
    record = (RecorderRecordHeader*)recordBuffer;
    recordTimeMs = (int)(record->time * 1000.0f);
    count = record->count;
    switch (recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds)) {
    case 5:
        recordPending = 1;
        return 1;
    case 3:
        recordPending = 1;
        return 1;
    case 11:
        readingReverse = 1;
        StopReading();
        return 1;
    case 12:
        Rewind();
        return 1;
    case 2:
        for (;;) {
            while (count--) {
                result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
                if (result == 0) {
                    recordCallback(recordType, recordBuffer, 1, recordId, &keep, recordTimeMs, &milliseconds);
                    if (keep) {
                        queued = 1;
                        frameRing->AppendPosition(value, recordTimeMs);
                    }
                } else if (result == 1) {
                    if (recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds) == 12) {
                        recordPending = 1;
                        Rewind();
                    }
                    return 1;
                } else if (result == 4) {
                    return 1;
                }
            }
            if (queued)
                frameRing->AppendPosition(-1, recordTimeMs);
            queued = 0;
            result = TakeRecord(&recordType, &recordId, &recordPosition, recordBuffer, &size, 1);
            if (result == 1) {
                result = recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds);
                if (result == 11) {
                    readingReverse = 1;
                    StopReading();
                    return 1;
                }
                if (result == 12)
                    Rewind();
                return 1;
            }
            if (result == 4)
                return 1;
            result = recordCallback(recordType, recordBuffer, 1, recordId, &keep, recordTimeMs, &milliseconds);
            if (result == 1) {
                record = (RecorderRecordHeader*)recordBuffer;
                recordTimeMs = (int)(record->time * 1000.0f);
                count = record->count;
                while (count--) {
                    result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
                    if (result == 0) {
                        recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                        if (keep) {
                            queued = 1;
                            frameRing->AppendPosition(value, recordTimeMs);
                        }
                    } else if (result == 1) {
                        if (recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds) == 12) {
                            recordPending = 1;
                            Rewind();
                        }
                        return 1;
                    } else if (result == 4) {
                        return 1;
                    }
                }
                break;
            }
            if (result == 3) {
                recordPending = 1;
                return 1;
            }
            if (result == 11) {
                readingReverse = 1;
                StopReading();
                return 1;
            }
            if (result == 12) {
                Rewind();
                return 1;
            }
            record = (RecorderRecordHeader*)recordBuffer;
            recordTimeMs = (int)(record->time * 1000.0f);
            count = record->count;
        }
        break;
    case 1:
        while (count--) {
            result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
            if (result == 0) {
                recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                if (keep) {
                    queued = 1;
                    frameRing->AppendPosition(value, recordTimeMs);
                }
            } else if (result == 1) {
                if (recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds) == 12) {
                    recordPending = 1;
                    Rewind();
                }
                return 1;
            } else if (result == 4) {
                return 1;
            }
        }
        break;
    case 7:
        while (count--) {
            result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
            if (result == 0) {
                recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                if (keep) {
                    queued = 1;
                    frameRing->AppendPosition(value, recordTimeMs);
                }
            } else if (result == 1) {
                if (recordCallback(-2, 0, 0, 0, &keep, recordTimeMs, &milliseconds) == 12) {
                    recordPending = 1;
                    Rewind();
                }
                return 1;
            } else if (result == 4) {
                return 1;
            }
        }
        recordPending = 1;
        break;
    default:
        return 1;
    }
    if (queued)
        frameRing->AppendPosition(-1, recordTimeMs);
    return 1;
}

// 0x004e86d0
void VCRInterface::CloseFile(int header, int headerSize, int wait) {
    int fast = vcrFile->memory;

    threadParameters.header = header;
    threadParameters.headerSize = headerSize;
    if (wait)
        SignalAndWait(closeFileEvent, fast ? 10 : 200);
    else
        SetEvent(closeFileEvent);
}

// 0x004e8720
int VCRInterface::QueueRecord(int type, int id, const void* data, unsigned int size) {
    void* block;
    int* record;
    int index;

    block = 0;
    if (!frameRing)
        return 0;
    EnterCriticalSection(&lock);
    if (isStopped) {
        LeaveCriticalSection(&lock);
        return 0;
    }
    if (!frameRing->ClaimRecordSlot(size + 12, &index, &block)) {
        LeaveCriticalSection(&lock);
        return 0;
    }
    LeaveCriticalSection(&lock);
    record = (int*)block;
    *record++ = type;
    *record++ = id;
    *record++ = size;
    block = record;
    memcpy(block, data, size);
    EnterCriticalSection(&lock);
    frameRing->CommitRecordSlot(index);
    LeaveCriticalSection(&lock);
    SetEvent(writeEvent);
    return 1;
}

// 0x004e8810
int VCRInterface::TakeRecord(int* type, int* id, int* position, void* data, unsigned int* size, int signal) {
    void* block;
    int* record;
    int index;
    unsigned int bytes;
    int result;
    int extra;

    block = 0;
    bytes = 0;
    if (!frameRing)
        return 4;
    EnterCriticalSection(&lock);
    result = frameRing->TakePlaybackSlot(&block, &bytes, &extra, &index);
    if (result == 0) {
        LeaveCriticalSection(&lock);
        if (*(unsigned char*)block == 0xff && extra == -1) {
            frameRing->ReleaseSlot(index);
            SetEvent(readReverseEvent);
            return 3;
        }
        record = (int*)block;
        *type = *record++;
        *id = *record++;
        *size = *record++;
        block = record;
        memcpy(data, block, *size);
        *position = extra;
        EnterCriticalSection(&lock);
        frameRing->ReleaseSlot(index);
        LeaveCriticalSection(&lock);
        if (signal)
            SetEvent(readForwardEvent);
        else
            SetEvent(readReverseEvent);
        return 0;
    }
    if (result == 1) {
        LeaveCriticalSection(&lock);
        StopReading();
        return 1;
    }
    if (result == 2) {
        LeaveCriticalSection(&lock);
        StopReading();
        return 2;
    }
    LeaveCriticalSection(&lock);
    return 4;
}

// 0x004e8990
void VCRInterface::StopReading() {
    EnterCriticalSection(&lock);
    readsReset = 0;
    fileReady = 0;
    LeaveCriticalSection(&lock);
    SetEvent(stopReadingEvent);
}

// 0x004e89c0
void VCRInterface::RestartReverseRead() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        isBusy = 1;
        LeaveCriticalSection(&lock);
        EnterCriticalSection(&lock);
        frameRing->ResetSlots();
        isBusy = 0;
        LeaveCriticalSection(&lock);
        SetEvent(readReverseEvent);
    }
}

// 0x004e8a20
void VCRInterface::Seek(unsigned int milliseconds) {
    if (frameRing) {
        SetEvent(seekEvent);
        sceneTimePending = 1;
        pendingSceneTime = milliseconds * 0.001f;
    }
}

// 0x004e8a70
void VCRInterface::Rewind() {
    if (frameRing) {
        EnterCriticalSection(&lock);
        sceneTimePending = 1;
        pendingSceneTime = 0;
        fileReady = 0;
        frameRing->ClearPositions();
        SetEvent(rewindEvent);
        LeaveCriticalSection(&lock);
    }
}
