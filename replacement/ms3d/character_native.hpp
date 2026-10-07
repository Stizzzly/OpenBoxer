#pragma once
#include "character_layout.hpp"
#include "character_assets.hpp"
namespace character::native {
constexpr uint64_t captureBudget = 64 * 1024 * 1024;
struct Registration {
    void *model, *mesh, *begin, *end, *positions, *normals, *uv, *triangles;
    const ApprovedAsset *asset;
    void *boundOwner = nullptr;
};
bool readable(const void *, uint64_t);
bool executable(uint32_t);
bool install(uintptr_t);
bool supported(void *owner, void *model, Registration &, const char *&reason);
// Bounded registration/source witness query for ANIM-0001. No draw preflight,
// GL checks, geometry scanning or floating arithmetic.
bool animationWitness(void *model);
bool snapshot(const char *path, const Registration &, void *owner);
} // namespace character::native
