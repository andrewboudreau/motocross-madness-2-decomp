// Runs our shared C++98 algorithm, not original x86 code or its real callees.
#include "EcoSystemPass.h"
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
using namespace mcm2_eco;

struct View {
    bool is_ready; int scratch_value; void* scratch_pointer; unsigned char tag;
    FramePrefix f; FramePrefix* frame_pointer;
    Word na,nb,ca,cb; float inverse,extent; RecordPrefix** a; RecordPrefix** b;
    RecordPrefix* storage_a[512]; RecordPrefix* storage_b[512];
    DefinitionPrefix definitions[2];
    View():is_ready(true),scratch_value(1),scratch_pointer(0),tag(7),frame_pointer(&f),
      na(9),nb(9),ca(500),cb(500),inverse(0),extent(0),a(storage_a),b(storage_b){
        f.scale_1c0=65535.0f; definitions[0].limit_1d8=1;definitions[1].limit_1d8=2;
        for(int i=0;i<512;++i)storage_a[i]=storage_b[i]=0;
    }
    bool ready(){return is_ready;} void* scratch(){return scratch_pointer;}
    FramePrefix* frame(){return frame_pointer;} unsigned char group(){return tag;}
    float& inverse_scale(){return inverse;} float& scaled_extent(){return extent;}
    Word& count_a(){return na;} Word& count_b(){return nb;}
    Word& capacity_a(){return ca;} Word& capacity_b(){return cb;}
    RecordPrefix**& list_a(){return a;} RecordPrefix**& list_b(){return b;}
    DefinitionPrefix* definition(unsigned char id){assert(id<2);return &definitions[id];}
};
struct Event { std::string name; Word value; int line; Event(const char* n,Word v=0,int l=0):name(n),value(v),line(l){} };
struct Ops {
    typedef long double Real;
    View& v; std::vector<RecordPrefix> records; std::vector<Event> events;
    unsigned index; int category; Word time,delta; bool global_enabled,visible;
    int eval_mode,eval_state,applied_state,applied_flags; int totals_calls,trim_calls,advance_calls;
    int old_bytes; Word total,trim_arg; bool replace_scratch,clear_ready_on_sample;
    bool move_a,move_b,change_definition_on_apply,change_group_after_first,throw_reallocate;
    int after_eval_id; float a_metric,b_metric,scale; Position point; float bound;
    int apply_calls,geometry_calls; DefinitionPrefix* metric_definition;
    RecordPrefix* moved_a[512];RecordPrefix* moved_b[512];void* last_advanced;
    Ops(View& view):v(view),index(0),category(4),time(0xfffffff0u),delta(21),global_enabled(true),visible(true),
      eval_mode(0),eval_state(1),applied_state(1),applied_flags(0),totals_calls(0),trim_calls(0),advance_calls(0),
      old_bytes(0),total(600000),trim_arg(0),replace_scratch(false),clear_ready_on_sample(false),
      move_a(false),move_b(false),change_definition_on_apply(false),change_group_after_first(false),throw_reallocate(false),
      after_eval_id(-1),a_metric(3),b_metric(10),scale(2),bound(0),apply_calls(0),geometry_calls(0),metric_definition(0),last_advanced(0){
      point.x=point.y=point.z=0; for(int i=0;i<512;++i)moved_a[i]=moved_b[i]=0;
    }
    void add(unsigned char tag=7){RecordPrefix r=RecordPrefix();r.group_008=tag;r.coordinate_00c=2;r.coordinate_00e=3;r.coordinate_010=4;records.push_back(r);}
    void event(const char* n,Word value=0,int line=0){events.push_back(Event(n,value,line));}
    int count(const char* n){int result=0;for(unsigned i=0;i<events.size();++i)if(events[i].name==n)++result;return result;}
    bool enabled(){event("enabled");return global_enabled;}
    int select(){event("select");int old=category;category=43;return old;}
    void restore(int old){event("restore");category=old;}
    Word sample(){event("sample");if(clear_ready_on_sample)v.is_ready=false;Word old=time;time+=delta;return old;}
    Word totals(void*,int* n){event("totals");++totals_calls;*n=old_bytes;if(replace_scratch)v.scratch_pointer=&index;return total;}
    void trim(void*,Word n){event("trim");++trim_calls;trim_arg=n;}
    void advance(void* p){event("advance");++advance_calls;last_advanced=p;}
    void publish(FramePrefix* f){event("publish");assert(f==v.frame_pointer);}
    float constant_65535(){return 65535.0f;} float constant_inverse(){return 1.5259021893143654e-05f;}
    double constant_half(){return 0.5;}
    void reset_iterator(){event("reset");index=0;}
    RecordPrefix* next(){event("next");if(change_group_after_first&&index==1)v.tag=8;return index<records.size()?&records[index++]:0;}
    void evaluate(RecordPrefix* r,int* mode,int* state){event("evaluate");*mode=eval_mode;*state=eval_state;if(after_eval_id>=0)r->definition_012=static_cast<unsigned char>(after_eval_id);}
    Real metric15(DefinitionPrefix* d,unsigned char){event("metric15");metric_definition=d;return a_metric;}
    Real metric14(DefinitionPrefix* d,unsigned char){event("metric14");assert(d==metric_definition);return b_metric;}
    float coordinate_scale(){return scale;}
    int test_geometry(FramePrefix* f,Position* p,float b,int flags){event("geometry");++geometry_calls;assert(f==v.frame_pointer&&flags==0);point=*p;bound=b;return visible;}
    void apply(RecordPrefix* r,int m,int s){event("apply");++apply_calls;assert(m==eval_mode&&s==eval_state);r->state_013=static_cast<unsigned char>(applied_state);r->flags_016=static_cast<unsigned char>(applied_flags);if(change_definition_on_apply)r->definition_012=1;}
    RecordPrefix** reallocate(RecordPrefix** p,Word bytes,int line){event("reallocate",bytes,line);if(throw_reallocate)throw std::runtime_error("probe boundary");if(line==0x8dd)return move_a?moved_a:p;assert(line==0x8ea);return move_b?moved_b:p;}
    void elapsed(Word value){event("elapsed",value);}
};
static int run(View& v,Ops& o){return mcm2_eco::Run(v,o);}

