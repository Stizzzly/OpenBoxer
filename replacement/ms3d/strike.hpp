#pragma once
#include <array>
#include <cstdint>
#include <cstring>
namespace strike {
using Vector=std::array<uint32_t,4>;
enum class Callback : uint32_t { Construct,Element,Position,Impulse,Cursor,Normalize,AngularZero,Velocity,ValueLength,Absolute,Add,Multiply,BodyVector,PointerLength,AudioQuery,AudioPlay,AudioStop,Sqrt,ScalarFirstMultiply };
// Every pointer is a typed supplied range. Callbacks own their original writes;
// callers copy the live returned range, never a presumed output snapshot.
struct Call {
    Callback id; void *owner=nullptr; std::array<uint32_t,4> args{}; unsigned count=0;
    bool callerCleanup=false; bool scalar=false;
};
struct alignas(16) Result {
    uint32_t eax=0;
    std::array<uint8_t,10> scalar80{};
    alignas(16) std::array<uint8_t,512> fp{};
};
// Global addresses are layout metadata, not names invented for unknown fields.
enum class Global : uint32_t {
    PositionPlayer=0x1761dc,PositionOpponent=0x176240,Distance=0x175eec,
    PlayerVector=0x184ae0,DirectionVector=0x184c98,PlayerBody=0x1849a8,OpponentBody=0x184a90,
    KeyZ=0x17696e,KeyX=0x17696c,KeyC=0x176957,LatchZ=0x185146,LatchX=0x185144,LatchC=0x18512f,
    Type=0x1761f8,Pending=0x17622c,Marker=0x17628c,Attack=0x1761c9,Timer=0x1761cc,
    Dt=0x176370,Fatigue=0x1761f4,Sound=0x177f9e,Audio=0x175628,OpponentHealth=0x176238,PlayerHealth=0x1761d4,Combo=0x1761fc,Fighter=0x184784,Duration=0x1762a4
};
struct State {
    virtual ~State()=default;
    virtual uint8_t *player()=0;
    virtual uint8_t *global(Global)=0;
    virtual Result call(const Call &)=0;
};
inline uint32_t word(const void *p){uint32_t v;std::memcpy(&v,p,4);return v;}
inline void put(void *p,uint32_t v){std::memcpy(p,&v,4);}
inline uint32_t address(const void *p){return uint32_t(reinterpret_cast<uintptr_t>(p));}
inline void *pointer(uint32_t p){return reinterpret_cast<void*>(uintptr_t(p));}
uint32_t update(State &);
bool install(uintptr_t,const char *);
uint32_t fixtures(uintptr_t);
uint32_t replays(uintptr_t);
void fixtureRouteCounters(unsigned *,unsigned *);
}
