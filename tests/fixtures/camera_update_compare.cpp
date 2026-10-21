// Private whole-state execution of the actual Camera::UpdateMatrices candidate.
#include <windows.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../src/reconstructed/Camera.h"
static unsigned char referenceSpace[0x800000];
static unsigned char cameraStorage[0x240];
static Camera* Object(){return (Camera*)(cameraStorage+16);}
static float& Float(int at){return *(float*)((BYTE*)Object()+at);}
static int& Integer(int at){return *(int*)((BYTE*)Object()+at);}
static int seed,mutation,throwAt,callCount;
static DWORD candidateAddress;
struct Event { int kind;float args[32];unsigned char camera[0x240]; };
struct Observation { int exception,count;unsigned int fpStatus;Event calls[5];unsigned char camera[0x240]; };
static Observation observed,expected;
static void Fail(int code){printf("Fixture invariant %d\n",code);ExitProcess(code);}
struct CameraLayout:Camera {
    static void Verify(){
        if(sizeof(Camera)!=0x220 || sizeof(Vector3)!=12 || sizeof(Matrix4)!=64 ||
           offsetof(CameraLayout,viewMatrix)!=0xac || offsetof(CameraLayout,projectionMatrix)!=0x6c ||
           offsetof(CameraLayout,screenMatrix)!=0xec || offsetof(CameraLayout,focalLength)!=0x198 ||
           offsetof(CameraLayout,viewport)!=0x1a0 || offsetof(CameraLayout,fieldOfView)!=0x16c ||
           offsetof(CameraLayout,eyePosition)!=0x170 || offsetof(CameraLayout,nearPlane)!=0x1bc)Fail(81);
    }
};
#ifdef CAMERA_REAL_HELPERS
void InitFastMath();
#endif
#ifndef CAMERA_REAL_HELPERS
static Event& Record(int kind){
    if(observed.count>=5)Fail(82);Event& e=observed.calls[observed.count++];e.kind=kind;
    memcpy(e.camera,cameraStorage,sizeof(cameraStorage));++callCount;return e;
}
static Matrix4 Result(int kind){
    Matrix4 m;for(int i=0;i<16;++i)((float*)&m)[i]=(seed*19+kind*37+i-100)*0.125f;
    if(mutation&1) { Integer(0x1a8)=320+callCount*19;Integer(0x1ac)=240-callCount*7; }
    if(mutation&2) { Float(0x16c)=33.75f+callCount;Float(0x1bc)=1.25f+callCount;Float(0x1c0)=1234.5f-callCount; }
    if(mutation&4) {
        for(int j=0;j<9;++j)Float(0x170+j*4)=j+callCount*0.75f;
        for(j=0;j<16;++j)Float(0xac+j*4)=j*1.5f+callCount;
    }
    if(throwAt==callCount)throw 70+callCount;
    return m;
}
Matrix4 ViewMatrix(Vector3 from,Vector3 direction,Vector3 up,float roll){
    Event& e=Record(1);memcpy(e.args,&from,12);memcpy(e.args+3,&direction,12);
    memcpy(e.args+6,&up,12);memcpy(e.args+9,&roll,4);return Result(1);
}
Matrix4 ProjectionMatrix(float nearPlane,float farPlane,float fov,float aspect){
    Event& e=Record(2);memcpy(e.args,&nearPlane,4);memcpy(e.args+1,&farPlane,4);
    memcpy(e.args+2,&fov,4);memcpy(e.args+3,&aspect,4);return Result(2);
}
Matrix4 IdentityMatrix(){Record(3);return Result(3);}
Matrix4 MatrixMult(Matrix4 a,Matrix4 b){
    Event& e=Record(4);memcpy(e.args,&a,64);memcpy(e.args+16,&b,64);return Result(4);
}
#endif
static DWORD Replacement(DWORD target){
    switch(target){case 0x4a1500:return (DWORD)ViewMatrix;case 0x4a1460:return (DWORD)ProjectionMatrix;
    case 0x4a1410:return (DWORD)IdentityMatrix;case 0x4a1860:return (DWORD)MatrixMult;}return 0;
}
static int MapReference(const char* path,const char* patches) {
    BYTE* mapped=(BYTE*)0x400000;DWORD old;
    if((DWORD)referenceSpace>0x400000 || (DWORD)referenceSpace+sizeof(referenceSpace)<0x690000 ||
       !VirtualProtect(mapped,0x290000,PAGE_EXECUTE_READWRITE,&old)) { printf("Bad reference reservation %p\n",referenceSpace);return 0; }
    FILE* f=fopen(path,"rb");if(!f)return 0;fseek(f,0,SEEK_END);long size=ftell(f);rewind(f);
    BYTE* raw=(BYTE*)malloc(size);if(!raw || fread(raw,1,size,f)!=(size_t)size)return 0;fclose(f);
    IMAGE_DOS_HEADER* dos=(IMAGE_DOS_HEADER*)raw;IMAGE_NT_HEADERS* pe=(IMAGE_NT_HEADERS*)(raw+dos->e_lfanew);
    if(pe->OptionalHeader.ImageBase!=0x400000 || pe->OptionalHeader.SizeOfImage>0x290000)return 0;
    memcpy(mapped,raw,pe->OptionalHeader.SizeOfHeaders);IMAGE_SECTION_HEADER* section=IMAGE_FIRST_SECTION(pe);
    for(int i=0;i<pe->FileHeader.NumberOfSections;++i) {
        if(section[i].PointerToRawData+section[i].SizeOfRawData>(DWORD)size ||
           section[i].VirtualAddress+section[i].SizeOfRawData>pe->OptionalHeader.SizeOfImage)return 0;
        memcpy(mapped+section[i].VirtualAddress,raw+section[i].PointerToRawData,section[i].SizeOfRawData);
    }free(raw);
    f=fopen(patches,"rb");if(!f)return 0;DWORD patch[2];int count=0;
    while(fread(patch,sizeof(patch),1,f)==1) {
        DWORD target=Replacement(patch[1]);
        if(!target || patch[0]<0x42ee30 || patch[0]+5>0x42f01c || *(BYTE*)patch[0]!=0xe8 ||
           patch[0]+5+*(DWORD*)(patch[0]+1)!=patch[1])return 0;
        *(DWORD*)(patch[0]+1)=target-patch[0]-5;++count;
    }fclose(f);if(count!=5)return 0;
    return FlushInstructionCache(GetCurrentProcess(),0,0)!=0;
}
static const unsigned int fovs[]={0,0x80000000,0x41200000,0x429a0000,0x42da0000,0x43340000,
    0xc29a0000,0x3dcccccd,0x43b40000,0x4b800001,0x00800000,0x00000001,0x7f800000,0xff800000,0x7fc12345,0x7f812345};
