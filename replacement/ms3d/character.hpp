#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>
namespace character {
using FloatBits = uint32_t;
struct Triple {
    FloatBits x, y, z;
};
struct CornerPose {
    Triple positionA, positionB, normalA, normalB;
};
// A source is a live view: callbacks may change its values during emission.
// Handles identify retained mesh records without exposing their binary layout.
using MeshHandle = void *;
class Source {
  public:
    virtual ~Source() = default;
    virtual uint32_t meshGate() = 0;
    virtual int32_t meshCount() const = 0;
    virtual MeshHandle mesh(int32_t index) = 0;
    virtual uint32_t frameOffset(MeshHandle, bool frameB) const = 0;
    virtual bool textured(MeshHandle) const = 0;
    virtual uint32_t texture(MeshHandle) = 0;
    virtual int32_t triangleCount(MeshHandle) const = 0;
    virtual int32_t cornerIndex(MeshHandle, int32_t triangle, unsigned corner) const = 0;
    virtual bool uv(MeshHandle, int32_t index, FloatBits &s, FloatBits &t) const = 0;
    virtual CornerPose pose(MeshHandle, uint32_t frameA, uint32_t frameB, int32_t index) const = 0;
    virtual FloatBits factor() const = 0;
};
class Output {
  public:
    virtual ~Output() = default;
    virtual void enableTexture() = 0;
    virtual void bindTexture(uint32_t) = 0;
    virtual void beginTriangles() = 0;
    virtual void texCoord(FloatBits, FloatBits) = 0;
    virtual void normal(Triple) = 0;
    virtual void vertex(Triple) = 0;
    virtual void end() = 0;
};
FloatBits interpolate(FloatBits a, FloatBits b, FloatBits factor);
uint32_t render(Source &, Output &);
bool install(uintptr_t base, const char *mode);
uint32_t fixtures(uintptr_t base);
uint32_t offlineFixtures();
void fixtureMode(bool);
unsigned replacementCount();
using Entry = uint32_t(__attribute__((thiscall)) *)(void *, void *);
struct AbiReport {
    uint32_t beforeStack, afterStack, ebx, esi, edi, ebp, result;
    uint32_t beforeCW, beforeSW, beforeMX, afterCW, afterSW, afterMX;
};
} // namespace character
extern "C" uint32_t __cdecl character_invoke(character::Entry, void *, void *,
                                             character::AbiReport *);
