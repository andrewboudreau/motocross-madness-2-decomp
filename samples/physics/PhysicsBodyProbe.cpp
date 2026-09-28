// PhysicsBody is confirmed by RTTI. Slot 31 is shared with PhysicsRigidBody and
// stores its sole 4-byte argument at this+0x180. Method/member names are unknown.
class PhysicsBody {
public:
    virtual void UnknownVirtualSlot31(int value);
};
void PhysicsBody::UnknownVirtualSlot31(int value) {
    *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x180) = value;
}
