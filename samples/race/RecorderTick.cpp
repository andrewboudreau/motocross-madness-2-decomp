// Complete playback tick candidate; see docs/RECORDER_TICK.md.
// Original translation-unit ownership is not established by this extraction.
#include <process.h>
#include <string.h>
#include <windows.h>
#include "../../src/reconstructed/Recorder.h"
#include "../../src/reconstructed/DebugAlloc.h"
#include "../../src/reconstructed/MatrixUtil.h"
#include "../../src/reconstructed/VCR.h"

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
                record = (UnknownRecorderRecord*)recordBuffer;
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
