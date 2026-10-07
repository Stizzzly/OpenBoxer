#include "lighting.hpp"
#include "texture_abi.hpp"
#include "world_runtime.hpp"
#include "loader.hpp"
#include <windows.h>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdio>
namespace lighting {
namespace {
enum Script { none=0,textureColor=1,textureBlend=2,positions=4,materialPointer=8,materialIndex=16,diffuse=32,shrink=64,globalCount=128 };
struct Case { const char* name; bool b; int32_t meshes=3,triangles=3; unsigned pattern=0,script=0,passes=1; uint32_t texture=17; bool emptyGroups=false,negativeGroups=false,reorder=false,duplicate=false,overflowScalar=false; };
struct Model {
    std::vector<uint8_t> object,groups,triangles,vertices,materials,alternative;
    std::vector<std::vector<int32_t>> memberships;
    unsigned outputs=0;
    explicit Model(const Case& test):object(size,0x5b),groups(36,0xa7),triangles(228,0x6c),vertices(144,0x91),materials(160,0xb6),alternative(160,0xc7),memberships(3) {
        put(object.data()+meshCount,static_cast<uint32_t>(test.meshes)); setPointer(object.data()+meshTable,groups.data());
        put(object.data()+triangleCount,static_cast<uint32_t>(test.triangles)); setPointer(object.data()+triangleTable,triangles.data());
        put(object.data()+vertexCount,9); setPointer(object.data()+vertexTable,vertices.data());
        put(object.data()+materialCount,2); setPointer(object.data()+materialTable,materials.data());
        const uint32_t xyz[3][3]={{0x3f800000,0x40800000,0x40e00000},{0x40000000,0x40a00000,0x41000000},{0x40400000,0x40c00000,0x41100000}};
        for(unsigned i=0;i<9;i++) for(unsigned component=0;component<3;component++) put(vertices.data()+i*16+4+component*4,xyz[i%3][component]);
        if(test.pattern==1) {
            const uint32_t sensitive[3][3]={{0x4b800000,0x5d800000,0x60ad78ec},{0x3f800000,0x3f800000,0xe0ad78ec},{0xcb800000,0xdd800000,0x40400000}};
            for(unsigned i=0;i<9;i++) for(unsigned component=0;component<3;component++) put(vertices.data()+i*16+4+component*4,sensitive[i%3][component]);
        } else if(test.pattern==2) {
            for(unsigned i=0;i<9;i++) { put(vertices.data()+i*16+4,(i%3)==0?0x3f800000:(i%3)==1?0x33800000:0); put(vertices.data()+i*16+8,(i%3)==0?0x3f000001:(i%3)==1?0x3f000000:0xbf000000); }
        }
        for(unsigned i=0;i<3;i++) {
            for(unsigned j=0;j<9;j++) put(triangles.data()+i*76+j*4,j%2?0x80000000:0x3f000000+j);
            for(unsigned j=0;j<3;j++) { put(triangles.data()+i*76+36+j*4,j%2?0x80000000:0x3e800000+j); put(triangles.data()+i*76+48+j*4,j%2?0:0xbf000000+j); put(triangles.data()+i*76+60+j*4,i*3+j); }
            memberships[i]={static_cast<int32_t>(test.reorder?2-i:i)};
            if(test.emptyGroups || test.negativeGroups) memberships[i].clear();
            if(test.duplicate && i==0) memberships[i]={2,0,2};
            if(test.duplicate && i>0) memberships[i].clear();
            put(groups.data()+i*12,i%2); put(groups.data()+i*12+4,test.negativeGroups?0xffffffff:static_cast<uint32_t>(memberships[i].size())); setPointer(groups.data()+i*12+8,memberships[i].data());
            if(static_cast<int32_t>(i)<test.meshes) outputs+=static_cast<unsigned>(memberships[i].size());
        }
        for(unsigned i=0;i<2;i++) {
            for(unsigned j=0;j<3;j++) { put(materials.data()+i*80+16+j*4,0x3e800000+i*0x100000+j*0x1000); put(alternative.data()+i*80+16+j*4,0x3f000000+i*0x100000+j*0x1000); }
            put(materials.data()+i*80+48,test.overflowScalar?0x7f7fffff:0x3e000000); put(alternative.data()+i*80+48,0x3e000000);
        }
        if(test.b) {
            // Independently constructed flat inputs, not output from candidate A.
            for(unsigned i=0;i<3;i++) {
                auto* record=object.data()+4+i*116; put(record,i%2);
                for(unsigned vertex=0;vertex<3;vertex++) for(unsigned component=0;component<3;component++) put(record+4+vertex*32+component*4,bits(vertices.data()+(i*3+vertex)*16+4+component*4));
            }
        }
    }
    static void setPointer(void* target,void* value) { std::memcpy(target,&value,4); }
    std::vector<uint8_t> normalizedObject() const {
        auto result=object; for(const auto field:{meshTable,triangleTable,vertexTable}) put(result.data()+field,static_cast<uint32_t>(field));
        put(result.data()+materialTable,pointer(object.data()+materialTable)==materials.data()?1:2); return result;
    }
    std::vector<uint8_t> sources() const {
        auto normalizedGroups=groups; for(unsigned i=0;i<3;i++) put(normalizedGroups.data()+i*12+8,10+i);
        std::vector<uint8_t> result=normalizedGroups;
        for(const auto* data:{&triangles,&vertices,&materials,&alternative}) result.insert(result.end(),data->begin(),data->end());
        for(const auto& list:memberships) { const auto* begin=reinterpret_cast<const uint8_t*>(list.data()); if(!list.empty()) result.insert(result.end(),begin,begin+list.size()*4); }
        return result;
    }
};
struct Recorder {
    Case test; Model model;
    uint8_t* count; uint8_t* records; uint8_t* texture;
    unsigned colors=0;
    std::vector<std::string> events;
    bool valid=true;
    Recorder(const Case& c,uint8_t* n,uint8_t* r,uint8_t* t):test(c),model(c),count(n),records(r),texture(t) {}
    std::string snapshot() const {
        std::ostringstream out; out<<"count="<<std::hex<<bits(count)<<" texture="<<bits(texture)<<" triangles="<<bits(model.object.data()+triangleCount)<<" materialTable="<<(pointer(model.object.data()+materialTable)==model.materials.data()?"A":"B");
        for(unsigned i=0;i<3;i++) { out<<" G"<<i<<'='; for(unsigned j=0;j<14;j++) out<<bits(records+i*56+j*4)<<','; }
        uint16_t cw,sw; __asm__ volatile("fnstcw %0":"=m"(cw)); __asm__ volatile("fnstsw %0":"=m"(sw)); out<<" CW="<<cw<<" exceptions="<<(sw&0x3f); return out.str();
    }
    void record(const std::string& name,const std::string& args) { events.push_back(name+"("+args+") | "+snapshot()); }
};
thread_local Recorder* current=nullptr;
uint32_t __stdcall colorRecorder(uint32_t red,uint32_t green,uint32_t blue,uint32_t alpha) {
    auto& r=*current; r.record("glColor4f",std::to_string(red)+","+std::to_string(green)+","+std::to_string(blue)+","+std::to_string(alpha));
    const bool perRecord=alpha==0x3f333333;
    if(!perRecord && r.colors==0 && (r.test.script&textureColor)) put(r.texture,0xffffffff);
    if(perRecord) {
        const unsigned i=r.colors;
        if(r.test.script&positions) for(unsigned vertex=0;vertex<3;vertex++) for(unsigned component=0;component<3;component++) put(r.model.object.data()+4+i*116+4+vertex*32+component*4,0x40000000+component*0x100000);
        if(r.test.script&materialPointer) Model::setPointer(r.model.object.data()+materialTable,r.model.alternative.data());
        if(r.test.script&materialIndex) put(r.model.object.data()+4+i*116,1);
        if(r.test.script&diffuse) for(unsigned component=0;component<3;component++) put(r.model.materials.data()+(i%2)*80+16+component*4,0x3f800000);
        if(r.test.script&shrink) put(r.model.object.data()+triangleCount,1);
        if(r.test.script&globalCount) put(r.count,7);
        ++r.colors;
    }
    return 0x6a5b4c3d;
}
void __stdcall blendRecorder(uint32_t source,uint32_t destination) { auto& r=*current; r.valid=r.valid && source==0x302 && destination==1; r.record("glBlendFunc",std::to_string(source)+","+std::to_string(destination)); if(r.test.script&textureBlend) put(r.texture,0x12345678); }
void __stdcall bindRecorder(uint32_t target,uint32_t id) { auto& r=*current; r.valid=r.valid && target==0x0de1 && id==bits(r.texture); r.record("glBindTexture",std::to_string(target)+","+std::to_string(id)); }
void __stdcall disableRecorder(uint32_t cap) { current->valid=current->valid && cap==0x0b44; current->record("glDisable",std::to_string(cap)); }
void __stdcall enableRecorder(uint32_t cap) { current->valid=current->valid && cap==0x0b44; current->record("glEnable",std::to_string(cap)); }
GL gl() { return {reinterpret_cast<Color>(&colorRecorder),blendRecorder,bindRecorder,disableRecorder,enableRecorder}; }
uint32_t __attribute__((thiscall)) offlineA(void* model) { return stageA(model); }
uint32_t __attribute__((thiscall)) offlineB(void* model) { return stageB(model,current->count,current->records,current->texture,gl()); }
std::vector<Case> cases() {
    return {
        {"A_empty",false,0,0},{"A_negative",false,-1,0},{"A_positive_empty",false,3,3,0,0,1,17,true},
        {"A_negative_memberships",false,3,3,0,0,1,17,false,true},
        {"A_installed_schema",false},{"A_reordered",false,3,3,0,0,1,17,false,false,true},
        {"A_duplicates",false,3,3,0,0,1,17,false,false,false,true},
        {"A_cancellation",false,3,3,1},{"A_rounding",false,3,3,2},
        {"B_empty",true,0,0},{"B_negative",true,0,-1},{"B_one",true,3,1},
        {"B_three",true},{"B_zero_texture",true,3,3,0,0,1,0},{"B_large_texture",true,3,3,0,0,1,0xffffffff},
        {"B_repeated",true,3,3,0,0,2},{"B_cancellation",true,3,3,1},{"B_rounding",true,3,3,2},
        {"B_dead_scalar_overflow",true,3,3,0,0,1,17,false,false,false,false,true},
        {"B_color_texture",true,3,3,0,textureColor},{"B_blend_texture",true,3,3,0,textureBlend},
        {"B_positions",true,3,3,0,positions},{"B_material_table",true,3,3,0,materialPointer},
        {"B_material_index",true,3,3,0,materialIndex},{"B_diffuse",true,3,3,0,diffuse},
        {"B_shrink_count",true,3,3,0,shrink},{"B_global_count",true,3,3,0,globalCount},
        {"B_combined_live_reads",true,3,3,0,positions|materialPointer|materialIndex|shrink|globalCount|textureBlend}
    };
}
void resetGlobals(Recorder& r) { put(r.count,0xa5a5a5a5); put(r.texture,r.test.texture); std::memset(r.records,0xd3,3*56+8); }
struct Result { uint32_t value=0; WorldAbiReport registers{}; uint16_t beforeCW=0,afterCW=0,beforeSW=0,afterSW=0; uint32_t beforeMX=0,afterMX=0; bool valid=true; };
Result invoke(Entry entry,Recorder& recorder,void* subject=nullptr) {
    uint16_t savedCW; uint32_t savedMX; __asm__ volatile("fnstcw %0":"=m"(savedCW)); __asm__ volatile("stmxcsr %0":"=m"(savedMX));
    const uint16_t controlled=0x027f; const uint32_t mx=0x1f80;
    __asm__ volatile("fnclex; fldcw %0"::"m"(controlled)); __asm__ volatile("ldmxcsr %0"::"m"(mx));
    current=&recorder; Result result;
    for(unsigned pass=0;pass<recorder.test.passes;pass++) {
        __asm__ volatile("fnstcw %0":"=m"(result.beforeCW)); __asm__ volatile("fnstsw %0":"=m"(result.beforeSW)); __asm__ volatile("stmxcsr %0":"=m"(result.beforeMX));
        result.value=texture_abi_probe(entry,subject?subject:recorder.model.object.data(),&result.registers);
        __asm__ volatile("fnstcw %0":"=m"(result.afterCW)); __asm__ volatile("fnstsw %0":"=m"(result.afterSW)); __asm__ volatile("stmxcsr %0":"=m"(result.afterMX));
        result.valid=result.valid && worldAbiValid(result.registers) && result.beforeCW==result.afterCW && result.beforeMX==result.afterMX && (result.beforeSW&0x3800)==(result.afterSW&0x3800);
    }
    current=nullptr; __asm__ volatile("fldcw %0"::"m"(savedCW)); __asm__ volatile("ldmxcsr %0"::"m"(savedMX)); return result;
}
bool oracle(const Recorder& recorder,uint32_t result,const std::vector<uint8_t>& beforeObject,const std::vector<uint8_t>& beforeSources) {
    const auto& r=recorder;
    if(!r.valid) return false;
    if(!r.test.b) {
        if(result!=(r.test.meshes<=0?0xcccccccc:static_cast<uint32_t>(r.test.meshes)) || r.model.sources()!=beforeSources) return false;
        const unsigned produced=r.model.outputs;
        const auto normalized=r.model.normalizedObject();
        for(std::size_t i=0;i<normalized.size();i++) if((i<4 || i>=4+produced*116) && normalized[i]!=beforeObject[i]) return false;
        for(unsigned i=0;i<produced;i++) if(bits(r.model.object.data()+4+i*116+112)!=0x5b5b5b5b) return false;
        if(produced && r.test.pattern==0 && bits(r.model.object.data()+104)!=0x40000000) return false;
        if(produced && r.test.pattern==1 && (bits(r.model.object.data()+104)!=0x3eaaaaab || bits(r.model.object.data()+108)!=0 || bits(r.model.object.data()+112)!=0x3f800000)) return false;
        return true;
    }
    if(result!=0x6a5b4c3d) return false;
    const unsigned iterations=r.test.triangles<=0?0:r.test.script&shrink?1:static_cast<unsigned>(r.test.triangles);
    const uint32_t expectedCount=(r.test.script&globalCount) && iterations?8:iterations;
    if(bits(r.count)!=expectedCount) return false;
    for(unsigned i=0;i<3;i++) for(unsigned j=0;j<12;j++) if(r.records[i*56+44+j]!=0xd3) return false;
    for(unsigned i=iterations;i<3;i++) for(unsigned j=0;j<56;j++) if(r.records[i*56+j]!=0xd3) return false;
    for(unsigned j=0;j<8;j++) if(r.records[3*56+j]!=0xd3) return false;
    for(unsigned i=0;i<iterations;i++) { auto* g=r.records+i*56; if(bits(g+24)!=0 || bits(g+28)!=0xbf800000 || bits(g+32)!=0 || bits(g+36)!=5 || bits(g+40)!=0) return false; }
    std::vector<std::string> names; for(const auto& event:r.events) names.push_back(event.substr(0,event.find('(')));
    std::vector<std::string> expected;
    for(unsigned pass=0;pass<r.test.passes;pass++) { expected.insert(expected.end(),{"glColor4f","glBlendFunc","glBindTexture","glDisable"}); for(unsigned i=0;i<iterations;i++) expected.push_back("glColor4f"); expected.insert(expected.end(),{"glEnable","glColor4f"}); }
    if(names!=expected) return false;
    if(r.test.script==0 && (r.model.normalizedObject()!=beforeObject || r.model.sources()!=beforeSources)) return false;
    return true;
}
void dump(FILE* file,const void* data,std::size_t size) { const uint32_t length=static_cast<uint32_t>(size); std::fwrite(&length,4,1,file); if(size) std::fwrite(data,1,size,file); }
void dumpModel(const char* name,const Recorder& r) { FILE* f=std::fopen(name,"wb"); if(!f) return; const auto model=r.model.normalizedObject(),source=r.model.sources(); dump(f,model.data(),model.size()); dump(f,source.data(),source.size()); dump(f,r.count,4); dump(f,r.records,3*56+8); dump(f,r.texture,4); std::fclose(f); }
void emit(FILE* file,unsigned id,const Recorder& r) { for(unsigned i=0;i<r.events.size();i++) std::fprintf(file,"scenario=%u sequence=%u %s\n",id,i,r.events[i].c_str()); std::fprintf(file,"scenario=%u FINAL %s\n",id,r.snapshot().c_str()); }
template<class Function> bool import(uintptr_t base,uint32_t rva,Function function) { auto* slot=reinterpret_cast<Function*>(base+rva); DWORD old; if(!VirtualProtect(slot,4,PAGE_READWRITE,&old)) return false; *slot=function; DWORD ignored; return VirtualProtect(slot,4,old,&ignored)!=0; }
std::vector<uint8_t> actualNormalized(void* object) {
    auto* bytes=static_cast<uint8_t*>(object); const auto& f=ms3d::fields(object);
    std::vector<uint8_t> result(bytes,bytes+size);
    for(const auto offset:{meshTable,materialTable,triangleTable,vertexTable}) put(result.data()+offset,static_cast<uint32_t>(offset));
    const auto append=[&](const void* data,std::size_t length) { if(length) { auto* begin=static_cast<const uint8_t*>(data); result.insert(result.end(),begin,begin+length); } };
    auto* groups=static_cast<uint8_t*>(f.groupData);
    for(int32_t i=0;i<f.groups;i++) {
        const auto offset=result.size(); append(groups+i*12,12); put(result.data()+offset+8,10+i);
        append(pointer(groups+i*12+8),static_cast<std::size_t>(bits(groups+i*12+4))*4);
    }
    auto* materials=static_cast<uint8_t*>(f.materialData);
    for(int32_t i=0;i<f.materials;i++) {
        const auto offset=result.size(); append(materials+i*80,80); put(result.data()+offset+76,20+i);
        const auto* name=static_cast<const char*>(pointer(materials+i*80+76)); append(name,std::strlen(name)+1);
    }
    append(f.triangleData,f.triangles*76); append(f.vertexData,f.vertices*16); return result;
}
bool actualFixture(uintptr_t base,uint8_t* count,uint8_t* records,uint8_t* texture,FILE* summary,FILE* originals,FILE* candidates) {
    using Construct=void* (__attribute__((thiscall)) *)(void*);
    using Load=bool (__attribute__((thiscall)) *)(void*,const char*);
    const ms3d::Allocator allocator{reinterpret_cast<ms3d::Allocate>(base+0x95420),reinterpret_cast<ms3d::Release>(base+0x958b0)};
    void* first=allocator.allocate(static_cast<uint32_t>(size)); void* second=allocator.allocate(static_cast<uint32_t>(size));
    if(!first || !second) { if(first) allocator.release(first); if(second) allocator.release(second); return false; }
    reinterpret_cast<Construct>(base+0x1956)(first); reinterpret_cast<Construct>(base+0x1956)(second);
    auto load=reinterpret_cast<Load>(base+0x159b0);
    const bool a=load(first,".\\base\\maps\\1\\light.ms3d"),b=load(second,".\\base\\maps\\1\\light.ms3d");
    if(!a || !b) { if(a) ms3d::cleanup(first,allocator); if(b) ms3d::cleanup(second,allocator); allocator.release(first); allocator.release(second); return false; }
    bool equal=actualNormalized(first)==actualNormalized(second);
    for(unsigned stage=0;stage<2;stage++) {
        Case test{"approved_map1_light",stage!=0}; Recorder r(test,count,records,texture),s(test,count,records,texture);
        resetGlobals(r); const auto old=invoke(reinterpret_cast<Entry>(base+(stage?0x35d90:0x27af0)),r,first);
        const auto expected=actualNormalized(first); std::vector<uint8_t> global(records,records+3*56+8); const uint32_t expectedCount=bits(count);
        emit(originals,29+stage,r);
        char name[256]; std::sprintf(name,"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-actual-%c-original.bin",stage?'B':'A'); FILE* f=std::fopen(name,"wb"); if(f) { dump(f,expected.data(),expected.size()); dump(f,global.data(),global.size()); std::fclose(f); }
        resetGlobals(s); const auto before=stage?countB():countA(); const auto replacement=invoke(reinterpret_cast<Entry>(base+(stage?0x1974:0x1690)),s,second);
        const auto result=actualNormalized(second); emit(candidates,29+stage,s);
        std::sprintf(name,"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-actual-%c-candidate.bin",stage?'B':'A'); f=std::fopen(name,"wb"); if(f) { dump(f,result.data(),result.size()); dump(f,records,3*56+8); std::fclose(f); }
        const bool route=(stage?countB():countA())==before+1;
        const bool match=route && old.value==replacement.value && old.valid && replacement.valid && old.afterCW==replacement.afterCW && (old.afterSW&0x3f)==(replacement.afterSW&0x3f) && old.afterMX==replacement.afterMX && expected==result && global==std::vector<uint8_t>(records,records+3*56+8) && expectedCount==bits(count) && r.events==s.events;
        equal=equal && match;
        std::fprintf(summary,"actualFixture=map1/light.ms3d stage=%c originalEAX=%08x candidateEAX=%08x route=%d CW=%04x/%04x exceptions=%02x/%02x result=%s\n",stage?'B':'A',old.value,replacement.value,route,old.afterCW,replacement.afterCW,old.afterSW&0x3f,replacement.afterSW&0x3f,match?"PASS":"FAIL");
    }
    ms3d::cleanup(first,allocator); ms3d::cleanup(second,allocator); allocator.release(first); allocator.release(second); return equal;
}
}
uint32_t offlineFixtures() {
    uint8_t globalCount[4],texture[4],records[3*56+8]; unsigned failures=0;
    for(const auto& test:cases()) { Recorder r(test,globalCount,records,texture); resetGlobals(r); const auto object=r.model.normalizedObject(),source=r.model.sources(); const auto result=invoke(test.b?offlineB:offlineA,r); if(!result.valid || !oracle(r,result.value,object,source)) ++failures; }
    return failures;
}
uint32_t fixtures(uintptr_t base) {
    const auto inputs=cases();
    FILE* summary=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/lighting-fixtures-summary.txt","wb");
    FILE* originals=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/lighting-original-trace.txt","wb"); FILE* candidates=std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/lighting-candidate-trace.txt","wb");
    if(!summary || !originals || !candidates) return 90;
    if(!import(base,0x18ca9c,&colorRecorder) || !import(base,0x18ca98,&blendRecorder) || !import(base,0x18ca58,&bindRecorder) || !import(base,0x18cab8,&disableRecorder) || !import(base,0x18ca68,&enableRecorder) || !install(base,"replace")) return 91;
    fixtureMode(true); unsigned failures=0,id=0;
    auto* count=reinterpret_cast<uint8_t*>(base+0x177f88); auto* records=reinterpret_cast<uint8_t*>(base+0x178110); auto* texture=reinterpret_cast<uint8_t*>(base+0x174f08);
    for(const auto& test:inputs) {
        ++id; Recorder a(test,count,records,texture),b(test,count,records,texture);
        const auto beforeObject=a.model.normalizedObject(),beforeSource=a.model.sources();
        resetGlobals(a);
        char inputName[256]; std::sprintf(inputName,"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-%02u-input.bin",id); dumpModel(inputName,a);
        const auto ar=invoke(reinterpret_cast<Entry>(base+(test.b?0x35d90:0x27af0)),a);
        const auto object=a.model.normalizedObject(),source=a.model.sources(); const std::string state=a.snapshot();
        const bool originalOracle=oracle(a,ar.value,beforeObject,beforeSource);
        char name[256]; std::sprintf(name,"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-%02u-original.bin",id); dumpModel(name,a); emit(originals,id,a);
        resetGlobals(b); const unsigned before=test.b?countB():countA(); const auto br=invoke(reinterpret_cast<Entry>(base+(test.b?0x1974:0x1690)),b);
        const bool route=(test.b?countB():countA())==before+test.passes,model=object==b.model.normalizedObject(),sources=source==b.model.sources(),globals=state==b.snapshot(),trace=a.events==b.events;
        const bool fp=ar.afterCW==br.afterCW && (ar.afterSW&0x3f)==(br.afterSW&0x3f) && ar.afterMX==br.afterMX;
        const bool equal=route && model && sources && globals && trace && fp && ar.value==br.value && ar.valid && br.valid && originalOracle && oracle(b,br.value,beforeObject,beforeSource);
        if(!equal) ++failures; std::sprintf(name,"C:/Users/ADMIN/Boxer-lab/ms3d/lighting-%02u-candidate.bin",id); dumpModel(name,b); emit(candidates,id,b);
        std::fprintf(summary,"scenario=%u name=%s stage=%c originalEAX=%08x candidateEAX=%08x route=%d model=%d source=%d globals=%d trace=%d FP=%d ABI=%d/%d oracle=%d/%d result=%s\n",id,test.name,test.b?'B':'A',ar.value,br.value,route,model,sources,globals,trace,fp,ar.valid,br.valid,originalOracle,oracle(b,br.value,beforeObject,beforeSource),equal?"PASS":"FAIL");
        std::fprintf(summary," abi originalESP=%08x/%08x candidateESP=%08x/%08x originalNV=%08x/%08x/%08x/%08x candidateNV=%08x/%08x/%08x/%08x CW=%04x/%04x SW=%04x/%04x exceptions=%02x/%02x MXCSR=%08x/%08x\n",ar.registers.before,ar.registers.after,br.registers.before,br.registers.after,ar.registers.ebp,ar.registers.ebx,ar.registers.esi,ar.registers.edi,br.registers.ebp,br.registers.ebx,br.registers.esi,br.registers.edi,ar.afterCW,br.afterCW,ar.afterSW,br.afterSW,ar.afterSW&0x3f,br.afterSW&0x3f,ar.afterMX,br.afterMX);
        if(!trace) for(unsigned i=0;i<(std::min)(a.events.size(),b.events.size());i++) if(a.events[i]!=b.events[i]) { std::fprintf(summary," firstDifference=%u original=%s candidate=%s\n",i,a.events[i].c_str(),b.events[i].c_str()); break; }
        std::fflush(summary);
    }
    if(!actualFixture(base,count,records,texture,summary,originals,candidates)) ++failures;
    std::fprintf(summary,"TOTAL scenarios=%u actualFixturePairs=2 failures=%u\n",id,failures); std::fclose(summary); std::fclose(originals); std::fclose(candidates); return failures;
}
}
