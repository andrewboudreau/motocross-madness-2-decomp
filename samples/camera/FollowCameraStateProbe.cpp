// Evidence-backed FollowCamera state/math candidates.
// RTTI confirms FollowCamera : PCCamera and the primary vtable owns slots 68/69/71.
// Names and the CameraValue3 semantic type remain provisional.

extern "C" float __cdecl MCM2Sqrt(float value);

struct CameraValue3 {
    float x;
    float y;
    float z;
};

class FollowCamera {
public:
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27();
    virtual void v28();
    virtual void UnknownVirtualSlot29(CameraValue3 value);
    virtual void v30(); virtual void v31(); virtual void v32(); virtual void v33();
    virtual void v34(); virtual void v35(); virtual void v36(); virtual void v37();
    virtual void v38(); virtual void v39(); virtual void v40(); virtual void v41();
    virtual void v42();
    virtual void UnknownVirtualSlot43(CameraValue3* value);
    virtual void v44(); virtual void v45(); virtual void v46(); virtual void v47();
    virtual void v48(); virtual void v49(); virtual void v50(); virtual void v51();
    virtual void v52(); virtual void v53(); virtual void v54(); virtual void v55();
    virtual void v56();
    virtual CameraValue3 UnknownVirtualSlot57(float value);
    virtual void v58(); virtual void v59(); virtual void v60(); virtual void v61(); virtual void v62();
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66();
    virtual void v67();
    virtual void UnknownVirtualSlot68();
    virtual void UnknownVirtualSlot69();
    virtual void v70();
    virtual void UnknownVirtualSlot71(int value);
    virtual void v72(); virtual void v73(); virtual void v74(); virtual void v75();
};

// Retail 0x00466D50.
// The helper at 0x00460B50 is sqrt-like from its input/output behavior; its
// original name is not known, so MCM2Sqrt is intentionally provisional.
void FollowCamera::UnknownVirtualSlot68() {
    char* p = reinterpret_cast<char*>(this);
    CameraValue3 value = *reinterpret_cast<CameraValue3*>(p + 0x2B4);
    value.y += 3.0f;

    float dx = *reinterpret_cast<float*>(p + 0x170) - value.x;
    if (dx < 0.0f) dx = -dx;
    float dz = *reinterpret_cast<float*>(p + 0x178) - value.z;
    if (dz < 0.0f) dz = -dz;

    float distance = MCM2Sqrt(dx * dx + dz * dz);
    float result = (200.0f - distance) * (1.0f / 180.0f) * 60.0f + 10.0f;
    *reinterpret_cast<float*>(p + 0x258) = result;

    if (*reinterpret_cast<float*>(p + 0x258) < 10.0f)
        *reinterpret_cast<float*>(p + 0x258) = 10.0f;
    if (*reinterpret_cast<float*>(p + 0x258) > 70.0f)
        *reinterpret_cast<float*>(p + 0x258) = 70.0f;

    UnknownVirtualSlot29(value);
}

// Retail 0x00466A80.
// VehicleCamera's slot-57 override proves MSVC's hidden return-buffer shape:
// slot 57 returns a 12-byte value by value and takes one explicit float arg.
void FollowCamera::UnknownVirtualSlot69() {
    char* p = reinterpret_cast<char*>(this);
    CameraValue3 value = UnknownVirtualSlot57(0.0f);
    *reinterpret_cast<CameraValue3*>(p + 0x2A8) = value;
    UnknownVirtualSlot43(reinterpret_cast<CameraValue3*>(p + 0x2A8));
}

// Retail 0x00466E50.
void FollowCamera::UnknownVirtualSlot71(int value) {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<int*>(p + 0x244) = value;
    v58();

    switch (*reinterpret_cast<int*>(p + 0x244)) {
    case 0:
        UnknownVirtualSlot66();
        break;
    case 1:
        UnknownVirtualSlot65();
        break;
    case 2:
        UnknownVirtualSlot64();
        break;
    case 3:
        *reinterpret_cast<unsigned int*>(p + 0x258) =
            *reinterpret_cast<unsigned int*>(p + 0x2F0);
        UnknownVirtualSlot63();
        break;
    case 4:
        *reinterpret_cast<unsigned int*>(p + 0x2F0) =
            *reinterpret_cast<unsigned int*>(p + 0x258);
        v60();
        break;
    }

    *reinterpret_cast<unsigned int*>(p + 0x2C4) =
        *reinterpret_cast<unsigned int*>(p + 0x220);
    *reinterpret_cast<unsigned int*>(p + 0x2C8) =
        *reinterpret_cast<unsigned int*>(p + 0x22C);
    *reinterpret_cast<unsigned int*>(p + 0x2CC) =
        *reinterpret_cast<unsigned int*>(p + 0x234);
    v61();
}
