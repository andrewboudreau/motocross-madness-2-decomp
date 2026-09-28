// RTTI confirms class Game and its vtable at 0x0055299c.
// Vtable slots 3 and 4 both point at 0x00468c90. The body is unambiguous,
// but the original method name/signature spelling is not recovered yet.
class Game {
public:
    virtual int UnknownVirtualSlot1();
    virtual int UnknownVirtualSlot3();
};

int Game::UnknownVirtualSlot1() {
    return 1;
}

int Game::UnknownVirtualSlot3() {
    return 1;
}
