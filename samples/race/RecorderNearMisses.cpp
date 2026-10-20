// Near-miss recorder.cpp candidates, kept out of src/reconstructed until they
// match. See docs/RECORDER.md. The canonical file is included first so the
// TU-local declarations and globals are the same.
//
// 0x004e6f80 (2148 bytes with its jump table, the worker thread): the stack
// frame, the prologue, the eleven-way switch, every case body and the error
// exits match. Retail case 1 ends in a `jmp` to case 2's strcpy(message,
// name); `goto opened` there reproduces it (516 -> 652 of 2180; the real
// extent is 2148 bytes, the rest is padding and the next function). With
// its own copy of the tail VC6 hoists the first `lea edx, message` of the
// strcpy above the header-write test (both successors start with it) and
// merges the tail suffixes of cases 1 and 2 into case 4; a shared
// `headerWriteError` label for cases 1 and 9 removes the hoist but not the
// merge, and `strcpy; goto` (label after case 2's strcpy) keeps the hoist.
// Left: case 2's "field_0xc8 = 1; LeaveCriticalSection" suffix still
// cross-jumps into case 4 (retail keeps it, after scheduling `push esi`
// above the stores), and with one `name` use fewer `name` and `headerSize`
// trade stack slots (0x08/0x10; declaration order does not move them).
// break/continue mixes, braces, a comma store and store orders did not
// change the case 2 merge.
// Session 10: retail keeps `name` in edi from case 2's Open to the shared
// strcpy and reloads headerSize into eax for the Seek; VC6 here gives edi
// to headerSize and reloads `name` at the join. Unchanged (652): the
// opened critical section as an inline helper, a lock pointer local, the
// two stores as a comma or chained assignment, `register` on name, an
// unsigned headerSize, name declared first or at its assignment.
// Session 14: with case 1 given its own copy of the tail (516), case 2 is
// retail's instruction for instruction (name in edi from its Open to the
// strcpy, headerSize reloaded for the Seek), so the goto is what costs case
// 2 its registers. Case 1's copy then starts its block with `lea edx,
// message`, which VC6 hoists above the header-write `je` (retail's case 1
// strcpy is scheduled like case 2's, `lea edx` after `sub edi, ecx`, and
// cross-jumps into it at `or ecx, -1`). The success path inline before the
// error path and a block-local copy of name give the same 516.
//
// 0x004e7d60 (2412 bytes with its jump table, VCRInterface slot 10; KrustyVCR
// shares it): control flow, call arguments and the callback switch order
// (5, 3, 11, 12, 2, 1, 7) are reconstructed, but many blocks differ: retail
// keeps the constant 0 in eax from the readingReverse test on (cmp ecx, eax;
// push eax for the flag; recordPending = eax) and 1 in ebp in the loops, assigns
// the stack slots value 0x18, queued 0x1c, size 0x20 and the readsReset
// flag/loop count 0x24, keeps loop counts in ebp/edx, and cross-jumps fewer
// `Rewind(); return 1;` exits.

#include "../../src/reconstructed/Recorder.cpp"
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

// 0x004e7d60: per-frame playback step (mode 1); KrustyVCR shares it. Takes
// queued records from the ring and hands them to the callback at +0x80.
// Always returns 1.
int VCRInterface::Tick(float frameTime) {
    int keep;
    int milliseconds;
    int value;
    int queued;
    unsigned int size;
    UnknownRecorderRecord* record;
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
        record = (UnknownRecorderRecord*)recordBuffer;
        recordTimeMs = (int)(record->time * 1000.0f);
        count = record->count;
        recordPending = 0;
        while (count--) {
            result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
            if (result == 0) {
                recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                if (keep) {
                    frameRing->AppendPosition(value, recordTimeMs);
                    queued = 1;
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
    record = (UnknownRecorderRecord*)recordBuffer;
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
                        frameRing->AppendPosition(value, recordTimeMs);
                        queued = 1;
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
                record = (UnknownRecorderRecord*)recordBuffer;
                recordTimeMs = (int)(record->time * 1000.0f);
                count = record->count;
                while (count--) {
                    result = TakeRecord(&recordType, &recordId, &value, recordBuffer, &size, 1);
                    if (result == 0) {
                        recordCallback(recordType, recordBuffer, 0, recordId, &keep, recordTimeMs, &milliseconds);
                        if (keep) {
                            frameRing->AppendPosition(value, recordTimeMs);
                            queued = 1;
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
            record = (UnknownRecorderRecord*)recordBuffer;
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
                    frameRing->AppendPosition(value, recordTimeMs);
                    queued = 1;
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
                    frameRing->AppendPosition(value, recordTimeMs);
                    queued = 1;
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
