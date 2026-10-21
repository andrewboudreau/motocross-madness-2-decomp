#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "vehicle/KrustyBike.h"
#define CHECK(name,x) typedef char name[(x)?1:-1]
CHECK(primary_size,sizeof(KrustyBike)==0x1638);
CHECK(remote,offsetof(KrustyBike,isRemote)==0x735);
CHECK(states,offsetof(KrustyBike,states)==0x11c8);
CHECK(velocity,offsetof(KrustyBike,deltaBaseVelocity)==0x1358);
CHECK(position,offsetof(KrustyBike,deltaBasePosition)==0x1364);
CHECK(angular,offsetof(KrustyBike,deltaBaseAngularVelocity)==0x1370);
CHECK(time,offsetof(KrustyBike,deltaBaseTime)==0x1388);
CHECK(recorder,offsetof(KrustyBike,netRecorder)==0x13fc);
CHECK(packet,sizeof(KbNetDelta)==0x16 && offsetof(KbNetDelta,field_0x0e)==0xe && offsetof(KbNetDelta,field_0x12)==0x12);
CHECK(state,sizeof(KbNetState)==0x54 && offsetof(KbNetState,step)==2 && offsetof(KbNetState,field_0x50)==0x50);
union Storage { double align; unsigned char bytes[0x1638+32]; };
static Storage actual, expected;
static unsigned char output[0x60+32], wanted[0x60+32];
static int count;
static void Pattern(void* p,int size,int seed) {
    unsigned char* b=(unsigned char*)p; for(int i=0;i<size;++i)b[i]=(unsigned char)(seed+i*37);
}
static void Put(void* p,int offset,float v) { memcpy((char*)p+offset,&v,4); }
static float Get(const void* p,int offset) { float v;memcpy(&v,(const char*)p+offset,4);return v; }
static int Fail(const char* name,int seed) {printf("FAIL %s %d\n",name,seed);return 1;}
int main() {
    // Prepared raw receiver/state storage: no constructors, vptr calls or lifetime
    // methods. The three actual bodies only access the explicitly initialized data.
    KrustyBike* bike=(KrustyBike*)(actual.bytes+16);
    int seed,i;
    for(seed=0;seed<256;++seed) {
        for(int which=0;which<3;++which) {
            Pattern(&actual,sizeof(actual),seed);expected=actual;
            KbRecorder* ptr=which==0?0:(KbRecorder*)(output+which*4);
            memcpy(expected.bytes+16+0x13fc,&ptr,4);
            bike->Fn_00496E20(ptr);
            if(memcmp(&actual,&expected,sizeof(actual)))return Fail("recorder",seed);
            ++count;
        }
    }
    const float cells[]={0.0f,-0.0f,0.25f,-0.25f,0.99f,-0.99f,1.0f,-1.0f,1.75f,-1.75f,127.5f,-128.5f};
    for(int remote=0;remote<2;++remote)for(int a=0;a<12;++a)for(int b=0;b<12;++b)for(int axis=0;axis<2;++axis) {
        Pattern(&actual,sizeof(actual),a*13+b);
        KbCell first,second;Pattern(&first,sizeof(first),a);Pattern(&second,sizeof(second),b);
        first.field_0x8=first.field_0x10=2.5f;second.field_0x8=second.field_0x10=2.5f;
        if(axis==0){first.field_0x8=cells[a];second.field_0x8=cells[b];}
        else{first.field_0x10=cells[a];second.field_0x10=cells[b];}
        bike->isRemote=(char)remote;
        ((KbNetBike*)bike)->field_0x11c8=&first;((KbNetBike*)bike)->field_0x11cc=&second;
        expected=actual;KbCell firstBefore=first,secondBefore=second;
        int want=remote && cells[a]!=0 && cells[b]!=0 && (int)cells[a]!=(int)cells[b];
        if(bike->Fn_00495C00()!=want || memcmp(&actual,&expected,sizeof(actual)) || memcmp(&first,&firstBefore,sizeof(first)) || memcmp(&second,&secondBefore,sizeof(second)))return Fail("cells",a*12+b);
        ++count;
    }
    const int deltas[]={-128,-127,-65,-1,0,1,63,64,126,127};
    for(seed=0;seed<64;++seed)for(int step=0;step<256;++step)for(int mode=0;mode<3;++mode) {
        Pattern(&actual,sizeof(actual),seed);Pattern(output,sizeof(output),seed+11);
        // Covers independent output, the real first embedded received-state record,
        // and a second embedded record. Packet is unaligned like the packed input.
        unsigned char* state=mode?actual.bytes+16+0x11d8+(mode-1)*0x60:output+16;
        unsigned char packet[32];Pattern(packet,sizeof(packet),seed+step);
        KbNetDelta* d=(KbNetDelta*)(packet+1);
        d->step=(unsigned char)step;
        for(i=0;i<3;++i) {
            d->delta0[i]=(signed char)deltas[(seed+i)%10];
            d->delta1[i]=(signed char)deltas[(seed+3+i)%10];
            d->delta2[i]=(signed char)deltas[(seed+6+i)%10];
            d->delta3[i]=(signed char)deltas[(seed+8+i)%10];
        }
        d->field_0x0e=seed^0x7f032211;d->field_0x12=step^0x81002041;
        for(i=0;i<12;++i)Put(bike,0x1358+i*4,(float)(seed-32)*(i+1)*0.125f);
        bike->deltaBaseTime=seed*1000+17;
        expected=actual;memcpy(wanted,output,sizeof(output));unsigned char packetBefore[32];memcpy(packetBefore,packet,32);
        unsigned char* expectState=mode?expected.bytes+16+0x11d8+(mode-1)*0x60:wanted+16;
        unsigned char* expectBike=expected.bytes+16;
        for(i=0;i<12;++i) {
            int component=i%3,group=i/3;
            int delta=group==0?d->delta0[component]:group==1?d->delta1[component]:group==2?d->delta2[component]:d->delta3[component];
            float scale=group<2?0.234375f:0.049087387f;
            // The first three vectors are materialized as float temporaries in
            // production; the last three scalar products remain on x87 until add.
            double product=(double)delta*scale;
            if(group<3){volatile float rounded=(float)product;product=rounded;}
            float v=(float)((double)Get(bike,0x1358+i*4)+product);
            Put(expectBike,0x1358+i*4,v);
            int offset=group==0?0x3c:group==1?8:group==2?0x30:0x14;
            Put(expectState,offset+component*4,v);
        }
        unsigned short elapsed=(unsigned short)((step&1)?(step>>1)*8:step>>1);
        memcpy(expectState+2,&elapsed,2);
        int time=bike->deltaBaseTime+elapsed;
        memcpy(expectBike+0x1388,&time,4);memcpy(expectState+0x4c,&time,4);
        memcpy(expectState+0x50,&d->field_0x0e,4);memcpy(expectState+0x48,&d->field_0x12,4);
        bike->Fn_004933E0(d,(KbNetState*)state);
        if(memcmp(&actual,&expected,sizeof(actual)) || memcmp(output,wanted,sizeof(output)) || memcmp(packet,packetBefore,32)) {
            for(int q=0;q<(int)sizeof(actual);++q)if(actual.bytes[q]!=expected.bytes[q])printf("receiver %x %02x %02x\n",q-16,actual.bytes[q],expected.bytes[q]);
            return Fail("delta",seed*256+step);
        }
        ++count;
    }
    printf("Race state: %d native guarded provider cases passed\n",count);
    return 0;
}
