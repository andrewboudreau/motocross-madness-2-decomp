// RTTI confirms FollowCamera : PCCamera. FollowCamera introduces slots 63-72;
// VehicleCamera/BikeCamera inherit these addresses unchanged where their vtables
// do not override them.
//
// FollowCam.cpp is the strongest recovered translation-unit candidate
// (filename overlap + nearby __FILE__ xrefs).
//
// Semantic field names remain unknown; preserve raw offsets until call-site
// evidence improves them.
class FollowCamera {
public:
    virtual void v00(); virtual void v01(); virtual void v02(); virtual void v03();
    virtual void v04(); virtual void v05(); virtual void v06(); virtual void v07();
    virtual void v08(); virtual void v09(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
    virtual void v20(); virtual void v21(); virtual void v22(); virtual void v23();
    virtual void v24(); virtual void v25(); virtual void v26(); virtual void v27();
    virtual void v28(); virtual void v29(); virtual void v30(); virtual void v31();
    virtual void v32(); virtual void v33(); virtual void v34(); virtual void v35();
    virtual void v36(); virtual void v37(); virtual void v38(); virtual void v39();
    virtual void v40(); virtual void v41(); virtual void v42(); virtual void v43();
    virtual void v44(); virtual void v45(); virtual void v46(); virtual void v47();
    virtual void v48(); virtual void v49(); virtual void v50(); virtual void v51();
    virtual void v52(); virtual void v53(); virtual void v54(); virtual void v55();
    virtual void v56(); virtual void v57(); virtual void v58(); virtual void v59();
    virtual void v60(); virtual void v61(); virtual void v62();

    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
    virtual void v68();
    virtual void v69();
    virtual void UnknownVirtualSlot70(int value);
    virtual void UnknownVirtualSlot71(int value);
    virtual void UnknownVirtualSlot72();
};

// Semantically strong but compiler-sensitive: retail VC6 zeros EAX once and
// reuses it. Modern clang emits immediate-zero stores instead.
void FollowCamera::UnknownVirtualSlot63() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<int*>(p + 0x22C) = 0;
    *reinterpret_cast<int*>(p + 0x234) = 0;
    *reinterpret_cast<int*>(p + 0x220) = 0;
}

void FollowCamera::UnknownVirtualSlot64() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F76FE4Eu;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0xBE832E86u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x4297F5C3u;
}

void FollowCamera::UnknownVirtualSlot65() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F2EEC82u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0xBF1C1D07u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x41B570A4u;
}

void FollowCamera::UnknownVirtualSlot66() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F860A91u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0x00000000u;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x413B851Fu;
    *reinterpret_cast<unsigned int*>(p + 0x258) = 0x42AA0000u;
}

void FollowCamera::UnknownVirtualSlot67() {
    char* p = reinterpret_cast<char*>(this);
    *reinterpret_cast<unsigned int*>(p + 0x22C) = 0x3F333333u;
    *reinterpret_cast<unsigned int*>(p + 0x234) = 0x40490FDBu;
    *reinterpret_cast<unsigned int*>(p + 0x220) = 0x41700000u;
    *reinterpret_cast<unsigned int*>(p + 0x258) = 0x42AA0000u;
}

// Strong semantic reconstruction, but not a clang smoke target.
void FollowCamera::UnknownVirtualSlot70(int value) {
    char* p = reinterpret_cast<char*>(this);

    *reinterpret_cast<int*>(p + 0x268) = value & 0xFF;

    if (static_cast<unsigned char>(value) != 0) {
        int current = *reinterpret_cast<int*>(p + 0x244);

        if (current != 5) {
            *reinterpret_cast<int*>(p + 0x248) = current;
            *reinterpret_cast<int*>(p + 0x24C) =
                *reinterpret_cast<int*>(p + 0x258);
            *reinterpret_cast<int*>(p + 0x244) = 5;
        }

        *reinterpret_cast<int*>(p + 0x26C) = 0;
    } else {
        *reinterpret_cast<int*>(p + 0x244) =
            *reinterpret_cast<int*>(p + 0x248);
        *reinterpret_cast<int*>(p + 0x258) =
            *reinterpret_cast<int*>(p + 0x24C);
    }
}

// Retail behavior strongly matches a simple cyclic state-list advance.
// VC6 keeps the increment/store/compare/wrap sequence explicit; modern clang
// folds the wrap through CMOV on P6+ CPU targets, making this another useful
// historical compiler/profile calibration target.
void FollowCamera::UnknownVirtualSlot72() {
    char* p = reinterpret_cast<char*>(this);

    int index = *reinterpret_cast<int*>(p + 0x30C) + 1;
    *reinterpret_cast<int*>(p + 0x30C) = index;

    if (index >= *reinterpret_cast<int*>(p + 0x310)) {
        *reinterpret_cast<int*>(p + 0x30C) = 0;
    }

    index = *reinterpret_cast<int*>(p + 0x30C);
    int value = *reinterpret_cast<int*>(p + 0x314 + index * 4);

    *reinterpret_cast<int*>(p + 0x244) = value;
    UnknownVirtualSlot71(value);
}
