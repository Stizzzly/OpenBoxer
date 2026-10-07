#include "texture.hpp"
namespace texture {
static_assert(sizeof(void*)==4);
int32_t count(void* model) { return *reinterpret_cast<volatile int32_t*>(static_cast<uint8_t*>(model)+countOffset); }
uint8_t* table(void* model) { return *reinterpret_cast<uint8_t* volatile*>(static_cast<uint8_t*>(model)+tableOffset); }
static const char* filename(void* model,int32_t index) { return *reinterpret_cast<const char* volatile*>(table(model)+index*80+76); }
uint32_t run(void* model,const Callbacks& callbacks) {
    int32_t index=0;
    uint32_t result=0xcccccccc;
    while(index<count(model)) {
        const uint32_t length=callbacks.length(filename(model,index));
        const char* path=filename(model,index);
        uint32_t oldId=0;
        if(callbacks.observe) oldId=*reinterpret_cast<volatile uint32_t*>(table(model)+index*80+72);
        const uint32_t id=length?callbacks.upload(path):0;
        *reinterpret_cast<volatile uint32_t*>(table(model)+index*80+72)=id;
        if(callbacks.observe) callbacks.observe(index,path,oldId,id,length!=0);
        ++index; result=static_cast<uint32_t>(index);
    }
    return result;
}
}