int main(){
    int scenarios=0;
    assert(sizeof(RecordPrefix)==0x18&&offsetof(RecordPrefix,group_008)==8&&offsetof(RecordPrefix,coordinate_00c)==12&&offsetof(RecordPrefix,state_013)==19&&offsetof(RecordPrefix,flags_016)==22);
    assert(offsetof(DefinitionPrefix,limit_1d8)==0x1d8&&offsetof(FramePrefix,scale_1c0)==0x1c0);
    {View v;Ops o(v);o.global_enabled=false;assert(run(v,o)==1&&o.count("select")==0&&o.count("sample")==0&&v.na==9);++scenarios;}
    {View v;Ops o(v);v.is_ready=false;assert(run(v,o)==1&&o.category==43&&o.count("sample")==1&&o.count("restore")==0&&v.na==9);++scenarios;}
    {View v;Ops o(v);o.clear_ready_on_sample=true;run(v,o);assert(o.count("reset")==0&&o.count("restore")==0);++scenarios;}
    {View v;Ops o(v);assert(run(v,o)==1&&v.na==0&&v.nb==0&&v.inverse==1.0f&&o.count("sample")==2&&o.category==4);assert(o.events[o.events.size()-2].value==21);++scenarios;}
    {View v;Ops o(v);v.scratch_pointer=&v.scratch_value;o.old_bytes=0x40000;run(v,o);assert(o.totals_calls==1&&o.trim_calls==0&&o.advance_calls==1);++scenarios;}
    {View v;Ops o(v);v.scratch_pointer=&v.scratch_value;o.old_bytes=0x40001;o.replace_scratch=true;run(v,o);assert(o.trim_arg==600000u-0x40001u&&o.last_advanced==&o.index);++scenarios;}
    {View v;Ops o(v);v.scratch_pointer=&v.scratch_value;o.old_bytes=-1;run(v,o);assert(o.trim_calls==0);++scenarios;}
    {View v;Ops o(v);o.add(8);run(v,o);assert(o.count("evaluate")==0&&o.apply_calls==0&&v.na==0);++scenarios;}
    {View v;Ops o(v);o.add();o.add(8);o.change_group_after_first=true;run(v,o);assert(o.apply_calls==2&&v.na==2);++scenarios;}
    {View v;Ops o(v);o.add();o.eval_mode=1;run(v,o);assert(o.geometry_calls==0&&o.count("metric15")==0&&o.apply_calls==1);++scenarios;}
    {View v;Ops o(v);o.add();o.visible=false;run(v,o);assert(o.geometry_calls==1&&o.apply_calls==0&&v.na==0&&v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();run(v,o);assert(o.point.x==4&&o.point.y==11&&o.point.z==8&&o.bound==5&&v.na==1&&v.nb==1);++scenarios;}
    {View v;Ops o(v);o.add();o.a_metric=9;run(v,o);assert(o.bound==9&&o.point.y==11);++scenarios;}
    {View v;Ops o(v);o.add();o.a_metric=5;run(v,o);assert(o.bound==5);++scenarios;}
    {View v;Ops o(v);o.add();o.a_metric=std::numeric_limits<float>::quiet_NaN();run(v,o);assert(o.bound==5);++scenarios;}
    {View v;Ops o(v);o.add();o.b_metric=std::numeric_limits<float>::quiet_NaN();run(v,o);assert(o.bound!=o.bound&&o.point.y!=o.point.y);++scenarios;}
    {View v;Ops o(v);o.add();o.records[0].coordinate_00c=65535;run(v,o);assert(o.point.x==131070);++scenarios;}
    {View v;Ops o(v);o.add();o.applied_state=0;o.applied_flags=1;run(v,o);assert(v.na==0&&v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();o.applied_state=255;run(v,o);assert(v.na==1);++scenarios;}
    {View v;Ops o(v);o.add();v.definitions[0].limit_1d8=-1;run(v,o);assert(v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();o.applied_flags=3;run(v,o);assert(v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();o.after_eval_id=1;o.applied_flags=1;run(v,o);assert(o.metric_definition==&v.definitions[1]&&v.nb==1);++scenarios;}
    {View v;Ops o(v);o.add();o.change_definition_on_apply=true;o.applied_flags=1;run(v,o);assert(v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();v.ca=0;o.move_a=true;run(v,o);assert(v.ca==100&&v.a==o.moved_a&&v.a[0]==&o.records[0]);assert(o.count("reallocate")==1);++scenarios;}
    {View v;Ops o(v);o.add();o.add();v.ca=0;run(v,o);assert(v.ca==0&&v.na==2&&o.count("reallocate")==1);++scenarios;} // equality, not >=
    {View v;Ops o(v);o.add();v.cb=0;o.move_b=true;run(v,o);assert(v.cb==20&&v.b==o.moved_b);++scenarios;}
    {View v;Ops o(v);o.add();v.cb=0;run(v,o);assert(v.cb==0&&v.nb==1);++scenarios;}
    {View v;Ops o(v);o.add();v.ca=v.cb=0;o.move_a=o.move_b=true;run(v,o);std::vector<Event> a;for(unsigned i=0;i<o.events.size();++i)if(o.events[i].name=="reallocate")a.push_back(o.events[i]);assert(a.size()==2&&a[0].value==400&&a[0].line==0x8dd&&a[1].value==80&&a[1].line==0x8ea);++scenarios;}
    {View v;Ops o(v);o.add();v.ca=0;o.throw_reallocate=true;try{run(v,o);assert(false);}catch(const std::runtime_error&){}assert(o.count("restore")==0);++scenarios;} // host model exception, not retail EH equivalence
    {View v;Ops o(v);v.f.scale_1c0=2;run(v,o);assert(v.inverse==32767.5f&&v.extent==2*o.constant_inverse());++scenarios;}
    {View v;Ops o(v);o.add();o.add();run(v,o);assert(o.count("reset")==1&&o.count("next")==3&&v.a[0]==&o.records[0]&&v.a[1]==&o.records[1]);++scenarios;}
    {View v;Ops o(v);o.add();o.applied_flags=1;o.applied_state=1;run(v,o);assert(v.na==1&&v.nb==0);++scenarios;}
    {View v;Ops o(v);o.add();o.applied_flags=0;o.applied_state=0;run(v,o);assert(v.na==0&&v.nb==1);++scenarios;}
    std::printf("%d EcoSystem algorithm scenarios passed; target code was not executed\n",scenarios);
}