static void Run(int retail,int width,int height){
    memset(&observed,0,sizeof(observed));memset(cameraStorage,0x53,sizeof(cameraStorage));callCount=0;
    unsigned int random=seed+1;
    for(int i=0;i<9;++i){random=random*1664525u+1013904223u;Float(0x170+4*i)=((int)(random%2049)-1024)*0.125f;}
    Float(0x194)=(seed-32)*0.0625f;memcpy(&Float(0x16c),fovs+seed%16,4);
    Float(0x1bc)=(seed%5)*0.25f;Float(0x1c0)=seed%7?10000.0f:Float(0x1bc);
    Integer(0x1a8)=width;Integer(0x1ac)=height;
    union { void(Camera::*method)();DWORD address; } call;call.address=retail?0x42ee30:candidateAddress;
    _fpreset();try{(Object()->*call.method)();}catch(int code){observed.exception=code;}
    observed.fpStatus=_statusfp();memcpy(observed.camera,cameraStorage,sizeof(cameraStorage));
}
static int checks;
static void Compare(int width,int height){
    Run(1,width,height);expected=observed;Run(0,width,height);++checks;
    if(memcmp(&expected,&observed,sizeof(observed))){
        int at=0;while(at<sizeof(observed)&&((BYTE*)&observed)[at]==((BYTE*)&expected)[at])++at;
        printf("Mismatch case%d seed%d mutation%d throw%d size%d/%d byte%d fp%x/%x\n",checks,seed,mutation,throwAt,width,height,at,expected.fpStatus,observed.fpStatus);ExitProcess(2);
    }
#ifndef CAMERA_REAL_HELPERS
    if(observed.count!=(throwAt?throwAt:5) || observed.exception!=(throwAt?70+throwAt:0))Fail(83);
    const int kinds[]={1,2,3,4,4};for(int i=0;i<observed.count;++i)if(observed.calls[i].kind!=kinds[i])Fail(84);
#endif
    for(int j=0;j<16;++j)if(cameraStorage[j]!=0x53 || cameraStorage[j+0x230]!=0x53)Fail(85);
}
int main(int argc,char**argv){
    if(argc!=4 || !MapReference(argv[1],argv[2]))return 3;CameraLayout::Verify();
    candidateAddress=strtoul(argv[3],0,16);
#ifdef CAMERA_REAL_HELPERS
    InitFastMath();
#endif
    const int widths[]={0,1,2,320,640,800,1024,16777215,16777217,0x7fffffff,(int)0x80000000,-1};
    for(seed=0;seed<64;++seed)for(int w=0;w<12;++w)for(int h=0;h<12;++h)
#ifndef CAMERA_REAL_HELPERS
    for(mutation=0;mutation<8;++mutation)
#endif
    { Compare(widths[w],widths[h]); }
#ifndef CAMERA_REAL_HELPERS
    for(seed=0;seed<64;++seed)for(mutation=0;mutation<8;++mutation)for(throwAt=1;throwAt<=5;++throwAt)Compare(640,480);
    printf("CameraUpdate: %d scripted state, argument, exception and floating-point scenarios match\n",checks);
#else
    printf("CameraUpdate: %d scenarios match with actual matrix providers\n",checks);
#endif
    return 0;
}
