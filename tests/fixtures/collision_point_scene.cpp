// Actual CollisionPoint producer/consumer and source scene-cache helpers.
// Only allocation and dependencies outside the exercised methods are scripted.
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "collision/CollisionPoint.h"
#include "math/Vec3.h"
#include "../../src/reconstructed/Matrix4.h"
#include "core/SoultreeObject.h"

float Vec3DotCall(const Vec3*, const Vec3*);
void UpdateCollisionPointWorldPositions(int, CollisionPoint**);
typedef char VecLayout[sizeof(Vec3)==12 && sizeof(CollisionVec3)==12 && offsetof(Vec3,x)==0 && offsetof(Vec3,y)==4 && offsetof(Vec3,z)==8 && offsetof(CollisionVec3,x)==0 && offsetof(CollisionVec3,y)==4 && offsetof(CollisionVec3,z)==8 ? 1:-1];
typedef char NodeLayout[sizeof(SoultreeObject)==0x1a4 && offsetof(SoultreeObject,localMatrix)==0xb8 && offsetof(SoultreeObject,worldMatrix)==0xf8 && offsetof(SoultreeObject,worldValid)==0x138 && offsetof(SoultreeObject,parent)==0x13c ? 1:-1];
typedef char ContactLayout[sizeof(CollisionPoint)==0xc4 && offsetof(CollisionPoint,ownerNode)==4 && offsetof(CollisionPoint,localPosition)==8 && offsetof(CollisionPoint,worldPosition)==0x14 && offsetof(CollisionPoint,inContact)==0xa4 ? 1:-1];

CollisionVec3::CollisionVec3(float,float,float) { ExitProcess(90); }
CollisionVec3* CollisionDivide(CollisionVec3*,const CollisionVec3*,float) { ExitProcess(91); return 0; }
float CollisionLength(const CollisionVec3*) { ExitProcess(92); return 0; }
float FastSqrt(float) { ExitProcess(93); return 0; }
float FastInvSqrt(float) { ExitProcess(94); return 0; }
CollisionVec3* Vec3Normalize(CollisionVec3*,const CollisionVec3*) { ExitProcess(96); return 0; }

struct GuardedContact { DWORD before[2]; DWORD value[0xc4/4]; DWORD after[2]; };
struct GuardedNode { DWORD before[2]; DWORD value[0x1a4/4]; DWORD after[2]; };
static GuardedContact points[6], expectedPoints[6];
static GuardedNode nodes[3], expectedNodes[3];
static CollisionPoint* list[8];
static unsigned allocated, checks;
static CollisionPoint* Point(int n) { return (CollisionPoint*)points[n].value; }
static SoultreeObject* Node(int n) { return (SoultreeObject*)nodes[n].value; }
static SoultreeObject* ExpectedNode(int n) { return (SoultreeObject*)expectedNodes[n].value; }
void* operator new(unsigned int size, const char* file, int line) {
    if(size!=0xc4 || line!=27 || !strstr(file,"CollisionPoint.cpp") || allocated>=6) ExitProcess(80);
    return points[allocated++].value;
}
void operator delete(void*, const char*, int) { ExitProcess(81); }

