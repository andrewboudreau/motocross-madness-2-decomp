// Execute the real SnapPlacement/UpdatePlacement pair with explicit sync/tree boundaries.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "collision/CollisionObject.h"
#include "../../src/reconstructed/Wrecker.h"

QuadTree* g_collisionQuadTree;
struct Storage { unsigned char before[16]; unsigned char object[0x10c]; unsigned char after[16]; };
struct Checkpoint { Storage state; unsigned int tree; int kind; };
static Storage actual, expected;
static Checkpoint checkpoints[4];
static unsigned int expectedTree;
static int seed, stopAt, count, seen, syncCount, treeCount;

static unsigned int Read(const Storage& s, int offset) {
    unsigned int v; memcpy(&v,s.object+offset,4); return v;
}
static void Write(Storage& s, int offset, unsigned int v) { memcpy(s.object+offset,&v,4); }
static void Fail(const char* reason) { printf("seed=%d stop=%d event=%d: %s\n",seed,stopAt,seen,reason); exit(1); }

// The oracle plans both passes before the actual methods execute. Its state
// updates use byte offsets rather than the production members or call graph.
static void PredictMutation(int kind,int ordinal) {
    if (kind==1) {
        unsigned int control=(seed>>(ordinal==1?2:5))&7;
        if (control&1) Write(expected,0x80,(control&2)?0xffffffffU:0);
        if (control&4) expectedTree=(control&2)?0x11110000U:0;
        Write(expected,0xa4,Read(expected,0xa4)^(0x13579bdfU+ordinal));
    } else {
        unsigned int control=(seed>>8)&7;
        if (control&1) Write(expected,0x80,(control&2)?7:0);
        if (control&4) expectedTree=(control&2)?0:0x22220000U;
        Write(expected,0x70,Read(expected,0x70)+0x1020304U+ordinal);
    }
}
static bool PlanBoundary(int kind,int ordinal) {
    checkpoints[count].state=expected;checkpoints[count].tree=expectedTree;
    checkpoints[count].kind=kind;++count;
    PredictMutation(kind,ordinal);
    return count==stopAt;
}
static void Plan() {
    int trees=0;count=0;
    for(int pass=1;pass<=2;++pass) {
        if(PlanBoundary(1,pass))return;
        if(expectedTree && Read(expected,0x80))
            if(PlanBoundary(2,++trees))return;
    }
}
static void Observe(CollisionObject* receiver,int kind) {
    if(receiver!=(CollisionObject*)actual.object || seen>=count)Fail("receiver or event count");
    const Checkpoint& p=checkpoints[seen];
    if(p.kind!=kind || p.tree!=(unsigned int)g_collisionQuadTree || memcmp(&actual,&p.state,sizeof actual))Fail("boundary state/order");
    ++seen;
    if(kind==1) {
        ++syncCount;unsigned int control=(seed>>(syncCount==1?2:5))&7;
        if(control&1)receiver->useBroadphase=(control&2)?-1:0;
        if(control&4)g_collisionQuadTree=(QuadTree*)((control&2)?0x11110000U:0);
        unsigned int v;memcpy(&v,actual.object+0xa4,4);v^=0x13579bdfU+syncCount;memcpy(actual.object+0xa4,&v,4);
    } else {
        ++treeCount;unsigned int control=(seed>>8)&7;
        if(control&1)receiver->useBroadphase=(control&2)?7:0;
        if(control&4)g_collisionQuadTree=(QuadTree*)((control&2)?0:0x22220000U);
        unsigned int v;memcpy(&v,actual.object+0x70,4);v+=0x1020304U+treeCount;memcpy(actual.object+0x70,&v,4);
    }
    if(seen==stopAt)throw 0x3560;
}
void CollisionObject::SyncTransformFromNode() { Observe(this,1); }
void CollisionObject::UpdateQuadtreeCell() { Observe(this,2); }

int main() {
    if(sizeof(CollisionObject)!=0xb8 || sizeof(UnknownWreckerCollisionModel)!=0x10c)return 2;
    unsigned char* primary=actual.object;
    UnknownWreckerCollisionModel* model=(UnknownWreckerCollisionModel*)primary;
    if((unsigned char*)(CollisionObject*)model!=primary || (unsigned char*)(GameObject*)model!=primary+12)return 3;
    unsigned long cases=0;
    for(seed=0;seed<2048;++seed)for(stopAt=0;stopAt<=4;++stopAt) {
        unsigned char* bytes=(unsigned char*)&actual;
        for(unsigned int i=0;i<sizeof actual;++i)bytes[i]=(unsigned char)(i*37+seed*13);
        ((CollisionObject*)primary)->useBroadphase=(seed&1)?3:0;
        g_collisionQuadTree=(QuadTree*)((seed&2)?0x33330000U:0);
        expected=actual;expectedTree=(unsigned int)g_collisionQuadTree;Plan();
        seen=syncCount=treeCount=0;int caught=0;
        try { model->SnapPlacement(); } catch(int value) { caught=value; }
        if(caught!=((stopAt && stopAt<=count)?0x3560:0))Fail("exception propagation");
        if(seen!=count || expectedTree!=(unsigned int)g_collisionQuadTree || memcmp(&actual,&expected,sizeof actual))Fail("final state");
        ++cases;
    }
    printf("Bike snap placement: %lu two-pass, mutation and exception cases passed\n",cases);
    return 0;
}
