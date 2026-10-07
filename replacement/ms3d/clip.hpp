#pragma once
#include <cstdint>
namespace clip {
struct State {
    virtual ~State() = default;
    virtual int32_t bound() const = 0;
    virtual void *lookup(int32_t) = 0;
    virtual int32_t compare(void *) = 0;
    virtual void selector(int32_t) = 0;
    virtual int32_t selector() const = 0;
    virtual uint32_t start(void *) const = 0;
    virtual void frame(uint32_t) = 0;
    virtual void factor(uint32_t) = 0;
    virtual uint32_t clock() const = 0;
    virtual void anchor(uint32_t) = 0;
    virtual uint32_t owner() const = 0;
};
uint32_t select(State &);
bool install(uintptr_t,const char *);
uint32_t fixtures(uintptr_t);
uint32_t replays(uintptr_t);
uint32_t offlineFixtures(const char *);
bool installAction(uintptr_t,const char *,bool movementEnabled=true);
void observeLookup(int32_t,void *);
uint32_t actionFixtures(uintptr_t);
uint32_t actionReplays(uintptr_t);
uint32_t offlineActionFixtures(const char *);
}
