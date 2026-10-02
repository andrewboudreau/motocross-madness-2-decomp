// Runs only our models with inert fake resources; never original game code.
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>
#include "PilotModels.h"

struct View {
    void** cb8; int cbc; void* c3c; void* p30; int n540; void* in544[4];
    void* p44; void* p34; void** c14; int c10; void* c84; void* c88;
    View() : cb8(0),cbc(0),c3c(0),p30(0),n540(0),p44(0),p34(0),c14(0),c10(0),c84(0),c88(0) {
        for(int i=0;i<4;++i) in544[i]=0;
    }
    void** table_cb8(){return cb8;} int count_cbc(){return cbc;}
    void* member_c3c(){return c3c;} void* member_030(){return p30;}
    int count_540(){return n540;} void* inline_item_544(int i){return in544[i];}
    void* member_044(){return p44;} void* member_034(){return p34;}
    void clear_034(){p34=0;}
    void** table_c14(){return c14;} int count_c10(){return c10;}
    void* member_c84(){return c84;} void* member_c88(){return c88;}
};
struct Event {
    std::string name; void* pointer; int value;
    Event(const char* n,void* p,int v):name(n),pointer(p),value(v){}
};
struct Operations {
    View& v; std::vector<Event> events; int category,previous;
    bool throw_release,clear_prepare,shrink_count,change_table,mutate_destroy;
    void** replacement;
    Operations(View& x):v(x),category(7),previous(-1),throw_release(false),clear_prepare(false),
      shrink_count(false),change_table(false),mutate_destroy(false),replacement(0){}
    void log(const char* n,void* p=0,int value=0){events.push_back(Event(n,p,value));}
    int select_terrain(){log("select");previous=category;category=42;return previous;}
    void release_ecx_slot2(void* p){
        log("release_ecx",p);
        if(throw_release)throw std::runtime_error("stub");
        if(shrink_count)v.cbc=v.c10=v.n540=0;
        if(change_table)v.cb8=replacement;
    }
    void free_debug(void* p,int line){log("free_debug",p,line);}
    void prepare_044(void* p,int flags){log("prepare",p,flags);if(clear_prepare)v.p44=0;}
    void delete_slot0(void* p,int flags){log("delete_slot0",p,flags);}
    void release_stack_slot2(void* p){log("release_stack",p);}
    void tracked_delete(void* p){log("tracked_delete",p);if(shrink_count)v.c10=0;}
    void destroy_401020(void* p){log("destroy",p);if(mutate_destroy)v.c84=v.c88=0;}
    void restore(int value){log("restore",0,value);category=value;}
    void base_cleanup(){log("base",0,category);}
    int count(const char* name)const{int n=0;for(unsigned i=0;i<events.size();++i)if(events[i].name==name)++n;return n;}
};
static int objects[8];
static void* p(int n){return &objects[n];}
static void terrain(View& v,Operations& o){mcm2_pilots::TerrainNormalCleanup(v,o);}

int main(){
    int scenarios=0;
    {View v;Operations o(v);terrain(v,o);assert(o.events.size()==3&&o.events[0].name=="select"&&o.events[1].name=="restore"&&o.events[2].name=="base"&&o.category==7);++scenarios;}
    {View v;Operations o(v);void* list[]={p(0),p(1)};v.cb8=list;v.cbc=2;terrain(v,o);assert(o.count("release_ecx")==2&&o.events[3].pointer==list&&o.events[3].value==0x4b3);++scenarios;}
    {View v;Operations o(v);void* list[]={0};v.cb8=list;v.cbc=1;terrain(v,o);assert(o.count("release_ecx")==1&&o.events[1].pointer==0);++scenarios;}
    {View v;Operations o(v);void* list[]={p(0)};v.cb8=list;v.cbc=-1;terrain(v,o);assert(o.count("release_ecx")==0&&o.count("free_debug")==1);++scenarios;}
    {View v;Operations o(v);v.c3c=p(0);v.p30=p(1);terrain(v,o);assert(o.events[1].pointer==p(0)&&o.events[2].pointer==p(1));++scenarios;}
    {View v;Operations o(v);v.n540=3;v.in544[0]=p(0);v.in544[2]=p(1);terrain(v,o);assert(o.count("release_ecx")==2);++scenarios;}
    {View v;Operations o(v);v.p44=p(0);terrain(v,o);assert(o.events[1].name=="prepare"&&o.events[2].name=="delete_slot0"&&o.events[2].value==1);++scenarios;}
    {View v;Operations o(v);v.p44=p(0);o.clear_prepare=true;terrain(v,o);assert(o.count("prepare")==1&&o.count("delete_slot0")==0);++scenarios;}
    {View v;Operations o(v);v.p34=p(0);terrain(v,o);assert(o.count("release_stack")==1&&v.p34==0);++scenarios;}
    {View v;Operations o(v);void* list[]={p(0),0,p(1)};v.c14=list;v.c10=3;terrain(v,o);assert(o.count("tracked_delete")==2&&o.events[3].value==0x4d0);++scenarios;}
    {View v;Operations o(v);v.c84=p(0);v.c88=p(1);terrain(v,o);assert(o.events[1].name=="destroy"&&o.events[2].name=="tracked_delete"&&o.events[3].pointer==p(1)&&o.events[4].pointer==p(1));++scenarios;}
    {View v;Operations o(v);v.c84=p(0);o.mutate_destroy=true;terrain(v,o);assert(v.c84==0&&o.events[2].pointer==p(0));++scenarios;}
    {View v;Operations o(v);void* list[]={p(0),p(1)};v.cb8=list;v.cbc=2;o.shrink_count=true;terrain(v,o);assert(o.count("release_ecx")==1);++scenarios;}
    {View v;Operations o(v);void* list[]={p(0),p(1)};void* list2[]={p(2),p(3)};v.cb8=list;v.cbc=2;o.change_table=true;o.replacement=list2;terrain(v,o);assert(o.events[2].pointer==p(3)&&o.events[3].pointer==list2);++scenarios;}
    {View v;Operations o(v);v.p30=p(0);o.throw_release=true;try{terrain(v,o);assert(false);}catch(const std::runtime_error&){}assert(o.count("restore")==0&&o.count("base")==0);++scenarios;} // Native model only; does NOT emulate the retail EH handler.
    std::printf("%d native pilot model scenarios passed; original code not executed\n",scenarios);
}
