#include "lighting.hpp"
namespace lighting {
static_assert(sizeof(void*)==4);
uint32_t bits(const void* value) { return *static_cast<const volatile uint32_t*>(value); }
void put(void* target,uint32_t value) { *static_cast<volatile uint32_t*>(target)=value; }
void* pointer(const void* value) { return *static_cast<void* const volatile*>(value); }
static int32_t integer(const void* value) { return *static_cast<const volatile int32_t*>(value); }
static uint8_t* array(void* model,std::size_t offset) { return static_cast<uint8_t*>(pointer(static_cast<uint8_t*>(model)+offset)); }
// These independently designed semantic arithmetic adapters retain the active
// x87 precision/rounding and sticky effects, with one final float32 store.
static uint32_t center(const void* first,const void* second,const void* third,bool multiply) {
    uint32_t result;
    const uint32_t constant=multiply?0x3eaaaa3b:0x40400000;
    if(multiply) {
        __asm__ volatile("flds (%1); fadds (%2); fadds (%3); fmuls %4; fstps %0":"=m"(result):"r"(first),"r"(second),"r"(third),"m"(constant):"st","memory");
    } else {
        __asm__ volatile("flds (%1); fadds (%2); fadds (%3); fdivs %4; fstps %0":"=m"(result):"r"(first),"r"(second),"r"(third),"m"(constant):"st","memory");
    }
    return result;
}
static void doubled(const void* value) {
    volatile uint32_t unused;
    __asm__ volatile("flds (%1); fadd %%st(0),%%st(0); fstps %0":"=m"(unused):"r"(value):"st","memory");
}
uint32_t stageA(void* object) {
    auto* model=static_cast<uint8_t*>(object);
    int32_t mesh=0,output=0;
    uint32_t result=0xcccccccc;
    while(mesh<integer(model+meshCount)) {
        auto* group=array(model,meshTable)+mesh*12;
        const uint32_t material=bits(group);
        for(int32_t member=0;member<integer(group+4);member++) {
            auto* list=static_cast<uint8_t*>(pointer(group+8));
            const int32_t triangleIndex=integer(list+member*4);
            auto* triangle=array(model,triangleTable)+triangleIndex*76;
            auto* record=model+4+output*116;
            put(record,material);
            uint8_t* positions[3];
            for(int32_t vertex=0;vertex<3;vertex++) {
                positions[vertex]=array(model,vertexTable)+integer(triangle+60+vertex*4)*16+4;
                const std::size_t at=4+vertex*32;
                for(unsigned component=0;component<3;component++) put(record+at+component*4,bits(positions[vertex]+component*4));
                put(record+at+12,bits(triangle+36+vertex*4)); put(record+at+16,bits(triangle+48+vertex*4));
                for(unsigned component=0;component<3;component++) put(record+at+20+component*4,bits(triangle+vertex*12+component*4));
            }
            for(unsigned component=0;component<3;component++) put(record+100+component*4,center(positions[0]+component*4,positions[1]+component*4,positions[2]+component*4,false));
            ++output;
        }
        ++mesh; result=static_cast<uint32_t>(mesh);
    }
    return result;
}
uint32_t stageB(void* object,uint8_t* count,uint8_t* records,uint8_t* texture,const GL& gl) {
    auto* model=static_cast<uint8_t*>(object);
    put(count,0);
    lighting_color_bits(gl.color,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
    gl.blend(0x302,1); gl.bind(0x0de1,bits(texture)); gl.disable(0x0b44);
    for(int32_t index=0;index<integer(model+triangleCount);index++) {
        auto* flat=model+4+index*116;
        const int32_t selected=integer(flat);
        auto* material=array(model,materialTable)+selected*80;
        doubled(material+48);
        const uint32_t blue=bits(material+24),green=bits(material+20),red=bits(material+16);
        lighting_color_bits(gl.color,red,green,blue,0x3f333333);
        uint32_t xyz[3];
        for(unsigned component=0;component<3;component++) xyz[component]=center(flat+4+component*4,flat+36+component*4,flat+68+component*4,true);
        auto* record=records+index*56;
        for(unsigned component=0;component<3;component++) put(record+component*4,xyz[component]);
        for(unsigned component=0;component<3;component++) {
            material=array(model,materialTable)+integer(flat)*80;
            put(record+12+component*4,bits(material+16+component*4));
        }
        put(record+24,0); put(record+28,0xbf800000); put(record+32,0); put(record+36,5); put(record+40,0);
        put(count,bits(count)+1);
    }
    gl.enable(0x0b44);
    return lighting_color_bits(gl.color,0x3f800000,0x3f800000,0x3f800000,0x3f800000);
}
}