static void Matrix(Matrix4& m,int seed) {
    memset(&m,0,sizeof(m));
    int axis=seed%3;
    for(int row=0;row<3;row++) m.m[row][(row+axis)%3]=(seed & (1<<row)) ? -1.0f:1.0f;
    m._41=(float)(seed%7-3);m._42=(float)(seed%11-5);m._43=(float)(seed%13-6);m._44=1;
}
// Independent integer-valued row/column oracle; all intermediate sums are
// exact for these fixtures. This does not claim arbitrary floating-point proof.
static void EnsureWorld(int index) {
    SoultreeObject* p=ExpectedNode(index);
    if(p->worldValid) return;
    if(p->parent) {
        int parent=p->parent==Node(0)?0:1;
        EnsureWorld(parent);
        Matrix4 product;
        for(int r=0;r<4;r++) for(int c=0;c<4;c++) {
            double sum=0;
            for(int k=0;k<4;k++) sum+=(double)p->localMatrix.m[r][k]*ExpectedNode(parent)->worldMatrix.m[k][c];
            product.m[r][c]=(float)sum;
        }
        p->worldMatrix=product;
    } else p->worldMatrix=p->localMatrix;
    p->worldValid=1;
}
static int FaultKind,FaultAddress;
static int Fault(EXCEPTION_POINTERS* p) {
    FaultKind=p->ExceptionRecord->ExceptionCode;
    FaultAddress=(int)p->ExceptionRecord->ExceptionInformation[1];
    return EXCEPTION_EXECUTE_HANDLER;
}
int main() {
    // Real CollisionPoint.cpp CRT initializers supply these shared objects.
    const CollisionVec3* zero=reinterpret_cast<const CollisionVec3*>(&kCollisionPointZero);
    const CollisionVec3* axis=reinterpret_cast<const CollisionVec3*>(&kCollisionPointYAxis);
    if(zero->x!=0 || zero->y!=0 || zero->z!=0 || axis->x!=0 || axis->y!=1 || axis->z!=0) return 78;
    for(int seed=0;seed<32;seed++) for(int hierarchy=0;hierarchy<4;hierarchy++)
    for(int states=0;states<8;states++) for(int count=0;count<=6;count++) for(int duplicate=0;duplicate<2;duplicate++) {
        memset(points,0xa5,sizeof(points));memset(nodes,0x5a,sizeof(nodes));
        memset(list,0,sizeof(list));allocated=0;
        for(int n=0;n<3;n++) {
            SoultreeObject* node=Node(n);Matrix(node->localMatrix,seed+n*5);
            Matrix(node->worldMatrix,seed+n*7+17);
            node->worldValid=(states>>n)&1;
            node->parent=n && hierarchy ? Node(hierarchy==1?0:n-1):0;
            if(hierarchy==3 && n==1) node->parent=0;
        }
        for(int p=0;p<6;p++) {
            CollisionVec3 local;local.x=(float)(p-3);local.y=(float)(seed%9-4);local.z=(float)(seed%5+p-2);
            int used=p;
            SoultreeObject* owner=((seed+p)%4)==3 ? 0:Node((seed+p)%3);
            CollisionPoint* result=AddCollisionPoint(6,list,&local,(CollisionContactOwner*)owner,
                .5f,&used,.6f,0);
            if(result!=Point(p) || used!=p+1 || list[p]!=result || result->ownerNode!=(CollisionContactOwner*)owner || memcmp(&result->localPosition,&local,12)) return 1;
            result->inContact=(states+p)%3;
        }
        if(allocated!=6) return 2;
        if(duplicate) {list[2]=list[0];list[5]=list[1];}
        CollisionPoint* originalList[8];memcpy(originalList,list,sizeof(list));
        memcpy(expectedPoints,points,sizeof(points));memcpy(expectedNodes,nodes,sizeof(nodes));
        for(int i=0;i<count;i++) {
            int index=0;while(Point(index)!=list[i]) index++;
            CollisionPoint* point=(CollisionPoint*)expectedPoints[index].value;
            if(point->ownerNode && point->inContact) {
                int owner=0;while((void*)Node(owner)!=(void*)point->ownerNode) owner++;
                EnsureWorld(owner);Matrix4& matrix=ExpectedNode(owner)->worldMatrix;
                for(int c=0;c<3;c++) {
                    double sum=matrix.m[3][c];
                    for(int k=0;k<3;k++) sum+=(double)((float*)&point->localPosition)[k]*matrix.m[k][c];
                    ((float*)&point->worldPosition)[c]=(float)sum;
                }
            }
        }
        UpdateCollisionPointWorldPositions(count,list);
        if(memcmp(points,expectedPoints,sizeof(points)) || memcmp(nodes,expectedNodes,sizeof(nodes)) || memcmp(list,originalList,sizeof(list))) {
            printf("world mismatch %d %d %d %d %d\n",seed,hierarchy,states,count,duplicate);return 3;
        }
        checks++;
    }
    UpdateCollisionPointWorldPositions(0,0);UpdateCollisionPointWorldPositions(-1,0);
    CollisionPoint* nullPoint=0;FaultKind=FaultAddress=0;
    __try {UpdateCollisionPointWorldPositions(1,&nullPoint);} __except(Fault(GetExceptionInformation())) {}
    if(FaultKind!=EXCEPTION_ACCESS_VIOLATION || FaultAddress!=4) return 4;
    checks+=3;
    for(int a=-8;a<=8;a++) for(int b=-8;b<=8;b++) for(int c=-4;c<=4;c++) {
        CollisionVec3 x,y;x.x=(float)a;x.y=(float)b;x.z=(float)c;
        y.x=(float)c;y.y=(float)a;y.z=(float)b;
        DWORD before[6];memcpy(before,&x,12);memcpy(before+3,&y,12);
        float result=Vec3DotCall((const Vec3*)&x,(const Vec3*)&y);
        if(result!=(float)(a*c+b*a+c*b) || memcmp(before,&x,12) || memcmp(before+3,&y,12)) return 5;
        if(Vec3DotCall((const Vec3*)&x,(const Vec3*)&x)!=(float)(a*a+b*b+c*c)) return 6;
        checks+=2;
    }
    printf("Contact scene: %u native allocation, cache, transform and dot cases passed\n",checks);
    return 0;
}
