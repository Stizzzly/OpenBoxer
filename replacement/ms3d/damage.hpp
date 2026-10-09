#pragma once
#include <array>
#include <cstdint>
#include <cstring>

// GAME-0002 approved behavioral contract. This interface does not implement
// behavior or authorize native installation. Site metadata remains required.
namespace damage {
using Vector = std::array<uint32_t, 4>;
enum class Callback : uint32_t {
    Position, Element, Sqrt, Negate, Multiply, Construct, Clear, Rand,
    Impulse, AudioPlay, Velocity, Cursor, AngularZero, ValueLength,
    Absolute, Add, BodyVector, PointerLength, AudioQuery, AudioStop,
    ScalarFirstMultiply
};
struct Call {
    Callback id;
    uint32_t site = 0; // Approved call-site identifier; never inferred from order.
    void *owner = nullptr;
    std::array<uint32_t, 4> args{};
    unsigned count = 0;
    bool callerCleanup = false;
    bool scalar = false;
};
struct alignas(16) Result {
    uint32_t eax = 0;
    std::array<uint8_t, 10> scalar80{};
    alignas(16) std::array<uint8_t, 544> fp{};
};
// FXSAVE alone does not necessarily preserve the legacy x87 pointer fields
// exposed by FNSTENV. Keep and restore that separately observed environment.
struct PreserveFp {
    alignas(16) std::array<uint8_t,544> bytes;
    PreserveFp(){__asm__ volatile("fxsave (%0); fnstenv 512(%0); fldenv 512(%0)"::"r"(bytes.data()):"memory");}
    ~PreserveFp(){__asm__ volatile("fxrstor (%0); fldenv 512(%0)"::"r"(bytes.data()):"memory","st");}
};
// The approved specification identifies fields by module RVA or actor offset.
// Preserve those identities without inventing semantic names for unknown data.
struct State {
    alignas(16) std::array<uint8_t,128> liveXmm;
    virtual ~State() = default;
    virtual uint8_t *actor() = 0; // Supplied writable 232-byte typed actor range.
    virtual uint8_t *global(uint32_t rva) = 0;
    virtual Result call(const Call &) = 0;
};
inline void saveXmm(void *out){__asm__ volatile("movups %%xmm0,0(%0); movups %%xmm1,16(%0); movups %%xmm2,32(%0); movups %%xmm3,48(%0); movups %%xmm4,64(%0); movups %%xmm5,80(%0); movups %%xmm6,96(%0); movups %%xmm7,112(%0)"::"r"(out):"memory");}
inline void restoreXmm(const void *in){__asm__ volatile("movups 0(%0),%%xmm0; movups 16(%0),%%xmm1; movups 32(%0),%%xmm2; movups 48(%0),%%xmm3; movups 64(%0),%%xmm4; movups 80(%0),%%xmm5; movups 96(%0),%%xmm6; movups 112(%0),%%xmm7"::"r"(in):"memory");}
inline uint32_t word(const void *p){uint32_t value;std::memcpy(&value,p,4);return value;}
inline void put(void *p,uint32_t value){std::memcpy(p,&value,4);}
inline uint32_t address(const void *p){return uint32_t(reinterpret_cast<uintptr_t>(p));}
inline void *pointer(uint32_t p){return reinterpret_cast<void*>(uintptr_t(p));}
uint32_t update(State &,int32_t mode,bool aiInitiation=false,bool aiContinuation=false);
// Pure getter/RNG capture is compact: no actor/body snapshot per event.
struct CompactEvent {
    uint32_t site = 0;
    Callback id = Callback::Rand;
    uint32_t eax = 0;
    Vector output{}; // Getter output only, interpreted through event identity.
    uint32_t rngBefore = 0;
    uint32_t rngAfter = 0;
    alignas(16) std::array<uint8_t, 512> fpBefore{};
    alignas(16) std::array<uint8_t, 512> fpAfter{};
};
bool installObserver(uintptr_t base,bool force=false,bool includeAi=false);
bool restoreObserver();
} // namespace damage
