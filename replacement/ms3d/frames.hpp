#pragma once
#include <cstdint>
namespace frames {
struct State {
    virtual ~State()=default;
    virtual int32_t selector() const=0;
    virtual void *lookup(int32_t)=0;
    virtual int32_t gate() const=0;
    virtual int32_t recordStart(void *) const=0;
    virtual int32_t divisor(void *) const=0;
    virtual uint32_t frame() const=0;
    virtual uint32_t slot(unsigned) const=0;
    virtual void slot(unsigned,uint32_t)=0;
    virtual uint32_t advance(uint32_t opaqueECX)=0;
};
uint32_t next(uint32_t frame,int32_t divisor,int32_t start);
uint32_t prepare(State &,uint32_t opaqueECX);
bool install(uintptr_t,const char *);
uint32_t fixtures(uintptr_t);
uint32_t replays(uintptr_t);
uint32_t offlineFixtures(const char *);
}
