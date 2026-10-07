#pragma once
#include "strike_layout.hpp"
#include <vector>
namespace strike {
struct Span {uint32_t rva;unsigned size;};
inline constexpr Span capturedGlobals[]={
{0x184794,1},{0x18479d,1},{0x184790,4},{0x177f90,4},{0x184784,4},{0x1762a4,128},
{0x176956,1},{0x17696a,1},{0x176939,1},{0x17693b,1},{0x176934,1},
{0x17696e,1},{0x17696c,1},{0x176957,1},{0x185146,1},{0x185144,1},{0x18512f,1},
{0x1761c8,1},{0x1761c9,1},{0x1761ca,1},{0x1761cc,4},{0x1761d0,1},{0x1761d4,4},
{0x1761dc,16},{0x176240,16},{0x175eec,4},{0x184ae0,16},{0x184c98,16},
{0x1761f0,1},{0x1761f2,1},{0x1761f4,4},{0x1761f8,4},{0x1761fc,4},{0x176208,1},
{0x17622c,1},{0x176238,4},{0x17628c,1},{0x176370,4},{0x177f9e,1},{0x175628,4},
{0x1849a8,4},{0x184a90,4},{0x1853a4,4}};
struct Snapshot {
    std::array<uint8_t,176> player{};
    std::array<uint8_t,2288> playerBody{},opponentBody{};
    std::vector<uint8_t> globals;
    std::vector<std::array<uint8_t,2288>> activeBodies;
    std::vector<uint32_t> activeAddresses;
    uint32_t world=0,head=0,tail=0,count=0;
    alignas(16) std::array<uint8_t,512> fp{};
};
struct Event {
    Call call;uint32_t caller=0;
    std::array<Vector,4> beforeVectors{},afterVectors{};
    std::array<unsigned,4> pointerArgument{};
    Result result;
    Vector returnedBytes{},ownerBefore{},ownerAfter{};unsigned returnedSize=0,ownerSize=0;
    Snapshot before,after;
};
void snapshot(uintptr_t,void *,Snapshot &);
bool installObservers(uintptr_t);
void restoreObservers(uintptr_t);
void beginCapture(uintptr_t,void *,const char *,bool);
void endCapture(uint32_t);
bool capturing();
using ScriptCallback=Result (*)(const Call &);
void scriptCallbacks(ScriptCallback);
void captureLabel(const char *);
struct Transcript {uint32_t base=0,playerAddress=0,eax=0;bool candidate=false,supported=false;Snapshot before,after;std::vector<Event> events;};
bool saveTranscript(const char *,const Transcript &);
bool loadTranscript(const char *,Transcript &);
Result observedCall(BinaryState &,const Call &,uint32_t caller);
}
