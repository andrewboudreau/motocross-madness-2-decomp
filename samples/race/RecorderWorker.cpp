// Complete replay worker candidate; see docs/RECORDER_WORKER.md.
// This grouping does not establish an original translation-unit boundary.
#include <process.h>
#include <string.h>
#include <windows.h>
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/reconstructed/VCR.h"
#include "../../src/reconstructed/VCRfile.h"

// 0x004e6f80: the worker thread. It records queued ring records to the VCR
// file, plays them back into the ring and seeks, driven by the eleven events;
// `message` keeps the last status text (the retail literals) but is not
// otherwise used.
unsigned __stdcall UnknownRecorderThread(void* parameters) {
    UnknownRecorderThreadParameters* params = (UnknownRecorderThreadParameters*)parameters;
    VCRInterface* vcr;
    UnknownVcrFile* file;
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
    file = (UnknownVcrFile*)vcr->vcrFile;
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
