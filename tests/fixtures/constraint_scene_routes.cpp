// Execute the actual scene methods and cache helpers, using prepared object
// storage rather than claiming constructor or full solver lifecycle coverage.
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "math/Math3D.h"
#include "constraint/ConstraintMethodCollisionModel.h"

typedef char VecLayout[(sizeof(ConVec3)==12 && sizeof(Vec3)==12 && offsetof(ConVec3,z)==8)?1:-1];
typedef char BodyNode[(offsetof(ConBody,node)==0x234)?1:-1];
typedef char ProbeNode[(sizeof(ConstraintProbe)==32 && offsetof(ConstraintProbe,node)==12)?1:-1];
typedef char NodeFields[(offsetof(SoultreeObject,localMatrix)==0xb8 && offsetof(SoultreeObject,worldValid)==0x138)?1:-1];
union Storage { double align; unsigned char bytes[6*sizeof(SoultreeObject)+32]; } actual, expected;
union Output { double align; unsigned char bytes[64]; } output, wantedOutput;
static SoultreeObject* n=(SoultreeObject*)(actual.bytes+16);
static SoultreeObject* e=(SoultreeObject*)(expected.bytes+16);
static int cases;
static void Require(int ok,int line) { if(!ok){printf("constraint scene failure %d case %d\n",line,cases);exit(1);} }
#define CHECK(x) Require((x),__LINE__)
static int Index(SoultreeObject* p) { return (int)(p-n); }
static void Matrix(Matrix4& m,int seed,int index) {
    memset(&m,0,64);
    for(int row=0;row<3;row++)m.m[row][(row+(seed+index)%3)%3]=((seed+row+index)&1)?-1.0f:1.0f;
    m._44=1;m._41=(float)(seed-index*2);m._42=(float)(index*3-7);m._43=(float)(index-seed);
}
static void Setup(int seed,int parent,int valid) {
    memset(&actual,seed+57,sizeof(actual));
    for(int i=0;i<6;i++) {
        n[i].parent=0;n[i].firstChild=0;n[i].nextSibling=0;
        n[i].worldValid=(valid>>i)&1;n[i].subtreeDirty=(seed+i)&1;
        Matrix(n[i].localMatrix,seed,i);Matrix(n[i].worldMatrix,seed+5,i+3);
    }
    if(parent)n[1].parent=&n[0];
    n[1].firstChild=&n[2];n[1].nextSibling=&n[5];
    n[2].parent=&n[1];n[2].nextSibling=&n[3];n[2].firstChild=&n[4];
    n[3].parent=&n[1];n[4].parent=&n[2];
    memcpy(&expected,&actual,sizeof(actual));memset(&output,0x97,sizeof(output));wantedOutput=output;
}
static void World(int i) {
    if(e[i].worldValid)return;
    if(e[i].parent) {
        int parent=Index(e[i].parent);World(parent);Matrix4 value;
        for(int row=0;row<4;row++)for(int col=0;col<4;col++) {
            double sum=0;for(int k=0;k<4;k++)sum+=(double)e[i].localMatrix.m[row][k]*e[parent].worldMatrix.m[k][col];
            value.m[row][col]=(float)sum;
        }
        e[i].worldMatrix=value;
    } else e[i].worldMatrix=e[i].localMatrix;
    e[i].worldValid=1;
}
static void InvalidateDescendants(int i) {
    if(e[i].nextSibling)InvalidateDescendants(Index(e[i].nextSibling));
    if(!e[i].worldValid)return;
    e[i].worldValid=0;
    if(e[i].firstChild)InvalidateDescendants(Index(e[i].firstChild));
}
static void Invalidated() {
    e[1].subtreeDirty=1;
    if(e[1].parent)e[0].subtreeDirty=1;
    if(e[1].worldValid) {
        e[1].worldValid=0;
        if(e[1].firstChild)InvalidateDescendants(Index(e[1].firstChild));
    }
}
static void Verify() {
    CHECK(memcmp(&actual,&expected,sizeof(actual))==0);
    CHECK(memcmp(&output,&wantedOutput,sizeof(output))==0);
}
int main() {
    for(int seed=0;seed<32;seed++)for(int parent=0;parent<2;parent++)for(int valid=0;valid<64;valid++) {
        int alias;
        // Actual explicit-output getter with an independent result or local-row alias.
        for(alias=0;alias<2;alias++) {
            Setup(seed,parent,valid);World(1);
            ConVec3* result=alias?(ConVec3*)&n[1].localMatrix._41:(ConVec3*)(output.bytes+16);
            void* wanted=alias?(void*)&e[1].localMatrix._41:(void*)(wantedOutput.bytes+16);
            memcpy(wanted,&e[1].worldMatrix._41,12);
            reinterpret_cast<SoultreeObject*>((ConNode*)&n[1])->GetPositionIn(0,reinterpret_cast<Vec3*>(result));
            Verify();cases++;
        }
        for(alias=0;alias<3;alias++) {
            Setup(seed,parent,valid);
            Vec3 input((float)(seed-8),3.0f,(float)(9-seed));
            const ConVec3* point=alias?(const ConVec3*)(alias==1?&n[1].localMatrix._41:&n[1].worldMatrix._41):(const ConVec3*)&input;
            double p[3]={point->x,point->y,point->z},local[3];
            if(parent) {
                World(0);
                for(int row=0;row<3;row++) {
                    local[row]=0;
                    for(int k=0;k<3;k++)local[row]+=(p[k]-e[0].worldMatrix.m[3][k])*e[0].worldMatrix.m[row][k];
                }
            } else for(int k=0;k<3;k++)local[k]=p[k];
            for(int k=0;k<3;k++)e[1].localMatrix.m[3][k]=(float)local[k];
            Invalidated();
            reinterpret_cast<SoultreeObject*>((ConNode*)&n[1])->SetPositionInFrame(0,reinterpret_cast<const Vec3*>(point));
            Verify();
            // Follow the setter with the same null-frame getter the solver uses.
            World(1);memcpy(wantedOutput.bytes+16,&e[1].worldMatrix._41,12);
            n[1].GetPositionIn(0,(Vec3*)(output.bytes+16));Verify();cases++;
        }
        for(alias=0;alias<3;alias++) {
            Setup(seed,parent,valid);
            Vec3 input((float)(seed-8),3.0f,(float)(9-seed));
            const ConVec3* point=alias?(const ConVec3*)(alias==1?&n[1].localMatrix._41:&n[1].worldMatrix._41):(const ConVec3*)&input;
            World(1);
            const float* p=alias?(alias==1?&e[1].localMatrix._41:&e[1].worldMatrix._41):&input.x;
            float result[3];
            for(int col=0;col<3;col++) {
                double value=e[1].worldMatrix.m[3][col];
                for(int k=0;k<3;k++)value+=(double)p[k]*e[1].worldMatrix.m[k][col];
                result[col]=(float)value;
            }
            memcpy(wantedOutput.bytes+16,result,12);
            *(ConVec3*)(output.bytes+16)=reinterpret_cast<const ConVec3&>(static_cast<const Vec3&>(
                reinterpret_cast<SoultreeObject*>((ConNode*)&n[1])->LocalToWorldPoint(reinterpret_cast<const Vec3&>(*point))));
            Verify();cases++;
        }
    }
    printf("Constraint scene routes: %d actual getter, placement, point, cache and alias cases passed\n",cases);
    return 0;
}
