#pragma once

// The object behind the global pointer at 0x0056e26c, used throughout the
// camera code. Its class is not established; only members that reconstructed
// functions touch are declared, at their observed offsets.

// Interface at (+0x14)->+0x34; FollowCamera slots 55 and 56 call its slot 5.
class UnknownInterface56e26c {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual void UnknownVirtualSlot2();
    virtual void UnknownVirtualSlot3();
    virtual void UnknownVirtualSlot4();
    virtual bool UnknownVirtualSlot5(int a, int b, int c);
};

// Object at +0x14. KrustyBikeCamera slot 55 calls its virtual slot 2.
class UnknownObject56e26cPart {
public:
    virtual void UnknownVirtualSlot0();
    virtual void UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot2(int a, int b);

    unsigned char field_0x04[0x30];
    UnknownInterface56e26c* field_0x34;
};

// Object returned by (+0x570)->0x0045d340; KrustyBikeCamera slot 58 hands it
// a message.
class UnknownMessageTarget;

class UnknownObject56e26cList {
public:
    UnknownMessageTarget* UnknownFunction45d340(); // 0x0045d340
};

struct UnknownKrustyBikeView;

struct UnknownObject56e26cViewOwner {
    unsigned char field_0x00[0x34];
    UnknownKrustyBikeView* field_0x34;
};

// Object embedded at +0x578; KrustyBikeCamera slot 52 reads its mode.
class UnknownObject56e26cMode {
public:
    int UnknownFunction524100(); // 0x00524100
};

class UnknownObject56e26c {
public:
    void UnknownFunction468880(); // 0x00468880 (PCCamera slot 27)
    // 0x00521970: copies string `id` into buffer (size bytes).
    void UnknownFunction521970(int id, char* buffer, int size);

    unsigned char field_0x0000[0x14];
    UnknownObject56e26cPart* field_0x14;
    int field_0x18;      // KrustyBikeCamera slot 42 tests > 1
    unsigned char field_0x001c[0x558 - 0x1c];
    // Objects whose +0x34 KrustyBikeCamera slot 10 takes as its view, by
    // field_0x2d74.
    UnknownObject56e26cViewOwner* field_0x558;
    UnknownObject56e26cViewOwner* field_0x55c;
    UnknownObject56e26cViewOwner* field_0x560;
    UnknownObject56e26cViewOwner* field_0x564;
    UnknownObject56e26cViewOwner* field_0x568;
    int field_0x56c;
    UnknownObject56e26cList* field_0x570;
    int field_0x574;
    UnknownObject56e26cMode field_0x578;
    unsigned char field_0x0579[0x2930 - 0x579];
    int field_0x2930;    // saved KrustyBikeCamera state (slots 61, 62)
    float field_0x2934;  // saved KrustyBikeCamera presets (slots 59, 60)
    float field_0x2938;
    float field_0x293c;
    float field_0x2940;
    unsigned char field_0x2944[0x2d74 - 0x2944];
    int field_0x2d74;    // selects KrustyBikeCamera's view (slot 10)
    unsigned char field_0x2d78[0x3430 - 0x2d78];
    int field_0x3430;    // blocks KrustyBikeCamera slot 23
};

extern UnknownObject56e26c* g_UnknownGlobal56e26c;
