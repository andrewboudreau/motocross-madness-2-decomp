// Private differential execution of complete standalone candidate objects.
// Only node-cache dependencies are scripted. Math dependencies execute their
// hash-pinned retail bodies in both runs; production never loads retail code.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/Math3D.h"

static unsigned char referenceSpace[0x800000];
static unsigned char nodeStorage[4][sizeof(SoultreeObject)];
static Matrix4 externalMatrix;
static Vec3 externalAxes[2], resultVector;
struct Event { int kind, node; Matrix4 local, world; };
static Event events[16];
static int eventCount, seedValue;
static SoultreeObject* Node(int n) { return (SoultreeObject*)nodeStorage[n]; }
static int NodeIndex(SoultreeObject* p) {
    for (int n=0;n<4;++n) if (p==Node(n)) return n;
    ExitProcess(81); return -1;
}
static void EventFor(int kind, SoultreeObject* node) {
    if (eventCount>=16) ExitProcess(82);
    Event& e=events[eventCount++]; e.kind=kind; e.node=NodeIndex(node);
    e.local=node->localMatrix; e.world=node->worldMatrix;
}
static void Fill(Matrix4* m, int seed) {
    for (int i=0;i<16;++i) {
        DWORD bits=(DWORD)(seed*1664525u+i*1013904223u);
        if (seed%3==1) {
            // Finite signed values with fractional mantissas, ~2^-10..2^10.
            bits=(bits&0x807fffff)|((117+(bits%21))<<23);
            memcpy((float*)m+i,&bits,4);
        } else ((float*)m)[i]=(float)((seed*13+i*7)%31-15)*0.125f;
    }
    // Include ordinary rigid transforms as well as general diagnostic matrices.
    if (!(seed%3)) {
        memset(m,0,sizeof(*m)); m->_11=m->_22=m->_33=m->_44=1;
        m->_41=(float)seed; m->_42=-2.5f; m->_43=0.125f;
    }
}
void SoultreeObject::UpdateWorldMatrix() {
    EventFor(1,this);
    if (!worldValid) { Fill(&worldMatrix,seedValue+NodeIndex(this)+39); worldValid=1; }
}
void SoultreeObject::InvalidateWorldMatrix() {
    EventFor(2,this); worldValid=0;
}
void SoultreeObject::SetLocalMatrix(const Matrix4* m) {
    EventFor(3,this); localMatrix=*m; worldValid=0;
}

// The candidate's actual external helper signatures, resolved by this fixture
// to the same arithmetic bodies the retail caller uses. Provider byte identity
// and real-game linkage are checked separately in Python/the game build.
void CollisionMatrixMultiply(Matrix4* out,const Matrix4* a,const Matrix4* b) {
    ((void(__cdecl*)(Matrix4*,const Matrix4*,const Matrix4*))0x436500)(out,a,b);
}
void MatrixMultiply(Matrix4* out,Matrix4 a,Matrix4 b) {
    ((void(__cdecl*)(Matrix4*,Matrix4,Matrix4))0x42a1a0)(out,a,b);
}
void Vec3TransformNormalTranspose(Vec3* out,Vec3 v,const Matrix4* m) {
    ((void(__cdecl*)(Vec3*,Vec3,const Matrix4*))0x42a4b0)(out,v,m);
}
Vec3 CrossProductCall(const Vec3& a,const Vec3& b) {
    return ((Vec3(__cdecl*)(const Vec3&,const Vec3&))0x515600)(a,b);
}
Vec3 Vec3Normalize(const Vec3& v) {
    return ((Vec3(__cdecl*)(const Vec3&))0x5087b0)(v);
}

template<class T> static DWORD MethodAddress(T method) {
    if (sizeof(method)!=sizeof(DWORD)) ExitProcess(83);
    DWORD address; memcpy(&address,&method,4); return address;
}
static int MapReference(const char* path,const char* patches) {
    // The private fixture's large BSS reserves this range before process heaps
    // are created. Assert that every reference byte stays inside that array.
    BYTE* mapped=(BYTE*)0x400000; DWORD old;
    if ((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
        !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) return 0;
    FILE* file=fopen(path,"rb"); if (!file) return 0;
    fseek(file,0,SEEK_END); long size=ftell(file); rewind(file);
    BYTE* raw=(BYTE*)malloc(size);
    if (!raw || fread(raw,1,size,file)!=(size_t)size) return 0;
    fclose(file);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;
    IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if (pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000) return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);
    IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for (int s=0;s<pe->FileHeader.NumberOfSections;++s) {
        if (section[s].PointerToRawData+section[s].SizeOfRawData>(DWORD)size ||
            section[s].VirtualAddress+section[s].SizeOfRawData>pe->OptionalHeader.SizeOfImage) return 0;
        memcpy(mapped+section[s].VirtualAddress,raw+section[s].PointerToRawData,section[s].SizeOfRawData);
    }
    free(raw);
    file=fopen(patches,"rb"); if (!file) return 0;
    DWORD pair[2]; int count=0;
    while (fread(pair,4,2,file)==2) {
        DWORD target=0;
        if (pair[1]==0x4fb4f0) target=MethodAddress(&SoultreeObject::UpdateWorldMatrix);
        if (pair[1]==0x4fb850) target=MethodAddress(&SoultreeObject::InvalidateWorldMatrix);
        if (pair[1]==0x4fca30) target=MethodAddress(&SoultreeObject::SetLocalMatrix);
        if (!target || *(BYTE*)pair[0]!=0xe8) return 0;
        *(DWORD*)(pair[0]+1)=target-pair[0]-5; ++count;
    }
    fclose(file);
    if (count!=9) return 0;
    // The same table initialization used by the retail normalization helper.
    ((void(__cdecl*)())0x460bb0)();
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}

