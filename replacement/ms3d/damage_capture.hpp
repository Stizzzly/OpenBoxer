#pragma once
#include "damage_native.hpp"
#include <vector>
namespace damage {
struct Snapshot {std::vector<uint8_t> actor,player,playerBody,opponentBody,globals,world;uint32_t playerAddress=0,opponentAddress=0,worldAddress=0;std::vector<unsigned> spanSizes;};
struct Event {unsigned site=0;uint32_t owner=0,eax=0,rngBefore=0,rngAfter=0;uint32_t args[4]{};unsigned returnedSize=0;std::vector<uint8_t> ownerBefore,ownerAfter,returned;std::array<uint32_t,4> combatBefore{},combatAfter{};std::array<std::vector<uint8_t>,4> beforeArgs,afterArgs;alignas(16) std::array<uint8_t,544> entryFp{},exitFp{};std::array<uint32_t,9> entryRegs{},exitRegs{};};
struct Capture {Snapshot before,after;std::vector<Event> events;alignas(16) std::array<uint8_t,544> entryFp{},exitFp{};std::array<uint32_t,9> entryRegs{},exitRegs{};uint32_t mode=0,caller=0,eax=0;uint32_t candidateInvocations=0,originalInvocations=0,actualEntrypoint=0;const char*admissionReason="not-dispatched";bool admittedRoute=false;bool nested=false,candidate=false;const char *label="original";};
using ScriptCallback=Result (*)(const Call&);
void setScript(ScriptCallback);
bool installObserver(uintptr_t,bool force,bool includeAi);
void beginManualCapture(uintptr_t,int32_t,const char*,bool);
void endManualCapture(uint32_t,const Witness* = nullptr);
uint32_t callbackTarget(uint32_t site,uint32_t fallback);
uint32_t audioTarget(uintptr_t gameBase);
inline uint32_t resolveAudioTarget(uint32_t capturedAudio,uintptr_t gameBase){return capturedAudio?capturedAudio:word(pointer(uint32_t(gameBase+0x18cc00)));}
uint32_t capturedCaller(uint32_t caller);
void candidateWitness(bool,const char*);
void setReplacementEnabled(bool);
const Capture *lastCapture();
void restoreSnapshot(uintptr_t,const Snapshot&);
Snapshot captureSnapshot(uintptr_t);
}
