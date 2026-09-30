class ReadinessProbe {
public:
    int value;
    int GetValue() const { return value; }
};
extern "C" int Vc6Readiness(ReadinessProbe* p) {
    return p->GetValue() + 1;
}