struct Snapshot {
    unsigned char nodes[sizeof(nodeStorage)]; Matrix4 matrix;
    Vec3 axes[2], result; int count; Event calls[16];
};
static void Capture(Snapshot* s) {
    memset(s,0,sizeof(*s)); memcpy(s->nodes,nodeStorage,sizeof(nodeStorage));
    s->matrix=externalMatrix; memcpy(s->axes,externalAxes,sizeof(externalAxes));
    s->result=resultVector; s->count=eventCount; memcpy(s->calls,events,sizeof(events));
}
static void Reset(int seed,int parent,int cached) {
    memset(nodeStorage,0x63,sizeof(nodeStorage)); memset(events,0,sizeof(events));
    eventCount=0; seedValue=seed;
    for (int n=0;n<4;++n) {
        Fill(&Node(n)->localMatrix,seed+n*3); Fill(&Node(n)->worldMatrix,seed+n*3+7);
        Node(n)->worldValid=(cached>>n)&1; Node(n)->parent=0;
        Node(n)->firstChild=Node(n)->nextSibling=Node(n)->prevSibling=0;
    }
    Node(0)->parent=parent<0?0:Node(parent);
    Fill(&externalMatrix,seed+21);
    externalAxes[0]=Vec3((float)(seed%7-3),0.5f,-1.25f);
    externalAxes[1]=Vec3(0.25f,(float)(seed%5-2),1.5f);
    if (seed%8==0) externalAxes[0]=externalAxes[1]=Vec3(0,0,0);
    if (seed%8==1) externalAxes[1]=externalAxes[0];
    resultVector=Vec3(19,23,29);
}
static Matrix4* MatrixInput(int alias) {
    if (!alias) return &externalMatrix;
    return alias&1?&Node((alias-1)/2)->localMatrix:&Node((alias-1)/2)->worldMatrix;
}
static Vec3* AxisInput(int alias,int axis) {
    if (!alias) return &externalAxes[axis];
    return (Vec3*)((float*)MatrixInput(alias)+axis*4);
}
static void Invoke(int method,DWORD address,int frame,int alias,int flags,int resultAlias) {
    SoultreeObject* f=frame<0?0:Node(frame);
    if (method==0) {
        // Spell the hidden result-buffer ABI explicitly to test output aliases.
        typedef Vec3*(SoultreeObject::*Fn)(Vec3*,const Vec3&);
        Fn fn; memcpy(&fn,&address,4);
        Vec3* input=AxisInput(alias,0);
        int shift=seedValue%2+1;
        if (alias && seedValue%4>=2) shift=-shift;
        Vec3* out=flags==0?&resultVector:flags==1?input:flags==2?
            AxisInput(resultAlias,1):(Vec3*)((float*)input+shift);
        if ((Node(0)->*fn)(out,*input)!=out) ExitProcess(84);
    } else if (method==1) {
        typedef void(SoultreeObject::*Fn)(SoultreeObject*,Matrix4*);
        Fn fn; memcpy(&fn,&address,4); (Node(0)->*fn)(f,MatrixInput(alias));
    } else {
        typedef void(SoultreeObject::*Fn)(SoultreeObject*,const Vec3*,const Vec3*,int,int);
        Fn fn; memcpy(&fn,&address,4);
        Vec3* z=AxisInput(alias,0);
        Vec3* y=seedValue%5==2?z:seedValue%5==3?(Vec3*)((float*)z+1):
            AxisInput(seedValue%5==4?(alias+3)%7:alias,1);
        (Node(0)->*fn)(f,z,y,flags&1?2:0,flags&2?-1:0);
    }
}
int main(int argc,char** argv) {
    if (argc!=6 || !MapReference(argv[1],argv[2])) return 3;
    DWORD candidate[3]; for (int i=0;i<3;++i) candidate[i]=strtoul(argv[i+3],0,16);
    const DWORD retail[3]={0x4fd710,0x4fb8c0,0x4fc050};
    Snapshot expected, actual; int checks=0;
    for (int method=0;method<3;++method) for (int seed=0;seed<64;++seed)
    for (int parent=-1;parent<3;++parent) for (int frame=-1;frame<3;++frame)
    for (int alias=0;alias<7;++alias) for (int flags=0;flags<4;++flags) {
        int cached=(seed+flags)%16, resultAlias=(seed+alias)%7;
        Reset(seed,parent,cached); Invoke(method,retail[method],frame,alias,flags,resultAlias); Capture(&expected);
        Reset(seed,parent,cached); Invoke(method,candidate[method],frame,alias,flags,resultAlias); Capture(&actual);
        if (memcmp(&expected,&actual,sizeof(expected))) {
            printf("Mismatch method%d seed%d parent%d frame%d alias%d flags%d calls%d/%d\n",
                method,seed,parent,frame,alias,flags,expected.count,actual.count);
            for (int b=0;b<sizeof(expected);++b) if (((BYTE*)&expected)[b]!=((BYTE*)&actual)[b]) {
                printf("first byte %d: %02x/%02x\n",b,((BYTE*)&expected)[b],((BYTE*)&actual)[b]); break;
            }
            return 2;
        }
        ++checks;
    }
    printf("Soultree transforms: %d complete-state and alias scenarios match\n",checks);
    return 0;
}
