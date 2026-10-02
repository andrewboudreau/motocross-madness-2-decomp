// Ordinary C++98 candidate, i386 MSVC ABI. All names below are provisional.
// Offset accessors are a probe, not a declaration of the original class layout.
#define MCM2_ECO_INLINE __forceinline
#include "EcoSystemPass.h"
using namespace mcm2_eco;
struct LedgerProbe { int Select(const char*); void Restore(int); };
struct EpochProbe { Word tick; Word Totals(int*); void Trim(Word); void Advance(); };
struct IteratorProbe { void Reset(); RecordPrefix* Next(); };
struct GeometryProbe { int Test(FramePrefix*, void*, Position*, float, int); };
struct EcoSystemProbe { int RunCandidate(); };
extern "C" {
extern int eco_enabled;
extern LedgerProbe* eco_ledger;
extern IteratorProbe* eco_iterator;
extern GeometryProbe* eco_geometry;
extern EcoSystemProbe* eco_global_instance;
extern char eco_category[], eco_source_path[];
extern char* eco_published_frame;
extern Word eco_elapsed;
extern float eco_constant_65535, eco_constant_inverse;
extern double eco_constant_half;
Word eco_sample_time();
void* eco_reallocate(void*, Word, const char*, int);
}
struct RawView {
    char* self;
    explicit RawView(EcoSystemProbe* p):self(reinterpret_cast<char*>(p)){}
    template<class T> T& at(int offset) { return *reinterpret_cast<T*>(self + offset); }
    bool ready(){return at<void*>(0x34)!=0;}
    EpochProbe* scratch(){return at<EpochProbe*>(0x59c);}
    FramePrefix* frame(){return *reinterpret_cast<FramePrefix**>(at<char*>(0x18)+8);}
    unsigned char group(){return at<unsigned char>(0x599);}
    float& inverse_scale(){return at<float>(0x5b8);}
    float& scaled_extent(){return at<float>(0x5bc);}
    Word& count_a(){return at<Word>(0x5a0);} Word& count_b(){return at<Word>(0x5a4);}
    Word& capacity_a(){return at<Word>(0x5b0);} Word& capacity_b(){return at<Word>(0x5b4);}
    RecordPrefix**& list_a(){return at<RecordPrefix**>(0x3c);}
    RecordPrefix**& list_b(){return at<RecordPrefix**>(0x40);}
    DefinitionPrefix* definition(unsigned char i){return at<DefinitionPrefix*>(0x58+4*i);}
};
struct RawOps {
    typedef double Real; // numeric candidate only: does not emulate the x87 control word
    bool enabled(){return eco_enabled!=0;}
    int select(){return eco_ledger->Select(eco_category);}
    void restore(int v){eco_ledger->Restore(v);}
    Word sample(){return eco_sample_time();}
    Word totals(EpochProbe* p,int* n){return p->Totals(n);}
    void trim(EpochProbe* p,Word n){p->Trim(n);}
    void advance(EpochProbe* p){p->Advance();}
    void publish(FramePrefix* f){eco_published_frame=reinterpret_cast<char*>(f)+0xac;}
    float constant_65535(){return eco_constant_65535;}
    float constant_inverse(){return eco_constant_inverse;}
    double constant_half(){return eco_constant_half;}
    void reset_iterator(){eco_iterator->Reset();}
    RecordPrefix* next(){return eco_iterator->Next();}
    void evaluate(RecordPrefix* p,int* mode,int* state){p->Evaluate(mode,state);}
    float metric15(DefinitionPrefix* d,unsigned char p){return d->Metric15(p);}
    float metric14(DefinitionPrefix* d,unsigned char p){return d->Metric14(p);}
    float coordinate_scale(){return *reinterpret_cast<float*>(reinterpret_cast<char*>(eco_global_instance)+0x5a8);}
    int test_geometry(FramePrefix* frame,Position* p,float bound,int flags){
        return eco_geometry->Test(frame,reinterpret_cast<char*>(frame)+0xec,p,bound,flags);
    }
    void apply(RecordPrefix* p,int mode,int state){p->Apply(mode,state);}
    RecordPrefix** reallocate(RecordPrefix** p,Word n,int line){
        return static_cast<RecordPrefix**>(eco_reallocate(p,n,eco_source_path,line));
    }
    void elapsed(Word value){eco_elapsed=value;}
};
int EcoSystemProbe::RunCandidate(){RawView self(this);RawOps ops;return mcm2_eco::Run(self,ops);}
// Probe layout floors only; none asserts a complete game class size.
typedef char check_record_prefix[sizeof(RecordPrefix)==0x18?1:-1];
typedef char check_word[sizeof(Word)==4?1:-1];
typedef char check_pointer[sizeof(void*)==4?1:-1];
