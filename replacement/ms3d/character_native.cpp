#include "character_native.hpp"
#include "character_trace.hpp"
#include "character_path.hpp"
#include "world_runtime.hpp"
#include "runtime_options.hpp"
#include <windows.h>
#include <map>
#include <algorithm>
#include <array>
namespace character::native {
namespace {
uintptr_t module = 0;
unsigned loaderCalls = 0;
bool assetsConfirmed = false;
std::map<void *, Registration> registrations;
std::string jsonText(const std::string &text) {
    std::string result;
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') {
            result += '\\';
            result += char(c);
        } else if (c < 32 || c >= 128) {
            char escape[7];
            std::snprintf(escape, sizeof(escape), "\\u%04x", unsigned(c));
            result += escape;
        } else
            result += char(c);
    }
    return result;
}
bool finite(uint32_t bits) { return (bits & 0x7f800000) != 0x7f800000; }
std::string filenameCopy(const char *p) {
    std::string result;
    if (!p)
        return result;
    for (unsigned i = 0; i < 512; ++i) {
        if (!readable(p + i, 1))
            return {};
        if (!p[i])
            return result;
        result += p[i];
    }
    return {};
}
const ApprovedAsset *assetFor(const std::string &filename) {
    for (const auto &a : approvedAssets) {
        const std::string path = a.path;
        if (filename == path)
            return &a;
    }
    return nullptr;
}
using Loader = uint32_t(__attribute__((thiscall)) *)(void *, void *, const char *);
uint32_t __attribute__((thiscall)) loaderObserver(void *loader, void *model, const char *filename) {
    runtime_options::Timing timing(runtime_options::Unit::Character);
    timing.checkpoint("loader");
    std::string copied;
    {
        FpPreserver preserve;
        registrations.erase(model);
        copied = filenameCopy(filename);
    }
    const uint32_t result = reinterpret_cast<Loader>(module + 0x7160)(loader, model, filename);
    FpPreserver preserve;
    ++loaderCalls;
    std::string normalized;
    const bool validPath = normalizeAssetPath(copied, normalized);
    const ApprovedAsset *asset = assetsConfirmed && validPath ? assetFor(normalized) : nullptr;
    bool registered = false;
    const char *reason = !assetsConfirmed ? "root-asset-hashes-unconfirmed"
                         : !asset         ? "filename-not-in-approved-manifest"
                                          : "loader-or-header-witness-rejected";
    std::array<uint8_t, 108> header{};
    const bool headerReadable = readable(bytes(loader) + 4, 108);
    if (headerReadable)
        std::memcpy(header.data(), bytes(loader) + 4, 108);
    if ((result & 255) == 1 && asset && headerReadable && word(header.data()) == 0x33504449 &&
        word(header.data() + 4) == 15 && word(header.data() + 76) == asset->frames &&
        word(header.data() + 80) == 0 && word(header.data() + 84) == 1 && readable(model, 80) &&
        word(model) == 1) {
        void *begin = pointer(bytes(model) + 28), *end = pointer(bytes(model) + 32);
        reason = "collection-range-rejected";
        if (readable(begin, 288) && uintptr_t(end) == uintptr_t(begin) + 288) {
            MeshView mesh{begin};
            reason = "mesh-count-or-allocation-witness-rejected";
            const uint64_t frameBytes = uint64_t(asset->frames) * asset->vertices * 12;
            if (uint32_t(mesh.vertices()) == asset->vertices &&
                uint32_t(mesh.triangles()) == asset->triangles &&
                frameBytes * 2 + uint64_t(asset->vertices) * 8 + uint64_t(asset->triangles) * 24 <=
                    captureBudget &&
                readable(mesh.positions(), frameBytes) && readable(mesh.normals(), frameBytes) &&
                readable(mesh.uvs(), uint64_t(asset->vertices) * 8) &&
                readable(mesh.indices(), uint64_t(asset->triangles) * 24)) {
                registrations[model] = {
                    model,          begin,      begin,          end,   mesh.positions(),
                    mesh.normals(), mesh.uvs(), mesh.indices(), asset, nullptr};
                registered = true;
                reason = "registered";
            }
        }
    }
    FILE *f = runtime_options::diagnostics(runtime_options::Unit::Character) || runtime_options::capture(runtime_options::Unit::Character) ? std::fopen("C:/Users/ADMIN/Boxer-lab/ms3d/character-loads.jsonl", "ab") : nullptr;
    if (f) {
        // Asset paths use normalized forward slashes in JSON.
        std::string path = asset ? asset->path : "UNKNOWN";
        std::replace(path.begin(), path.end(), '\\', '/');
        const std::string escapedFilename = jsonText(copied),
                          escapedNormalized = jsonText(normalized);
        std::fprintf(f,
                     "{\"call\":%u,\"generation\":%u,\"model_identity\":%u,\"loader_identity\":%u,"
                     "\"result\":%u,\"registered\":%s,\"registration_reason\":\"%s\",\"copied_"
                     "filename\":\"%s\",\"normalized_filename\":\"%s\",\"header_readable\":%s,"
                     "\"signature_le\":%u,\"version\":%u,\"frames\":%u,\"tags\":%u,\"surfaces\":%u,"
                     "\"root_asset_hashes_confirmed\":%s,\"asset\":\"%s\",\"sha256\":\"%s\"}\n",
                     loaderCalls, loaderCalls, uint32_t(uintptr_t(model)),
                     uint32_t(uintptr_t(loader)), result, registered ? "true" : "false", reason,
                     escapedFilename.c_str(), escapedNormalized.c_str(),
                     headerReadable ? "true" : "false", word(header.data()),
                     word(header.data() + 4), word(header.data() + 76), word(header.data() + 80),
                     word(header.data() + 84), assetsConfirmed ? "true" : "false", path.c_str(),
                     asset ? asset->sha256 : "UNKNOWN");
        std::fclose(f);
    }
    return result;
}
} // namespace
bool readable(const void *ptr, uint64_t size) {
    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    if (!size)
        return true;
    if (!p || size > captureBudget || uint64_t(p) + size > 0xffffffffULL)
        return false;
    while (size) {
        MEMORY_BASIC_INFORMATION m{};
        if (!VirtualQuery(reinterpret_cast<void *>(p), &m, sizeof(m)) || m.State != MEM_COMMIT ||
            (m.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            return false;
        const uintptr_t end = reinterpret_cast<uintptr_t>(m.BaseAddress) + m.RegionSize;
        const uint64_t step = (std::min)(size, uint64_t(end - p));
        if (!step)
            return false;
        p += uintptr_t(step);
        size -= step;
    }
    return true;
}
bool executable(uint32_t address) {
    MEMORY_BASIC_INFORMATION m{};
    return address && VirtualQuery(reinterpret_cast<void *>(uintptr_t(address)), &m, sizeof(m)) &&
           m.State == MEM_COMMIT && !(m.Protect & (PAGE_NOACCESS | PAGE_GUARD)) &&
           (m.Protect &
            (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY));
}
bool animationWitness(void *model) {
    const auto found=registrations.find(model);
    if(found==registrations.end()) return false;
    auto &r=found->second;
    // Registration alone does not distinguish owner+532 from owner+644. Wait
    // for the existing rendering witness to establish the actual owner.
    void *owner=r.boundOwner;
    if(!owner || uintptr_t(model)!=uintptr_t(owner)+644) return false;
    if(!readable(owner,756) || !readable(model,112) || word(model)!=1 ||
       pointer(bytes(model)+28)!=r.begin || pointer(bytes(model)+32)!=r.end ||
       !readable(r.mesh,288)) return false;
    const MeshView mesh{r.mesh};
    if(uint32_t(mesh.vertices())!=r.asset->vertices ||
       uint32_t(mesh.triangles())!=r.asset->triangles ||
       mesh.positions()!=r.positions || mesh.normals()!=r.normals ||
       mesh.uvs()!=r.uv || mesh.indices()!=r.triangles) return false;
    return true;
}
bool supported(void *owner, void *model, Registration &registration, const char *&reason) {
    FpPreserver preserve;
    uint16_t cw;
    __asm__ volatile("fnstcw %0" : "=m"(cw));
    if (cw != 0x027f) {
        reason = "unsupported-cw";
        return false;
    }
    const auto found = registrations.find(model);
    if (found == registrations.end()) {
        reason = "unregistered-loader-bounds";
        return false;
    }
    registration = found->second;
    // Scope is one witnessed embedded model in one disposable startup. Loader
    // entry invalidates registration before original parsing/reload begins.
    bool embedded = false;
    for (uint32_t offset = 420; offset <= 1316; offset += 112)
        if (uintptr_t(model) == uintptr_t(owner) + offset)
            embedded = true;
    if (!embedded || (registration.boundOwner && registration.boundOwner != owner)) {
        reason = "unmatched-owner-lifetime";
        return false;
    }
    if (!readable(model, 80) || word(model) != 1 ||
        pointer(bytes(model) + 28) != registration.begin ||
        pointer(bytes(model) + 32) != registration.end || !readable(registration.mesh, 288)) {
        reason = "changed-model-witness";
        return false;
    }
    MeshView mesh{registration.mesh};
    const auto &asset = *registration.asset;
    if (uint32_t(mesh.vertices()) != asset.vertices ||
        uint32_t(mesh.triangles()) != asset.triangles ||
        mesh.positions() != registration.positions || mesh.normals() != registration.normals ||
        mesh.uvs() != registration.uv || mesh.indices() != registration.triangles) {
        reason = "changed-mesh-witness";
        return false;
    }
    if (mesh.textured()) {
        reason = "owner-texture-capacity-unknown";
        return false;
    }
    ModelView source{model};
    const int32_t a = source.frame(false), b = source.frame(true);
    const uint32_t factor = source.factor();
    if (a < 0 || b < 0 || uint32_t(a) >= asset.frames || uint32_t(b) >= asset.frames ||
        !finite(factor)) {
        reason = "invalid-pose";
        return false;
    }
    const uint64_t frameBytes = uint64_t(asset.frames) * asset.vertices * 12;
    if (!readable(mesh.positions(), frameBytes) || !readable(mesh.normals(), frameBytes) ||
        !readable(mesh.uvs(), uint64_t(asset.vertices) * 8) ||
        !readable(mesh.indices(), uint64_t(asset.triangles) * 24)) {
        reason = "unreadable-allocation";
        return false;
    }
    for (unsigned i = 0; i < 7; ++i)
        if (!executable(word(reinterpret_cast<uint8_t *>(module) + slots[i]))) {
            reason = "invalid-gl-dispatch";
            return false;
        }
    for (uint32_t t = 0; t < asset.triangles; ++t)
        for (unsigned corner = 0; corner < 3; ++corner) {
            const int32_t index = int32_t(word(bytes(mesh.indices()) + 24 * t + 4 * corner));
            if (index < 0 || uint32_t(index) >= asset.vertices) {
                reason = "invalid-triangle-index";
                return false;
            }
        }
    for (uint32_t i = 0; i < asset.vertices * 2; ++i)
        if (!finite(word(bytes(mesh.uvs()) + 4 * i))) {
            reason = "nonfinite-uv";
            return false;
        }
    // Validate the two complete accessed frames; unused frames remain bounded
    // by loader allocation evidence but are not arithmetic inputs this draw.
    for (unsigned array = 0; array < 2; ++array) {
        const auto *p = bytes(array ? mesh.normals() : mesh.positions());
        for (uint32_t v = 0; v < asset.vertices * 3; ++v) {
            const uint32_t av = word(p + 12 * asset.vertices * uint32_t(a) + 4 * v),
                           bv = word(p + 12 * asset.vertices * uint32_t(b) + 4 * v);
            if (!finite(av) || !finite(bv) || !finite(interpolate(av, bv, factor))) {
                reason = "nonfinite-emission";
                return false;
            }
        }
    }
    found->second.boundOwner = owner;
    registration.boundOwner = owner;
    reason = "approved-untextured-loader-bounds";
    return true;
}
bool snapshot(const char *path, const Registration &r, void *owner) {
    FILE *f = std::fopen(path, "wb");
    if (!f)
        return false;
    ModelView model{r.model};
    // Version 2 stores two complete accessed frames rather than unused frames.
    // Full frame bounds remain explicit; replay recreates sparse frame-major
    // storage and keeps the actual original A/B indices.
    const uint32_t header[] = {0x36485243,
                               2,
                               r.asset->frames,
                               r.asset->vertices,
                               r.asset->triangles,
                               uint32_t(model.frame(false)),
                               uint32_t(model.frame(true)),
                               model.factor(),
                               uint32_t(uintptr_t(owner)),
                               uint32_t(uintptr_t(r.model))};
    bool ok = std::fwrite(header, 4, 10, f) == 10;
    uint8_t modelBytes[80], meshBytes[288];
    std::memcpy(modelBytes, r.model, 80);
    std::memcpy(meshBytes, r.mesh, 288);
    // Logical collection/array roles replace numerical pointers in the capture.
    for (unsigned offset : {12u, 16u, 20u, 28u, 32u, 36u})
        put(modelBytes + offset, 0);
    for (unsigned offset : {272u, 276u, 280u, 284u})
        put(meshBytes + offset, 0);
    ok = ok && std::fwrite(modelBytes, 1, 80, f) == 80 && std::fwrite(meshBytes, 1, 288, f) == 288;
    const size_t frameBytes = size_t(r.asset->vertices) * 12;
    for (const void *array : {r.positions, r.normals})
        for (int32_t frame : {model.frame(false), model.frame(true)})
            ok = ok && std::fwrite(bytes(array) + frameBytes * uint32_t(frame), 1, frameBytes, f) ==
                           frameBytes;
    ok = ok && std::fwrite(r.uv, 8, r.asset->vertices, f) == r.asset->vertices &&
         std::fwrite(r.triangles, 24, r.asset->triangles, f) == r.asset->triangles;
    return std::fclose(f) == 0 && ok;
}
bool install(uintptr_t base) {
    module = base;
    char witness[8]{};
    GetEnvironmentVariableA("OPENBOXER_CHARACTER_ASSETS_CONFIRMED", witness, sizeof(witness));
    assetsConfirmed = std::strcmp(witness, "1") == 0;
    return world::redirect(base + 0x144c, reinterpret_cast<uintptr_t>(&loaderObserver));
}
} // namespace character::native
