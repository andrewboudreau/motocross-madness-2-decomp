// RTTI confirms FollowCamera : PCCamera. FollowCamera introduces slots 63-67; VehicleCamera/BikeCamera inherit these addresses unchanged.
// FollowCam.cpp is the strongest recovered translation-unit candidate (filename overlap + nearby __FILE__ xrefs).
// Semantic field names remain unknown; preserve raw offsets until call-site evidence improves them.
class FollowCamera {
public:
    virtual void UnknownVirtualSlot63();
    virtual void UnknownVirtualSlot64();
    virtual void UnknownVirtualSlot65();
    virtual void UnknownVirtualSlot66();
    virtual void UnknownVirtualSlot67();
};

// Semantically strong but compiler-sensitive: retail VC6 zeros EAX once and reuses it.
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
