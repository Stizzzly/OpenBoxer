#pragma once
#include <cstdint>
namespace animation {
// Behavioral interface: the binary layout and calling convention live separately.
struct State {
    virtual ~State() = default;
    virtual int32_t count() = 0;
    virtual uint32_t clock() const = 0;
    virtual uint32_t anchor() const = 0;
    virtual int32_t selector() const = 0;
    virtual int32_t lookupRate(int32_t) = 0;
    virtual uint32_t source(unsigned steps) const = 0;
    virtual void frame(uint32_t) = 0;
    virtual void anchor(uint32_t) = 0;
    virtual void factor(uint32_t) = 0;
};
struct Progress { unsigned steps; uint32_t initialFactor; };
uint32_t advance(State &);
Progress predict(uint32_t elapsed, int32_t rate);
bool install(uintptr_t, const char *mode);
uint32_t fixtures(uintptr_t);
uint32_t replays(uintptr_t);
uint32_t offlineFixtures(const char *report);
}
