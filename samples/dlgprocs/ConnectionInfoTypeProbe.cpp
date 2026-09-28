// ConnectionInfoType is confirmed by RTTI. Vtable slot 1 points at a one-byte
// `ret`, so the body is an unambiguous no-op. The original method name is not known.
class ConnectionInfoType {
public:
    virtual void UnknownVirtualSlot1();
};

void ConnectionInfoType::UnknownVirtualSlot1() {
}
