// QuadTreeObject is confirmed by RTTI. Slot 1 takes one 4-byte argument and
// unconditionally returns 15. The argument's semantic type/name is unknown.
class QuadTreeObject {
public:
    virtual int UnknownVirtualSlot1(int unknown);
};
int QuadTreeObject::UnknownVirtualSlot1(int) {
    return 15;
}
