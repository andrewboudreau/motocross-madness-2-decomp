// Near-miss recorder.cpp candidates, kept out of src/reconstructed until they
// match. See docs/RECORDER.md. The canonical file is included first so the
// TU-local declarations and globals are the same.
//
// 0x004e6f80 (2148 bytes with its jump table, the worker thread): the stack
// frame, the prologue, the eleven-way switch, every case body and the error
// exits match. Remaining difference: VC6 cross-jumping of the case tails.
// Retail merges case 1's whole success tail into case 2's (case 1 ends in
// `jmp` to case 2's strcpy(message, name)) and leaves case 4's tail intact;
// here VC6 hoists the first `lea edx, message` of case 1's strcpy above the
// header-write test (both successors start with it) and then merges the
// "push esi; field_0xc8 = 1; LeaveCriticalSection" suffixes of cases 1 and 2
// into case 4, so the candidate is 15 instructions longer. break/continue
// mixes, braces, a shared goto label and store orders did not reproduce it.
//
// 0x004e7d60 (2412 bytes with its jump table, VCRInterface slot 10; KrustyVCR
// shares it): control flow, call arguments and the callback switch order
// (5, 3, 11, 12, 2, 1, 7) are reconstructed, but many blocks differ: retail
// keeps the constant 0 in eax from the field_0x88 test on (cmp ecx, eax;
// push eax for the flag; field_0x84 = eax) and 1 in ebp in the loops, assigns
// the stack slots value 0x18, queued 0x1c, size 0x20 and the field_0xc0
// flag/loop count 0x24, keeps loop counts in ebp/edx, and cross-jumps fewer
// `UnknownFunction4e8a70(); return 1;` exits.

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
    header = (void*)params->field_0x00;
    headerSize = params->field_0x04;
    name = (char*)params->field_0x08;
    vcr = params->owner;
    file = (UnknownVcrFile*)vcr->field_0xac;
    events[0] = vcr->field_0x30;
    events[1] = vcr->field_0x34;
    events[2] = vcr->field_0x38;
    events[3] = vcr->field_0x40;
    events[4] = vcr->field_0x3c;
    events[5] = vcr->field_0x44;
    events[6] = vcr->field_0x48;
    events[7] = vcr->field_0x4c;
    events[8] = vcr->field_0x50;
    events[9] = vcr->field_0x54;
    events[10] = vcr->field_0x58;
    while ((result = WaitForMultipleObjects(11, events, FALSE, INFINITE)) != WAIT_FAILED) {
        switch (result) {
        case 0: // write the queued records
            if (vcr->field_0xb0->UnknownFunction524350()) {
                void* data;
                int size;
                int index;

                data = 0;
                EnterCriticalSection(&vcr->field_0x60);
                vcr->field_0xb0->UnknownFunction524300(&data, &size, &index);
                LeaveCriticalSection(&vcr->field_0x60);
                if (!file->UnknownFunction5251f0(data, size, 1, handle)) {
                    strcpy(message, "VCRtape file write error\n");
                    goto done;
                }
                EnterCriticalSection(&vcr->field_0x60);
                vcr->field_0xb0->UnknownFunction524540(index);
                LeaveCriticalSection(&vcr->field_0x60);
            }
            if (vcr->field_0xb0->UnknownFunction524350())
                SetEvent(vcr->field_0x30);
            break;
        case 1: // create the file and write the header
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xbc = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            handle = file->UnknownFunction524dd0(name, "wb");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->UnknownFunction5251f0(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header write error\n");
                goto done;
            }
            strcpy(message, name);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xb4 = 0;
            vcr->field_0xc8 = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            break;
        case 2: // reopen the file for appending
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xbc = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            handle = file->UnknownFunction524dd0(name, "rb+");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->UnknownFunction525070(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header (reopen) read error\n");
                goto done;
            }
            file->UnknownFunction5253d0(handle, headerSize, SEEK_SET);
            strcpy(message, name);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xb4 = 0;
            vcr->field_0xc8 = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            break;
        case 3: // read the next record into the ring
            EnterCriticalSection(&vcr->field_0x60);
            if (vcr->field_0xb8) {
                LeaveCriticalSection(&vcr->field_0x60);
                break;
            }
            LeaveCriticalSection(&vcr->field_0x60);
            if (vcr->field_0xb0->UnknownFunction524560()) {
                void* data;
                int position;
                unsigned int size;
                int index;

                data = 0;
                position = file->UnknownFunction525440(handle);
                if (!file->UnknownFunction525070(&size, 4, 1, handle)) {
                    if (!file->UnknownFunction5254a0(handle)) {
                        strcpy(message, "VCRtape file read error (size)\n");
                        goto done;
                    }
                    EnterCriticalSection(&vcr->field_0x60);
                    vcr->field_0xb0->UnknownFunction524940();
                    LeaveCriticalSection(&vcr->field_0x60);
                    break;
                }
                if (size > 0x400) {
                    strcpy(message, "data size exceeds buffer.\n");
                    goto done;
                }
                EnterCriticalSection(&vcr->field_0x60);
                vcr->field_0xb0->UnknownFunction524390(size, &index, &data);
                LeaveCriticalSection(&vcr->field_0x60);
                if (!file->UnknownFunction525070(data, size, 1, handle)) {
                    if (!file->UnknownFunction5254a0(handle)) {
                        strcpy(message, "VCRtape file read error (data)\n");
                        goto done;
                    }
                } else {
                    EnterCriticalSection(&vcr->field_0x60);
                    vcr->field_0xb0->UnknownFunction524440(position, index);
                    LeaveCriticalSection(&vcr->field_0x60);
                }
                if (vcr->field_0xb0->UnknownFunction524560())
                    SetEvent(vcr->field_0x40);
            }
            break;
        case 4: // open the file for playback
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xbc = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            handle = file->UnknownFunction524dd0(name, "rb");
            if (handle == -1) {
                strcpy(message, "Can't create VCRtape file\n");
                goto done;
            }
            if (!file->UnknownFunction525070(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header read error\n");
                goto done;
            }
            vcr->field_0xb0->UnknownFunction5248f0(file->UnknownFunction525440(handle));
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xb4 = 0;
            LeaveCriticalSection(&vcr->field_0x60);
            SetEvent(vcr->field_0x40);
            strcpy(message, name);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xc8 = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            break;
        case 5: // read the record at the next recorded position
            EnterCriticalSection(&vcr->field_0x60);
            if (vcr->field_0xb8) {
                LeaveCriticalSection(&vcr->field_0x60);
                break;
            }
            LeaveCriticalSection(&vcr->field_0x60);
            if (vcr->field_0xb0->UnknownFunction524560()) {
                void* data;
                int position;
                int extra;
                unsigned int size;
                int index;

                data = 0;
                position = vcr->field_0xb0->UnknownFunction5246c0(&extra);
                vcr->field_0xd4 = extra;
                if (position == 0) {
                    EnterCriticalSection(&vcr->field_0x60);
                    vcr->field_0xb0->UnknownFunction524970();
                    LeaveCriticalSection(&vcr->field_0x60);
                    break;
                }
                if (position == -1) {
                    EnterCriticalSection(&vcr->field_0x60);
                    vcr->field_0xb0->UnknownFunction524390(1, &index, &data);
                    *(unsigned char*)data = 0xff;
                    vcr->field_0xb0->UnknownFunction524440(-1, index);
                    LeaveCriticalSection(&vcr->field_0x60);
                } else {
                    file->UnknownFunction5253d0(handle, position, SEEK_SET);
                    if (!file->UnknownFunction525070(&size, 4, 1, handle)) {
                        if (!file->UnknownFunction5254a0(handle)) {
                            strcpy(message, "VCRtape file read error (size)\n");
                            goto done;
                        }
                        EnterCriticalSection(&vcr->field_0x60);
                        vcr->field_0xb0->UnknownFunction524970();
                        LeaveCriticalSection(&vcr->field_0x60);
                        break;
                    }
                    if (size > 0x400) {
                        strcpy(message, "data size exceeds buffer.\n");
                        goto done;
                    }
                    EnterCriticalSection(&vcr->field_0x60);
                    vcr->field_0xb0->UnknownFunction524390(size, &index, &data);
                    LeaveCriticalSection(&vcr->field_0x60);
                    if (!file->UnknownFunction525070(data, size, 1, handle)) {
                        if (!file->UnknownFunction5254a0(handle)) {
                            strcpy(message, "VCRtape file read error (data)\n");
                            goto done;
                        }
                    } else {
                        EnterCriticalSection(&vcr->field_0x60);
                        vcr->field_0xb0->UnknownFunction524440(position, index);
                        LeaveCriticalSection(&vcr->field_0x60);
                    }
                }
                if (vcr->field_0xb0->UnknownFunction524560())
                    SetEvent(vcr->field_0x44);
            }
            break;
        case 6: { // seek back to the previous recorded position
            int extra;
            int position;

            ResetEvent(vcr->field_0x40);
            ResetEvent(vcr->field_0x44);
            position = vcr->field_0xb0->UnknownFunction5247c0(&extra);
            if (position == -1)
                position = vcr->field_0xb0->UnknownFunction5247c0(&extra);
            file->UnknownFunction5253d0(handle, position, SEEK_SET);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xb0->UnknownFunction524910();
            LeaveCriticalSection(&vcr->field_0x60);
            SetEvent(vcr->field_0x40);
            break;
        }
        case 7: { // rewind to the first recorded position
            int position;

            ResetEvent(vcr->field_0x40);
            ResetEvent(vcr->field_0x44);
            position = vcr->field_0xb0->UnknownFunction524900();
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xb0->UnknownFunction524910();
            LeaveCriticalSection(&vcr->field_0x60);
            file->UnknownFunction5253d0(handle, position, SEEK_SET);
            vcr->field_0xbc = 1;
            SetEvent(vcr->field_0x40);
            break;
        }
        case 8:
            ResetEvent(vcr->field_0x40);
            ResetEvent(vcr->field_0x44);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xc0 = 1;
            vcr->field_0xbc = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            break;
        case 9: // rewrite the header and close the file
            file->UnknownFunction5253d0(handle, 0, SEEK_SET);
            if (!file->UnknownFunction5251f0(header, headerSize, 1, handle)) {
                strcpy(message, "VCRtape file header write error\n");
                goto done;
            }
            file->UnknownFunction525000(handle);
            EnterCriticalSection(&vcr->field_0x60);
            vcr->field_0xc8 = 1;
            LeaveCriticalSection(&vcr->field_0x60);
            handle = -1;
            break;
        case 10:
            strcpy(message, "Received normal Kill message.\n");
            goto done;
        }
    }
done:
    EnterCriticalSection(&vcr->field_0x60);
    vcr->field_0xb4 = 0;
    LeaveCriticalSection(&vcr->field_0x60);
    if (handle >= 0)
        file->UnknownFunction525000(handle);
    _endthreadex(0);
    return 0;
}

// 0x004e7d60: per-frame playback step (mode 1); KrustyVCR shares it. Takes
// queued records from the ring and hands them to the callback at +0x80.
// Always returns 1.
int VCRInterface::UnknownVirtualSlot10(float frameTime) {
    int keep;
    int milliseconds;
    int value;
    int queued;
    unsigned int size;
    UnknownRecorderRecord* record;
    int count;
    int result;

    EnterCriticalSection(&field_0x60);
    if (!field_0xbc) {
        LeaveCriticalSection(&field_0x60);
        return 1;
    }
    LeaveCriticalSection(&field_0x60);
    if (field_0x78 != 1)
        return 1;
    if (field_0x88) {
        int flag;

        EnterCriticalSection(&field_0x60);
        flag = field_0xc0;
        LeaveCriticalSection(&field_0x60);
        if (flag) {
            EnterCriticalSection(&field_0x60);
            field_0xc0 = 0;
            field_0xc4 = 1;
            LeaveCriticalSection(&field_0x60);
            UnknownFunction4e89c0();
            return 1;
        }
        EnterCriticalSection(&field_0x60);
        flag = field_0xc4;
        LeaveCriticalSection(&field_0x60);
        if (!flag)
            return 1;
        for (;;) {
            result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 0);
            if (result == 0) {
                if (field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds) != 11) {
                    field_0x88 = 0;
                    UnknownFunction4e8990();
                    field_0xb0->UnknownFunction524870(value);
                    UnknownFunction4e8a20(milliseconds);
                    field_0xc4 = 0;
                    return 1;
                }
            } else if (result == 2) {
                field_0x80(-3, 0, 0, 0, &keep, field_0xd4, &milliseconds);
                field_0x88 = 0;
                EnterCriticalSection(&field_0x60);
                field_0xc4 = 0;
                LeaveCriticalSection(&field_0x60);
                field_0x84 = 0;
                UnknownFunction4e8a70();
                return 1;
            } else if (result == 3) {
                return 1;
            } else if (result == 4) {
                UnknownFunction4e8990();
                return 1;
            }
        }
    }
    queued = 0;
    if (field_0x84) {
        result = field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds);
        if (result == 3)
            return 1;
        if (result == 5) {
            field_0x84 = 1;
            return 1;
        }
        if (result == 12) {
            field_0x84 = 0;
            UnknownFunction4e8a70();
            return 1;
        }
        record = (UnknownRecorderRecord*)field_0x7c;
        field_0xd4 = (int)(record->time * 1000.0f);
        count = record->count;
        field_0x84 = 0;
        while (count--) {
            result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 1);
            if (result == 0) {
                field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds);
                if (keep) {
                    field_0xb0->UnknownFunction524590(value, field_0xd4);
                    queued = 1;
                }
            } else {
                if (result == 1 &&
                    field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds) == 12) {
                    field_0x84 = 0;
                    UnknownFunction4e8a70();
                    return 1;
                }
                break;
            }
        }
        if (queued)
            field_0xb0->UnknownFunction524590(-1, field_0xd4);
        return 1;
    }
    field_0x84 = 0;
    result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &field_0xa8, field_0x7c, &size, 1);
    if (result == 1) {
        result = field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds);
        if (result == 11) {
            field_0x88 = 1;
            UnknownFunction4e8990();
            return 1;
        }
        if (result == 12)
            UnknownFunction4e8a70();
        return 1;
    }
    if (result == 4)
        return 1;
    if (field_0xa0 != -1)
        return 1;
    field_0xb0->UnknownFunction524590(field_0xa8, field_0xd4);
    record = (UnknownRecorderRecord*)field_0x7c;
    field_0xd4 = (int)(record->time * 1000.0f);
    count = record->count;
    switch (field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds)) {
    case 5:
        field_0x84 = 1;
        return 1;
    case 3:
        field_0x84 = 1;
        return 1;
    case 11:
        field_0x88 = 1;
        UnknownFunction4e8990();
        return 1;
    case 12:
        UnknownFunction4e8a70();
        return 1;
    case 2:
        for (;;) {
            while (count--) {
                result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 1);
                if (result == 0) {
                    field_0x80(field_0xa0, field_0x7c, 1, field_0xa4, &keep, field_0xd4, &milliseconds);
                    if (keep) {
                        field_0xb0->UnknownFunction524590(value, field_0xd4);
                        queued = 1;
                    }
                } else if (result == 1) {
                    if (field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds) == 12) {
                        field_0x84 = 1;
                        UnknownFunction4e8a70();
                    }
                    return 1;
                } else if (result == 4) {
                    return 1;
                }
            }
            if (queued)
                field_0xb0->UnknownFunction524590(-1, field_0xd4);
            queued = 0;
            result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &field_0xa8, field_0x7c, &size, 1);
            if (result == 1) {
                result = field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds);
                if (result == 11) {
                    field_0x88 = 1;
                    UnknownFunction4e8990();
                    return 1;
                }
                if (result == 12)
                    UnknownFunction4e8a70();
                return 1;
            }
            if (result == 4)
                return 1;
            result = field_0x80(field_0xa0, field_0x7c, 1, field_0xa4, &keep, field_0xd4, &milliseconds);
            if (result == 1) {
                record = (UnknownRecorderRecord*)field_0x7c;
                field_0xd4 = (int)(record->time * 1000.0f);
                count = record->count;
                while (count--) {
                    result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 1);
                    if (result == 0) {
                        field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds);
                        if (keep) {
                            field_0xb0->UnknownFunction524590(value, field_0xd4);
                            queued = 1;
                        }
                    } else if (result == 1) {
                        if (field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds) == 12) {
                            field_0x84 = 1;
                            UnknownFunction4e8a70();
                        }
                        return 1;
                    } else if (result == 4) {
                        return 1;
                    }
                }
                break;
            }
            if (result == 3) {
                field_0x84 = 1;
                return 1;
            }
            if (result == 11) {
                field_0x88 = 1;
                UnknownFunction4e8990();
                return 1;
            }
            if (result == 12) {
                UnknownFunction4e8a70();
                return 1;
            }
            record = (UnknownRecorderRecord*)field_0x7c;
            field_0xd4 = (int)(record->time * 1000.0f);
            count = record->count;
        }
        break;
    case 1:
        while (count--) {
            result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 1);
            if (result == 0) {
                field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds);
                if (keep) {
                    field_0xb0->UnknownFunction524590(value, field_0xd4);
                    queued = 1;
                }
            } else if (result == 1) {
                if (field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds) == 12) {
                    field_0x84 = 1;
                    UnknownFunction4e8a70();
                }
                return 1;
            } else if (result == 4) {
                return 1;
            }
        }
        break;
    case 7:
        while (count--) {
            result = UnknownFunction4e8810(&field_0xa0, &field_0xa4, &value, field_0x7c, &size, 1);
            if (result == 0) {
                field_0x80(field_0xa0, field_0x7c, 0, field_0xa4, &keep, field_0xd4, &milliseconds);
                if (keep) {
                    field_0xb0->UnknownFunction524590(value, field_0xd4);
                    queued = 1;
                }
            } else if (result == 1) {
                if (field_0x80(-2, 0, 0, 0, &keep, field_0xd4, &milliseconds) == 12) {
                    field_0x84 = 1;
                    UnknownFunction4e8a70();
                }
                return 1;
            } else if (result == 4) {
                return 1;
            }
        }
        field_0x84 = 1;
        break;
    default:
        return 1;
    }
    if (queued)
        field_0xb0->UnknownFunction524590(-1, field_0xd4);
    return 1;
}
